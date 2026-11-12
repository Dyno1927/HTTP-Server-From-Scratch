# HTTP-Server-From-Scratch

I'm writing an HTTP server in C++ from scratch — no framework, no library that
already speaks HTTP for me. I want to know what actually happens between a
browser typing a URL and the bytes coming back, so I'm building the thing
instead of reading about the thing.

This repo is the scaffold: the layout, the build, and the CI are ready. The
server itself starts here and grows one piece at a time.

## What I want out of it

- A TCP listener that accepts connections and reads real bytes off a socket.
- An HTTP/1.1 parser: request line, headers, body — including the parts that
  are easy to get wrong (`Content-Length`, chunked encoding, header folding).
- A response writer with correct status lines, headers, and body framing.
- Keep-alive, then concurrency, then the boring but necessary bits: timeouts,
  limits, and not falling over on a malformed request.

I'm learning in public. Progress lands as commits, not promises.

## Layout

```
Include/    my headers
Src/        my implementation
Tests/      unit tests
Build/      compiler output (gitignored, created by make)
Makefile    build + run + sanitizer + test targets
```

## Build

Everything builds with Clang and C++20:

```sh
make          # build Build/Server
make run      # build, then run it (listens on 127.0.0.1:8080)
make test     # build and run the unit tests (Tests/)
make san      # rebuild with AddressSanitizer + UndefinedBehaviorSanitizer
make clean    # remove Build/
```

If GCC is the only compiler you have, `make CXX=g++` works too (and
`make CXX=g++ test` for tests).

## Run

```sh
make run
```

Then in another terminal:

```sh
# valid request
printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc 127.0.0.1 8080

# fragmented (two halves with a delay)
(printf 'GET / HTTP/1.1\r\n'; sleep 0.3; printf 'Host: x\r\n\r\n') | nc 127.0.0.1 8080

# malformed -> 400 Bad Request
printf 'GET / HTTP/1.1\r\nBadLine\r\n\r\n' | nc 127.0.0.1 8080
```

The server binds to loopback only and handles one connection at a time.
Press Ctrl+C to stop.

## Branches

- `stable` — stable. Only merged, working code lands here.
- `dev` — developing. Day-to-day work and experiments happen here; merged into
  `stable` once a phase's completion criteria are met.

I keep `stable` green and do all work on `dev`.

## CI

GitHub Actions (`.github/workflows/Build.yml`) runs on every push and pull
request to `stable` (and `dev`): a matrix build with both Clang and GCC, the
unit tests (`make test`), and a sanitizer build (ASan + UBSan). A green check
means the code compiles on both compilers, the tests pass, and it stays
sanitizer-clean.

## Status

Phase 3 complete. The server accepts TCP connections, parses an HTTP/1.1
request line + headers incrementally (handling fragmented input), and returns
`200 OK` for valid requests or `400 Bad Request` for malformed ones. Builds
clean on both Clang and GCC, sanitizer-clean (ASan + UBSan).

Previous phases: Phase 1 (TCP listener) and Phase 2 (valid HTTP response) are
tagged `phase-1` and `phase-2`. Next up: Phase 4 (first REST API).

## License

MIT — see [LICENSE](LICENSE). Take it, fork it, ship it, use it in whatever you
like, commercial or not. The one thing the license asks is that you keep the
copyright notice and credit me, Dyno, as the original author.

---

Built by [Dyno](https://github.com/Dyno1927).
