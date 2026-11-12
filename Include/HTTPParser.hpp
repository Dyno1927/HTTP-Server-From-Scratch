#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include "HTTPRequest.hpp"

#include <cstddef>

// Incremental HTTP/1.1 request parser.
//
// Usage:
//   HTTPParser parser;
//   auto r = parser.feed(bytes, n);   // Incomplete / Complete / Error
//   if (r == ParseResult::Complete) { auto req = parser.takeRequest(); ... }
//
// The parser keeps its own buffer and state between feed() calls, because a
// single recv() rarely lines up with a message boundary.
class HTTPParser
{
  public:
    // resource limits. reject before allocating unbounded memory.
    static constexpr std::size_t MAX_REQUEST_LINE = 8192;
    static constexpr std::size_t MAX_HEADER_SIZE = 8192;
    static constexpr std::size_t MAX_HEADER_COUNT = 100;
    static constexpr std::size_t MAX_TOTAL_BYTES = 65536;

    HTTPParser();

    // append bytes and advance as far as possible.
    ParseResult feed(const char *data, std::size_t len);
    ParseResult feed(const std::string &data);

    // valid only after feed() returned Complete. moves the request out.
    HTTPRequest takeRequest();

    // valid only after feed() returned Error.
    const std::string &errorReason() const;

    // reset state + buffer so the parser can handle the next request
    // (used later for keep-alive). safe to call after Complete or Error.
    void reset();

  private:
    enum class State
    {
        RequestLine, // reading the request line
        Headers,     // reading header fields
        Complete,    // done; waiting for takeRequest()/reset()
        Error        // malformed; see errorReason_
    };

    State state_ = State::RequestLine;
    std::string buffer_; // unprocessed bytes
    HTTPRequest request_;
    std::string errorReason_;
    std::size_t headerCount_ = 0;

    // try to advance state_ as far as buffer_ allows.
    // returns the new outcome; never consumes past a message boundary.
    ParseResult tryParse();
};

#endif
