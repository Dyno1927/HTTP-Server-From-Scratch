#include "Router.hpp"

#include <iostream>
#include <string>

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
    // --- match() ---

    // exact match, no params
    {
        std::map<std::string, std::string> p;
        CHECK(Router::match("/health", "/health", p) == true);
        CHECK(p.empty());
    }

    // param extraction
    {
        std::map<std::string, std::string> p;
        CHECK(Router::match("/api/items/{id}", "/api/items/42", p) == true);
        CHECK(p.count("id") == 1);
        CHECK(p.at("id") == "42");
    }

    // reject empty id (trailing slash)
    {
        std::map<std::string, std::string> p;
        CHECK(Router::match("/api/items/{id}", "/api/items/", p) == false);
    }

    // reject extra trailing segment
    {
        std::map<std::string, std::string> p;
        CHECK(Router::match("/api/items/{id}", "/api/items/42/extra", p) == false);
    }

    // reject unknown static segment
    {
        std::map<std::string, std::string> p;
        CHECK(Router::match("/api/items/{id}", "/api/other/42", p) == false);
    }

    // params cleared on failure
    {
        std::map<std::string, std::string> p;
        p["stale"] = "value";
        CHECK(Router::match("/health", "/nope", p) == false);
        CHECK(p.empty());
    }

    // --- handle() ---

    // known route returns handler's response
    {
        Router r;
        r.add("GET", "/health",
              [](const HTTPRequest &)
              {
                  HTTPResponse res;
                  res.statusCode = 200;
                  res.reason = "OK";
                  res.body = "ok";
                  return res;
              });
        HTTPRequest req;
        req.method = "GET";
        req.target = "/health";
        HTTPResponse res = r.handle(req);
        CHECK(res.statusCode == 200);
        CHECK(res.body == "ok");
    }

    // unknown path -> 404
    {
        Router r;
        r.add("GET", "/health",
              [](const HTTPRequest &)
              {
                  HTTPResponse res;
                  res.statusCode = 200;
                  res.body = "ok";
                  return res;
              });
        HTTPRequest req;
        req.method = "GET";
        req.target = "/nope";
        HTTPResponse res = r.handle(req);
        CHECK(res.statusCode == 404);
    }

    // known path, wrong method -> 405
    {
        Router r;
        r.add("GET", "/health",
              [](const HTTPRequest &)
              {
                  HTTPResponse res;
                  res.statusCode = 200;
                  res.body = "ok";
                  return res;
              });
        HTTPRequest req;
        req.method = "POST";
        req.target = "/health";
        HTTPResponse res = r.handle(req);
        CHECK(res.statusCode == 405);
    }

    // param reaches handler
    {
        Router r;
        r.add("GET", "/api/items/{id}",
              [](const HTTPRequest &req)
              {
                  HTTPResponse res;
                  res.statusCode = 200;
                  res.body = "id=" + req.params.at("id");
                  return res;
              });
        HTTPRequest req;
        req.method = "GET";
        req.target = "/api/items/42";
        HTTPResponse res = r.handle(req);
        CHECK(res.statusCode == 200);
        CHECK(res.body == "id=42");
    }

    if (failures == 0)
    {
        std::cout << "all router tests passed\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed\n";
    return 1;
}
