#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <map>
#include <string>

// One parsed HTTP request. Phase 3 scope: request line + headers only.
// The body arrives in Phase 5 (message framing), so there is no body field yet.
struct HTTPRequest
{
    std::string method;  // e.g. "GET"
    std::string target;  // origin-form request-target, e.g. "/api/items/42"
    std::string version; // e.g. "HTTP/1.1"

    // Header names are stored lowercased so lookups are case-insensitive.
    std::map<std::string, std::string> headers;
};

// Outcome of feeding bytes to the parser.
//   Incomplete -> need more bytes; not an error, just wait for next recv()
//   Complete   -> full request line + headers parsed; request_ is ready
//   Error      -> malformed input; see errorReason()
enum class ParseResult
{
    Incomplete,
    Complete,
    Error
};

#endif
