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
Build/      compiler output (gitignored, created by make)
Makefile    build + run + sanitizer targets
```

## Build

Everything builds with Clang and C++20:

```sh
make          # build Build/Server
make run      # build, then run it
make san      # rebuild with AddressSanitizer + UndefinedBehaviorSanitizer
make clean    # remove Build/
```

If GCC is the only compiler you have, `make CXX=g++` works too.

## Branches

- `stable` — stable. Only merged, working code lands here.
- `dev` — developing. Day-to-day work and experiments happen here; merged into
  `stable` once a phase's completion criteria are met.

I keep `stable` green and do all work on `dev`.

## CI

GitHub Actions (`.github/workflows/Build.yml`) runs on every push and pull
request to `stable` (and `dev`): a matrix build with both Clang and GCC, plus a sanitizer
build (ASan + UBSan). A green check means the code compiles on both compilers
and stays sanitizer-clean.

## Status

Scaffold only — there is no server code in here yet. The CI workflow builds
whatever lands in `Src/` and runs a sanitizer pass, so the first real commit
gets checked automatically.

## License

MIT — see [LICENSE](LICENSE). Take it, fork it, ship it, use it in whatever you
like, commercial or not. The one thing the license asks is that you keep the
copyright notice and credit me, Dyno, as the original author.

---

Built by [Dyno](https://github.com/Dyno1927).
