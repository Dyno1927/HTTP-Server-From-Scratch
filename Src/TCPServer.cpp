#include "TCPServer.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
// <sys/socket.h> & <netinet/in.h> not included as they are included by "TCPServer.hpp"

using namespace std;

TCPServer::TCPServer() : serverFd(-1) {}

TCPServer::~TCPServer()
{
    if (serverFd >= 0)
    {
        close(serverFd);
    }
}

// Open a TCP socket (AF_INET = IPv4, SOCK_STREAM = TCP, 0 = default protocol).
bool TCPServer::createSocket()
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0)
    {
        perror("socket");
        return false;
    }
    return true;
}

// SO_REUSEADDR lets a fast restart succeed instead of failing with
// "Address already in use".
bool TCPServer::setReuseAddr()
{
    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        return false;
    }
    return true;
}

// Bind to 127.0.0.1:port (loopback only - never expose an incomplete server).
bool TCPServer::bindTo(int port)
{
    sockaddr_in addr{};                            // zero-initialised
    addr.sin_family = AF_INET;                     // IPv4
    addr.sin_port = htons(port);                   // host -> network byte order
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (bind(serverFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
    {
        perror("bind");
        return false;
    }
    return true;
}

// Mark the socket as listening; backlog sizes the pending-connection queue.
bool TCPServer::listenOn(int backlog)
{
    if (listen(serverFd, backlog) < 0)
    {
        perror("listen");
        return false;
    }
    return true;
}

// Accept one client and return its fd (or -1 on error).
int TCPServer::acceptClient()
{
    int clientFd = accept(serverFd, nullptr, nullptr);
    if (clientFd < 0)
    {
        perror("accept");
        return -1;
    }
    return clientFd;
}

// Blocking accept loop: accept -> recv -> reply -> close.
void TCPServer::run()
{
    while (true)
    {
        int clientFd = acceptClient();
        if (clientFd < 0)
        {
            continue; // accept() failed; try again
        }

        char buffer[1024];
        // Read up to 1023 bytes so there's room for a '\0' terminator.
        ssize_t receive = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

        if (receive < 0)
        {
            perror("recv");
        }
        else if (receive == 0)
        {
            cout << "Client closed connection" << endl;
        }
        else
        {
            buffer[receive] = '\0';
            cout << "Received: " << buffer << endl;
            const char *reply = "Yo! It's Workin'";
            if (send(clientFd, reply, strlen(reply), 0) < 0)
            {
                perror("send");
            }
        }

        close(clientFd); // every path
    }
}

// Close the listening socket if open; safe to call more than once.
void TCPServer::stop()
{
    if (serverFd >= 0)
    {
        close(serverFd);
        serverFd = -1;
    }
}
