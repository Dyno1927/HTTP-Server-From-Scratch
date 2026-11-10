#include "TCPServer.hpp"
#include <iostream>

using namespace std;

// Entry point: wire the socket lifecycle in order, then run the accept loop.
// Stop with Ctrl+C (graceful shutdown is a later phase).
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

    cout << "Server starting on port " << PORT << "..." << endl;
    server.run(); // runs forever

    return 0;
}
