#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

#include <functional>
#include <map>
#include <string>
#include <vector>

// A handler takes a request (with any extracted path params already filled in)
// and returns a response. Capturing lambdas (e.g. [&store]) work fine here.
using Handler = std::function<HTTPResponse(const HTTPRequest &)>;

struct Route
{
    std::string method;  // "GET"
    std::string pattern; // "/health" or "/api/items/{id}"
    Handler handler;
};

class Router
{
  public:
    // Register a handler for method+pattern.
    void add(const std::string &method,
             const std::string &pattern,
             Handler handler);

    // Dispatch: find a matching route, run its handler, or produce
    // 404 (unknown path) / 405 (known path, wrong method).
    HTTPResponse handle(const HTTPRequest &request) const;

    // Does `pattern` match `target`? If yes, fill `params` with extracted
    // values (e.g. params["id"] = "42") and return true.
    // Public so it can be unit-tested directly.
    static bool match(const std::string &pattern,
                      const std::string &target,
                      std::map<std::string, std::string> &params);

  private:
    std::vector<Route> routes_;
};

#endif