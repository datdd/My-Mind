#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 4096

typedef struct
{
    int socket;
    FILE *file;
    char filename[BUFFER_SIZE];
    char command_buffer[BUFFER_SIZE]; // Buffer for handling partial messages
    int command_length;
    int receiving_file;
} ClientInfo;

void sanitize_filename(char *filename)
{
    char *invalid_chars = "/\\:*?\"<>|";
    for (int i = 0; filename[i]; i++)
    {
        if (strchr(invalid_chars, filename[i]))
        {
            filename[i] = '_'; // Replace invalid characters
        }
    }
}

int main()
{
    int server_fd, new_socket, max_fd;
    ClientInfo clients[MAX_CLIENTS];
    fd_set read_fds;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].socket = 0;
        clients[i].file = NULL;
        clients[i].command_length = 0;
        clients[i].receiving_file = 0;
    }

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set server socket to non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // Add client sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].socket > 0)
            {
                FD_SET(clients[i].socket, &read_fds);
                if (clients[i].socket > max_fd)
                {
                    max_fd = clients[i].socket;
                }
            }
        }

        // Wait for activity on sockets
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR)
        {
            perror("Select error");
        }

        // Handle new connections
        if (FD_ISSET(server_fd, &read_fds))
        {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
            if (new_socket < 0)
            {
                perror("Accept failed");
            }
            else
            {
                // Set new socket to non-blocking mode
                flags = fcntl(new_socket, F_GETFL, 0);
                fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

                // Add new client to list
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i].socket == 0)
                    {
                        clients[i].socket = new_socket;
                        clients[i].command_length = 0;
                        clients[i].receiving_file = 0;
                        printf("New client connected, socket fd: %d, IP: %s, Port: %d\n",
                               new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        break;
                    }
                }
            }
        }

        // Handle client data
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int client_fd = clients[i].socket;
            if (client_fd > 0 && FD_ISSET(client_fd, &read_fds))
            {
                int valread = read(client_fd, buffer, BUFFER_SIZE);

                if (valread == 0)
                {
                    // Client disconnected
                    getpeername(client_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected, IP: %s, Port: %d\n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(client_fd);
                    if (clients[i].file)
                    {
                        fclose(clients[i].file);
                        remove(clients[i].filename); // Remove incomplete file
                        printf("Incomplete file %s removed.\n", clients[i].filename);
                    }
                    clients[i].socket = 0;
                    clients[i].file = NULL;
                    clients[i].receiving_file = 0;
                }
                else if (valread > 0)
                {
                    if (clients[i].receiving_file == 0)
                    {
                        // Store command into buffer
                        memcpy(clients[i].command_buffer + clients[i].command_length, buffer, valread);
                        clients[i].command_length += valread;
                        clients[i].command_buffer[clients[i].command_length] = '\0';

                        if (strncmp(clients[i].command_buffer, "UPLOAD ", 7) == 0)
                        {
                            sscanf(clients[i].command_buffer, "UPLOAD %s", clients[i].filename);
                            sanitize_filename(clients[i].filename);

                            clients[i].file = fopen(clients[i].filename, "wb");
                            if (!clients[i].file)
                            {
                                perror("Failed to open file");
                                continue;
                            }
                            clients[i].receiving_file = 1;
                            printf("Receiving file: %s\n", clients[i].filename);
                        }
                    }
                    else if (clients[i].receiving_file == 1)
                    {
                        if (strncmp(buffer, "END_UPLOAD", 10) == 0)
                        {
                            fclose(clients[i].file);
                            clients[i].file = NULL;
                            clients[i].receiving_file = 0;
                            printf("File transfer completed: %s\n", clients[i].filename);
                        }
                        else
                        {
                            fwrite(buffer, 1, valread, clients[i].file);
                            printf("Received %d bytes for file %s\n", valread, clients[i].filename);
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
