#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

// POSIX socket stuff, in the header so any file that includes it compiles alone.
#include <netinet/in.h>
#include <sys/socket.h>

// wraps one listening TCP socket + its accept loop.
// order of calls: createSocket -> setReuseAddr -> bindTo -> listenOn -> run
// run() never comes back; stop() or the destructor closes the socket.
class TCPServer
{
  private:
    int serverFd; // listening fd, -1 = not open yet

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
