# Echo Server
A simple server which echoes back whatever the client transmits.

Made for learning purposes, referenced from the book "Beej’s Guide to Network Programming" by Brian “Beej Jorgensen” Hall

## Specifications
- Uses Stream Socket and TCP
- Uses a process-per-connection model (`fork()`) to handle multiple clients concurrently
- Connection can be terminated by sending "exit"

## How to use

### Server

Compile the server
```bash
g++ -std=c++11 server.cpp -o server
```
Run the server
```bash
./server
```
### Client

Compile the client
```bash
g++ -std=c++11 client.cpp -o client
```
Run the client with the address to the server

If on the same machine, then open another terminal and run:
```bash
./client localhost
```

If on different machine
```bash
./client <ip_address>
```

## Requirements
- POSIX-compliant operating system (Linux, macOS, BSD)
- C++11 or later
- An available TCP port 7777 (or modify PORT in the source)