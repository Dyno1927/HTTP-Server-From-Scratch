#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <map>
#include <string>

// Represents one HTTP response: status line, headers, body.
// Phase 2 goal: build this, serialize it, send it over the TCP socket.
//
// Wire format (CRLF = "\r\n"):
//   HTTP/1.1 <status-code> <reason-phrase>\r\n
//   <Header-Name>: <value>\r\n
//   ...
//   \r\n
//   <body>
//
// Content-Length must equal body.size() (byte count). Connection: close
// means the server closes after sending.
struct HTTPResponse
{
    int statusCode = 200;
    std::string reason = "OK";
    std::map<std::string, std::string> headers;
    std::string body;

    // Serialize to the wire format described above.
    std::string toString() const;
};

#endif
