#include "TCPServer.hpp"

#include "HTTPParser.hpp"
#include "HTTPResponse.hpp"

#include <arpa/inet.h>
#include <cerrno>
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
        if (errno != EINTR)
        {
            perror("accept");
        }
        return -1;
    }
    return clientFd;
}

// handle one connection end to end: read -> parse -> respond -> close.
void TCPServer::handleClient(int clientFd)
{
    HTTPParser parser;
    HTTPResponse response;
    ParseResult result = ParseResult::Incomplete;

    // read loop: keep recv'ing until the parser is Complete or Error, or the
    // peer closes (n == 0), or a real socket error happens.
    char buffer[4096];
    while (true)
    {
        ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

        if (bytesRead > 0)
        {
            result = parser.feed(buffer, static_cast<std::size_t>(bytesRead));

            if (result == ParseResult::Complete)
            {
                break; // request fully parsed
            }
            if (result == ParseResult::Error)
            {
                break; // malformed
            }
            // Incomplete: loop and recv again
        }
        else if (bytesRead == 0)
        {
            // peer closed the connection
            break;
        }
        else // bytesRead < 0
        {
            if (errno == EINTR)
            {
                continue; // interrupted, retry
            }
            std::cerr << "recv: " << std::strerror(errno) << "\n";
            close(clientFd);
            return; // no response on a hard socket error
        }
    }

    // build the response from the parse outcome
    if (result == ParseResult::Complete)
    {
        HTTPRequest request = parser.takeRequest();
        std::cout << request.method << " " << request.target << " " << request.version << std::endl;

        response.statusCode = 200;
        response.reason = "OK";
        response.body = "Hello World!";
        response.headers["Content-Type"] = "text/plain";
        response.headers["Content-Length"] = std::to_string(response.body.size());
        response.headers["Connection"] = "close";
    }
    else
    {
        if (result == ParseResult::Error)
        {
            std::cout << "parse error: " << parser.errorReason() << std::endl;
        }
        else
        {
            std::cout << "incomplete request (client closed early)" << std::endl;
        }

        response.statusCode = 400;
        response.reason = "Bad Request";
        response.body = "Bad Request\n";
        response.headers["Content-Type"] = "text/plain";
        response.headers["Content-Length"] = std::to_string(response.body.size());
        response.headers["Connection"] = "close";
    }

    // send loop: send() may write fewer bytes than requested, so advance an
    // offset until the whole serialized response is on the wire.
    std::string serialized = response.toString();
    std::size_t offset = 0;
    while (offset < serialized.size())
    {
        ssize_t bytesWritten = send(clientFd, serialized.c_str() + offset, serialized.size() - offset, 0);
        if (bytesWritten < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            std::cerr << "send: " << std::strerror(errno) << "\n";
            break;
        }
        if (bytesWritten == 0)
        {
            break;
        }
        offset += static_cast<std::size_t>(bytesWritten);
    }

    close(clientFd); // every path, no fd leaks
}

// accept -> handle -> repeat. loops forever.
void TCPServer::run()
{
    while (true)
    {
        int clientFd = acceptClient();
        if (clientFd < 0)
        {
            continue; // accept failed, try again
        }

        handleClient(clientFd);
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
