#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

// POSIX socket stuff, in the header so any file that includes it compiles alone.
#include <netinet/in.h>
#include <sys/socket.h>

#include "Item.hpp"
#include "Router.hpp"

#include <map>

// wraps one listening TCP socket + its accept loop.
// order of calls: createSocket -> setReuseAddr -> bindTo -> listenOn -> run
// run() never comes back; stop() or the destructor closes the socket.
class TCPServer
{
  private:
    int serverFd; // listening fd, -1 = not open yet

    // in-memory store seeded at startup. declared before router_ so the
    // handlers (which capture it by reference) see a live object.
    std::map<int, Item> store_;

    Router router_;

    // handle one accepted connection end to end: read, parse, respond, close.
    void handleClient(int clientFd);

    // seed store_ and register routes. called from the constructor.
    void setupRoutes();

  public:
    TCPServer();
    ~TCPServer();

    bool createSocket();                    // socket()
    bool setReuseAddr();                    // setsockopt(SO_REUSEADDR)
    bool bindTo(int port);                  // bind() to 127.0.0.1:port
    bool listenOn(int backlog = SOMAXCONN); // listen()
    int acceptClient();                     // accept() one client
    void run();                             // blocking accept loop
    void stop();                            // close() the listener
};

#endif
