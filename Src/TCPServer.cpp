#include "TCPServer.hpp"

#include "HTTPResponse.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

TCPServer::TCPServer() : serverFd(-1) {}

TCPServer::~TCPServer()
{
    if (serverFd >= 0)
    {
        close(serverFd);
    }
}

// open a TCP socket. AF_INET = IPv4, SOCK_STREAM = TCP.
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

// SO_REUSEADDR so a fast restart doesn't die with "Address already in use".
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

// bind to 127.0.0.1:port. loopback only - don't expose half-baked servers.
bool TCPServer::bindTo(int port)
{
    sockaddr_in addr{};                            // zeroed out
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

// mark it listening. backlog = how many connections can queue up.
bool TCPServer::listenOn(int backlog)
{
    if (listen(serverFd, backlog) < 0)
    {
        perror("listen");
        return false;
    }
    return true;
}

// accept one client, hand back its fd (or -1 on error).
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

// accept -> recv -> reply -> close. loops forever.
void TCPServer::run()
{
    while (true)
    {
        int clientFd = acceptClient();
        if (clientFd < 0)
        {
            continue; // accept failed, try again
        }

        char buffer[1024];
        // read up to 1023 so there's room for a '\0'.
        ssize_t receive = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

        if (receive < 0)
        {
            perror("recv");
        }
        else if (receive == 0)
        {
            std::cout << "Client closed connection" << std::endl;
        }
        else
        {
            buffer[receive] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            // phase 2: fixed response on purpose. request parsing is phase 3,
            // so for now every request gets the same valid HTTP reply. the
            // point here is correct framing, not routing.
            HTTPResponse response;
            response.statusCode = 200;
            response.reason = "OK";
            response.body = "Hello World!";
            response.headers["Content-Type"] = "text/plain";
            response.headers["Content-Length"] = std::to_string(response.body.size());
            response.headers["Connection"] = "close";

            std::string serialized = response.toString();
            ssize_t sent = send(clientFd, serialized.c_str(), serialized.size(), 0);
            if (sent < 0)
            {
                perror("send");
            }
        }

        close(clientFd); // every path, no fd leaks
    }
}

// close the listener if open. safe to call twice.
void TCPServer::stop()
{
    if (serverFd >= 0)
    {
        close(serverFd);
        serverFd = -1;
    }
}
