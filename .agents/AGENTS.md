# Shared agent instructions — HTTP-Server-From-Scratch

These instructions are shared by Codex, AGY, OpenCode, and other compatible agents.

## Working agreements

- Answer the requested question or task directly and avoid unrelated features or cleanup.
- Distinguish known facts, inferences, assumptions, and hypotheses.
- Preserve existing architecture and external contracts.
- Use Pascal-Kebab-Case directories, PascalCase files, camelCase functions and variables, and PascalCase types unless tooling, APIs, schemas, or language conventions require otherwise.
- Plan before large, multi-file, architectural, or risky changes; execute small clear changes directly.
- Run relevant tests, builds, linters, formatters, and type checks after implementation.
- Documentation follows successful verification.
- Never expose credentials, tokens, or private configuration.
- Local commits are permitted; pull, push, published merges, and destructive Git operations require explicit approval.

## Project memory

- Build the HTTP/1.1 server from raw POSIX sockets and standard C++20; do not introduce third-party networking or HTTP libraries without explicit direction.
- Preserve the `Include/`, `Src/`, `Build/`, and `Docs/` layout and consult `Docs/Roadmap.md` before architectural changes.
- Use `clang++` with C++20 and the existing Makefile targets: `make`, `make run`, and `make san`.
- Use the global `~/.clang-format`; do not add a repository-local formatter configuration.
