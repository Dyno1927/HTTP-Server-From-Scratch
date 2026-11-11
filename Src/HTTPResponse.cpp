#include "HTTPResponse.hpp"

// builds the raw HTTP bytes. get the \r\n right or the client gets confused.
std::string HTTPResponse::toString() const
{
    std::string output;

    // status line: "HTTP/1.1 200 OK\r\n"
    output += "HTTP/1.1 " + std::to_string(statusCode) + " " + reason + "\r\n";

    // headers: "Name: value\r\n". colon then one space, not the other way round.
    for (const auto &pair : headers)
    {
        output += pair.first + ": " + pair.second + "\r\n";
    }

    // blank line = end of headers. skip this and the client can't find the body.
    output += "\r\n";

    // body last, no trailing newline - Content-Length already said how many bytes.
    output += body;

    return output;
}
