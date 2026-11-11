#include "TCPServer.hpp"

#include <iostream>

// wire the socket lifecycle in order, then run the accept loop. Ctrl+C to stop.
int main()
{
    TCPServer server;
    const uint16_t PORT = 8080;

    if (!server.createSocket())
        return 1; // socket()
    if (!server.setReuseAddr())
        return 1; // setsockopt(SO_REUSEADDR)
    if (!server.bindTo(PORT))
        return 1; // bind() to 127.0.0.1:PORT
    if (!server.listenOn())
        return 1; // listen()

    std::cout << "Server starting on port " << PORT << "..." << std::endl;
    server.run(); // blocks forever

    return 0;
}
