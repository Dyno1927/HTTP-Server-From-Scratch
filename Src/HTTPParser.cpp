#include "HTTPParser.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <utility>

namespace
{
// lowercase a copy of s. header names are case-insensitive, so I canonicalise
// them on storage and look them up lowercased later.
std::string toLower(std::string s)
{
    for (char &c : s)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

// strip leading/trailing OWS (spaces and tabs) from a header value.
// only the ends - internal whitespace is part of the value.
std::string trimOWS(std::string s)
{
    auto isOWS = [](char c) { return c == ' ' || c == '\t'; };
    std::size_t start = 0;

    while (start < s.size() && isOWS(s[start]))
    {
        ++start;
    }

    std::size_t end = s.size();

    while (end > start && isOWS(s[end - 1]))
    {
        --end;
    }
    return s.substr(start, end - start);
}

// RFC 7230 "token" characters: alphanumerics plus a fixed special set.
// used for method and header-name validation.
bool isTokenChar(char c)
{
    unsigned char uc = static_cast<unsigned char>(c);
    if (std::isalnum(uc) != 0)
    {
        return true;
    }
    const char *specials = "!#$%&'*+-.^_`|~";
    return std::strchr(specials, c) != nullptr;
}

bool isToken(const std::string &s)
{
    if (s.empty())
    {
        return false;
    }
    for (char c : s)
    {
        if (!isTokenChar(c))
        {
            return false;
        }
    }
    return true;
}
} // namespace

HTTPParser::HTTPParser() = default;

ParseResult HTTPParser::feed(const char *data, std::size_t len)
{
    // once errored, stay errored until reset(). don't keep buffering junk.
    if (state_ == State::Error)
    {
        return ParseResult::Error;
    }
    if (state_ == State::Complete)
    {
        return ParseResult::Complete;
    }

    // enforce the total-bytes cap BEFORE appending. a client streaming megabytes
    // with no newlines must not grow memory unbounded.
    if (buffer_.size() + len > MAX_TOTAL_BYTES)
    {
        errorReason_ = "request exceeds total size limit";
        state_ = State::Error;
        return ParseResult::Error;
    }

    buffer_.append(data, len);
    return tryParse();
}

ParseResult HTTPParser::feed(const std::string &data)
{
    return feed(data.data(), data.size());
}

HTTPRequest HTTPParser::takeRequest()
{
    // move the parsed request out and reset so the parser is ready for the
    // next request on the same connection (keep-alive, Phase 6).
    HTTPRequest out = std::move(request_);
    reset();
    return out;
}

const std::string &HTTPParser::errorReason() const
{
    return errorReason_;
}

void HTTPParser::reset()
{
    state_ = State::RequestLine;
    buffer_.clear();
    request_ = HTTPRequest{};
    errorReason_.clear();
    headerCount_ = 0;
}

ParseResult HTTPParser::tryParse()
{
    // drive the state machine as far as buffer_ allows. every iteration either
    // consumes a complete line and loops, or returns (Incomplete/Error/Complete).
    while (true)
    {
        if (state_ == State::RequestLine)
        {
            // need a CRLF to terminate the request line.
            std::size_t end = buffer_.find("\r\n");
            if (end == std::string::npos)
            {
                // no terminator yet. incomplete unless the line is already too
                // long (a client sending bytes with no newline forever).
                if (buffer_.size() > MAX_REQUEST_LINE)
                {
                    errorReason_ = "request line too long";
                    state_ = State::Error;
                    return ParseResult::Error;
                }
                return ParseResult::Incomplete;
            }

            // terminator found. reject an over-long line even though it ended.
            if (end > MAX_REQUEST_LINE)
            {
                errorReason_ = "request line too long";
                state_ = State::Error;
                return ParseResult::Error;
            }

            std::string line = buffer_.substr(0, end);
            // consume the line AND its CRLF.
            buffer_.erase(0, end + 2);

            // split into exactly 3 tokens: METHOD SP TARGET SP VERSION
            std::size_t sp1 = line.find(' ');
            if (sp1 == std::string::npos)
            {
                errorReason_ = "malformed request line (missing spaces)";
                state_ = State::Error;
                return ParseResult::Error;
            }
            std::size_t sp2 = line.find(' ', sp1 + 1);
            if (sp2 == std::string::npos)
            {
                errorReason_ = "malformed request line (missing version)";
                state_ = State::Error;
                return ParseResult::Error;
            }
            // a third space means too many tokens.
            if (line.find(' ', sp2 + 1) != std::string::npos)
            {
                errorReason_ = "malformed request line (extra spaces)";
                state_ = State::Error;
                return ParseResult::Error;
            }

            std::string method = line.substr(0, sp1);
            std::string target = line.substr(sp1 + 1, sp2 - sp1 - 1);
            std::string version = line.substr(sp2 + 1);

            if (!isToken(method))
            {
                errorReason_ = "invalid method";
                state_ = State::Error;
                return ParseResult::Error;
            }
            if (target.empty())
            {
                errorReason_ = "empty request target";
                state_ = State::Error;
                return ParseResult::Error;
            }
            // target must not contain control characters.
            for (char c : target)
            {
                unsigned char uc = static_cast<unsigned char>(c);
                if (uc < 0x20 || uc == 0x7F)
                {
                    errorReason_ = "invalid character in request target";
                    state_ = State::Error;
                    return ParseResult::Error;
                }
            }
            if (version != "HTTP/1.0" && version != "HTTP/1.1")
            {
                errorReason_ = "unsupported HTTP version";
                state_ = State::Error;
                return ParseResult::Error;
            }

            request_.method = method;
            request_.target = target;
            request_.version = version;

            state_ = State::Headers;
            // loop: try to parse headers now, same feed() call.
        }
        else if (state_ == State::Headers)
        {
            std::size_t end = buffer_.find("\r\n");
            if (end == std::string::npos)
            {
                // no terminator for the current header line.
                if (buffer_.size() > MAX_HEADER_SIZE)
                {
                    errorReason_ = "headers too large";
                    state_ = State::Error;
                    return ParseResult::Error;
                }
                return ParseResult::Incomplete;
            }

            // blank line (CRLF immediately) = end of headers.
            if (end == 0)
            {
                buffer_.erase(0, 2);
                state_ = State::Complete;
                return ParseResult::Complete;
            }

            std::string line = buffer_.substr(0, end);
            buffer_.erase(0, end + 2);

            // must have a colon separating name and value.
            std::size_t colon = line.find(':');
            if (colon == std::string::npos || colon == 0)
            {
                errorReason_ = "malformed header (missing or empty name)";
                state_ = State::Error;
                return ParseResult::Error;
            }

            std::string name = line.substr(0, colon);
            std::string value = trimOWS(line.substr(colon + 1));

            if (!isToken(name))
            {
                errorReason_ = "invalid header name";
                state_ = State::Error;
                return ParseResult::Error;
            }

            // per-header size cap.
            if (name.size() + value.size() > MAX_HEADER_SIZE)
            {
                errorReason_ = "header too large";
                state_ = State::Error;
                return ParseResult::Error;
            }

            // header count cap.
            if (headerCount_ >= MAX_HEADER_COUNT)
            {
                errorReason_ = "too many headers";
                state_ = State::Error;
                return ParseResult::Error;
            }

            // canonicalise name to lowercase for case-insensitive lookup.
            request_.headers[toLower(name)] = value;
            ++headerCount_;

            // loop: maybe more headers, or the terminating blank line, is
            // already in buffer_.
        }
        else if (state_ == State::Complete)
        {
            return ParseResult::Complete;
        }
        else // State::Error
        {
            return ParseResult::Error;
        }
    }
}
