#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

// POSIX socket types. Included here so the header stays self-contained.
#include <netinet/in.h>
#include <sys/socket.h>

// Wraps one listening TCP socket and its accept loop.
// Lifecycle (in order): createSocket -> setReuseAddr -> bindTo -> listenOn -> run.
// run() never returns; stop() / the destructor close the listening socket.
class TCPServer
{
  private:
    int serverFd; // listening fd; -1 means "not created yet"

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
