# netlink-user-app/netlink-user-app/README.md

# Netlink User Application

This project is a user application that communicates with a Linux kernel module using netlink sockets. It allows sending and receiving messages between user space and kernel space.

## Project Structure

- `src/main.c`: Entry point of the user application. Initializes the netlink socket, sends messages to the kernel module, and handles incoming messages.
- `src/netlink.c`: Contains the implementation of functions to create, send, and receive messages through the netlink socket.
- `include/netlink.h`: Header file defining constants, structures, and function prototypes for netlink communication.
- `Makefile`: Build instructions for compiling the user application.

## Prerequisites

- Linux operating system
- Kernel module that supports netlink communication
- Development tools (gcc, make)

## Building the Application

To build the application, navigate to the project directory and run:

```
make
```

This will compile the source files and create the executable.

## Running the Application

After building, you can run the application with:

```
./netlink-user-app
```

Ensure that the kernel module is loaded before running the user application.

## License

This project is licensed under the GPL.