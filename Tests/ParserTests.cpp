#include "HTTPParser.hpp"
#include "HTTPRequest.hpp"

#include <cassert>
#include <iostream>
#include <string>

// tiny test harness: count failures, print pass/fail, return non-zero on any
// failure so CI / `make test` fails loudly.
static int failures = 0;
#define CHECK(cond)                                                                                                    \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(cond))                                                                                                   \
        {                                                                                                              \
            std::cerr << "FAIL: " << #cond << " (" << __FILE__ << ":" << __LINE__ << ")\n";                            \
            ++failures;                                                                                                \
        }                                                                                                              \
    } while (false)

int main()
{
    // 1. complete request in one feed
    {
        HTTPParser p;
        CHECK(p.feed("GET / HTTP/1.1\r\nHost: x\r\n\r\n") == ParseResult::Complete);
        HTTPRequest r = p.takeRequest();
        CHECK(r.method == "GET");
        CHECK(r.target == "/");
        CHECK(r.version == "HTTP/1.1");
        CHECK(r.headers.at("host") == "x");
    }

    // 2. fragmented across two feeds
    {
        HTTPParser p;
        CHECK(p.feed("GET /api/items/42 HTTP/1.1\r\nHost: localhost\r\n") == ParseResult::Incomplete);
        CHECK(p.feed("X-Custom: hello\r\n\r\n") == ParseResult::Complete);
        HTTPRequest r = p.takeRequest();
        CHECK(r.target == "/api/items/42");
        CHECK(r.headers.at("x-custom") == "hello");
    }

    // 3. malformed: missing colon
    {
        HTTPParser p;
        CHECK(p.feed("GET / HTTP/1.1\r\nBadLine\r\n\r\n") == ParseResult::Error);
    }

    // 4. unsupported version
    {
        HTTPParser p;
        CHECK(p.feed("GET / HTTP/2.0\r\nHost: x\r\n\r\n") == ParseResult::Error);
    }

    // 5. too many headers
    {
        HTTPParser p;
        std::string req = "GET / HTTP/1.1\r\n";
        for (int i = 0; i < 101; ++i)
        {
            req += "H" + std::to_string(i) + ": v\r\n";
        }
        req += "\r\n";
        CHECK(p.feed(req) == ParseResult::Error);
    }

    // 6. case-insensitive header names
    {
        HTTPParser p;
        CHECK(p.feed("GET / HTTP/1.1\r\nContent-Type: text/plain\r\n\r\n") == ParseResult::Complete);
        HTTPRequest r = p.takeRequest();
        CHECK(r.headers.at("content-type") == "text/plain");
    }

    // 7. OWS trimming on header values
    {
        HTTPParser p;
        CHECK(p.feed("GET / HTTP/1.1\r\nHost:   example.com   \r\n\r\n") == ParseResult::Complete);
        HTTPRequest r = p.takeRequest();
        CHECK(r.headers.at("host") == "example.com");
    }

    // 8. empty-ish target rejected
    {
        HTTPParser p;
        CHECK(p.feed("GET  HTTP/1.1\r\nHost: x\r\n\r\n") == ParseResult::Error);
    }

    if (failures == 0)
    {
        std::cout << "all parser tests passed\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed\n";
    return 1;
}
