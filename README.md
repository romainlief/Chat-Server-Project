# Chat Server Project

## Description

This project implements a multi-threaded chat server and client system written in C. The application allows multiple clients to connect to a central server and exchange messages in real-time through TCP sockets. The project demonstrates advanced operating system concepts including threading, socket programming, inter-process communication, and synchronization mechanisms.

## Development Team

- **Romain Liefferinckx**
- **Manuel Rocca**
- **Rares Radu-Loghin**

## Features

### Server (`serveur-chat`)

- **Multi-threaded architecture**: Each client connection is handled by a dedicated thread
- **Concurrent client management**: Supports up to 1000 simultaneous connections
- **Thread-safe operations**: Uses mutex locks to protect shared resources
- **Dynamic client management**: Real-time addition/removal of clients
- **Message routing**: Routes messages between clients based on recipient pseudonyms
- **Error handling**: Robust error handling and graceful disconnection management

### Client (`chat`)

- **Real-time messaging**: Send and receive messages in real-time
- **Multiple display modes**:
  - **Manual mode** (`--manuel`): Messages are buffered and displayed on SIGINT
  - **Bot mode** (`--bot`): Simplified output format for automated clients
  - **Default mode**: Real-time message display with formatting
- **Signal handling**: Custom signal handlers for graceful shutdown
- **Thread-based I/O**: Separate threads for reading and writing operations
- **Environment configuration**: Configurable server IP and port via environment variables

### Chat-Auto Script

- **Automated messaging**: Bash script for simplified batch messaging
- **Pipe-friendly**: Designed to work with input redirection and pipes
- **User-friendly interface**: Streamlined interaction for automated scenarios

## Installation and Compilation

### Prerequisites

- GCC compiler with C2x standard support
- POSIX-compliant system (Linux/macOS)
- pthread library
- Standard C libraries

### Compilation

```bash
# Build both client and server
make all

# Build only the client
make chat

# Build only the server
make serveur-chat
```

### Cleanup

```bash
make clean
```

## Usage

### Starting the Server

```bash
# Start server on default port (1234)
./serveur-chat

# Configure custom port via environment variable
export PORT_SERVEUR=8080
./serveur-chat
```

### Client Usage

```bash
# Basic usage with username
./chat <username>

# Manual mode (messages buffered until SIGINT)
./chat <username> --manuel

# Bot mode (simplified output)
./chat <username> --bot

# Combined modes
./chat <username> --manuel --bot
```

### Environment Variables

### Message Format

Messages follow the format: `recipient_username message_content`

Example:

```text
alice Hello, how are you?
bob Good morning!
```

### Using Chat-Auto Script

```bash
# Interactive usage
./chat-auto username

# With input redirection
echo -e "alice\nHello Alice!" | ./chat-auto bob

# From file
./chat-auto username < message_file.txt
```
