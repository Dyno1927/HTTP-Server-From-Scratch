# Testing

## Unit tests

```sh
make test
```

Builds `Build/ParserTests` from `Tests/ParserTests.cpp` + `Src/HTTPParser.cpp`
and runs it. Returns non-zero on any failure.

## Sanitizers

```sh
make san          # ASan + UBSan build of the server
make clean
make CXX=clang++ CXXFLAGS="-std=c++20 -Wall -Wextra -g -O0 -IInclude -fsanitize=address,undefined" test
```

## Manual / protocol tests

Start the server with `make run`, then in another terminal:

```sh
# valid request
printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc 127.0.0.1 8080

# fragmented
(printf 'GET / HTTP/1.1\r\n'; sleep 0.3; printf 'Host: x\r\n\r\n') | nc 127.0.0.1 8080

# malformed -> 400
printf 'GET / HTTP/1.1\r\nBadLine\r\n\r\n' | nc 127.0.0.1 8080
```

## CI

`.github/workflows/Build.yml` runs the matrix build, `make test`, and the
sanitizer build on every push/PR to `stable` and `dev`.
