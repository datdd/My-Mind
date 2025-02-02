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

typedef struct {
    int socket;
    FILE *file;
    char filename[BUFFER_SIZE];
    int receiving_file;
} ClientInfo;

int main() {
    int server_fd, new_socket, max_fd;
    ClientInfo clients[MAX_CLIENTS];
    fd_set read_fds;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // Initialize clients array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        clients[i].file = NULL;
        clients[i].receiving_file = 0;
    }

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
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
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // Add client sockets to set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket > 0) {
                FD_SET(clients[i].socket, &read_fds);
            }
            if (clients[i].socket > max_fd) {
                max_fd = clients[i].socket;
            }
        }

        // Wait for activity on sockets
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("Select error");
        }

        // Handle new connections
        if (FD_ISSET(server_fd, &read_fds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                perror("Accept failed");
            } else {
                // Set new socket to non-blocking mode
                flags = fcntl(new_socket, F_GETFL, 0);
                fcntl(new_socket, F_SETFL, flags | O_NONBLOCK);

                // Add new client to list
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].socket == 0) {
                        clients[i].socket = new_socket;
                        clients[i].receiving_file = 0; // Reset state
                        printf("New client connected, socket fd: %d, IP: %s, Port: %d\n",
                               new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                        break;
                    }
                }
            }
        }

        // Handle client data
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int client_fd = clients[i].socket;
            if (client_fd > 0 && FD_ISSET(client_fd, &read_fds)) {
                int valread = read(client_fd, buffer, BUFFER_SIZE);
                
                if (valread == 0) {
                    // Client disconnected
                    getpeername(client_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Client disconnected, IP: %s, Port: %d\n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(client_fd);
                    if (clients[i].file) {
                        fclose(clients[i].file);
                        printf("File transfer incomplete or client disconnected early.\n");
                    }
                    clients[i].socket = 0;
                    clients[i].file = NULL;
                    clients[i].receiving_file = 0;
                } 
                else if (valread > 0) {
                    buffer[valread] = '\0';

                    // Handle "UPLOAD filename"
                    if (clients[i].receiving_file == 0 && strncmp(buffer, "UPLOAD", 6) == 0) {
                        sscanf(buffer, "UPLOAD %s", clients[i].filename);
                        printf("Client %d is uploading file: %s\n", client_fd, clients[i].filename);

                        // Open file for writing
                        clients[i].file = fopen(clients[i].filename, "wb");
                        if (!clients[i].file) {
                            perror("Failed to open file");
                            continue;
                        }

                        clients[i].receiving_file = 1; // Mark file transfer in progress
                    } 
                    else if (clients[i].receiving_file == 1) {
                        // Writing file data
                        fwrite(buffer, 1, valread, clients[i].file);
                        printf("Received %d bytes for file %s\n", valread, clients[i].filename);
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
