#include "Router.hpp"

namespace
{
// small helper: build a plain-text error response with correct framing.
HTTPResponse makeError(int code, const std::string &reason, const std::string &body)
{
    HTTPResponse response;
    response.statusCode = code;
    response.reason = reason;
    response.body = body;
    response.headers["Content-Type"] = "text/plain";
    response.headers["Content-Length"] = std::to_string(response.body.size());
    response.headers["Connection"] = "close";
    return response;
}
} // namespace

void Router::add(const std::string &method, const std::string &pattern, Handler handler)
{
    routes_.push_back({method, pattern, std::move(handler)});
}

HTTPResponse Router::handle(const HTTPRequest &request) const
{
    bool pathSeen = false;
    bool found = false;
    Handler foundHandler;
    std::map<std::string, std::string> foundParams;

    for (const auto &route : routes_)
    {
        std::map<std::string, std::string> params;
        if (match(route.pattern, request.target, params))
        {
            pathSeen = true;
            if (route.method == request.method)
            {
                found = true;
                foundHandler = route.handler;
                foundParams = params;
                break;
            }
        }
    }

    if (found)
    {
        HTTPRequest req = request; // copy so we don't mutate the const ref
        req.params = foundParams;
        return foundHandler(req);
    }

    if (pathSeen)
    {
        return makeError(405, "Method Not Allowed", "Method Not Allowed\n");
    }
    return makeError(404, "Not Found", "Not Found\n");
}

bool Router::match(const std::string &pattern, const std::string &target, std::map<std::string, std::string> &params)
{
    // clear any stale entries from a previous (possibly failed) call.
    params.clear();

    auto splitPath = [](const std::string &path)
    {
        std::vector<std::string> segments;
        size_t start = 0;
        size_t end = path.find('/');

        // walk every slash; push the segment before it, then advance.
        while (end != std::string::npos)
        {
            segments.push_back(path.substr(start, end - start));
            start = end + 1;
            end = path.find('/', start);
        }
        // push the trailing segment (empty string for a trailing slash).
        segments.push_back(path.substr(start));

        return segments;
    };

    std::vector<std::string> patternSeg = splitPath(pattern);
    std::vector<std::string> targetSeg = splitPath(target);

    // segment counts must line up. a trailing slash on the target now leaves
    // an empty final segment, so "/api/items/" no longer collapses to 2.
    if (patternSeg.size() != targetSeg.size())
    {
        return false;
    }

    for (size_t i = 0; i < patternSeg.size(); ++i)
    {
        const std::string &pSeg = patternSeg[i];
        const std::string &tSeg = targetSeg[i];

        if (pSeg.size() >= 2 && pSeg.front() == '{' && pSeg.back() == '}')
        {
            std::string name = pSeg.substr(1, pSeg.size() - 2);
            if (name.empty() || tSeg.empty())
            {
                return false;
            }
            params[name] = tSeg;
        }
        else
        {
            if (pSeg != tSeg)
            {
                return false;
            }
        }
    }

    return true;
}
