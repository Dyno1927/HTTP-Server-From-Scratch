# HTTP-Server-From-Scratch — Codex project instructions

- Build the HTTP/1.1 server from raw POSIX sockets and standard C++20; do not introduce third-party networking or HTTP libraries without explicit direction.
- Preserve the `Include/`, `Src/`, `Build/`, and `Docs/` layout and consult `Docs/Roadmap.md` before architectural changes.
- Use `clang++` with C++20 and the existing Makefile targets: `make`, `make run`, and `make san`.
- Use the global `~/.clang-format`; do not add a repository-local formatter configuration.
- Do not include unrelated existing working-tree changes in commits. Documentation follows successful verification.
