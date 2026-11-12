# HTTP Scope — what this server implements

Current phase: **Phase 3 (request parsing)**. Everything below is what the
code in `Src/` actually does today. Planned features are not listed here.

## Implemented

- TCP listener on `127.0.0.1:8080`, one connection at a time.
- Request line parsing: `METHOD SP request-target SP HTTP-version CRLF`.
  Accepted versions: `HTTP/1.0`, `HTTP/1.1`.
- Header parsing: `name: value` pairs, case-insensitive names (stored
  lowercased), OWS trimming on values.
- Incremental parsing: handles fragmented input across multiple `recv()`
  calls; distinguishes `Incomplete` (need more bytes) from `Error`.
- Resource limits: request line 8 KB, per header 8 KB, 100 headers max,
  64 KB total.
- Responses: `200 OK` ("Hello World!") on a valid request, `400 Bad Request`
  on parse failure or early client close.
- Correct framing: status line, headers, blank line, body; `Content-Length`
  matches body size; `Connection: close`.

## Not implemented yet

- Request body (Phase 5).
- `Content-Length` / chunked transfer coding (Phase 5).
- Keep-alive / persistent connections (Phase 6).
- Routing / REST endpoints (Phase 4).
- Concurrency (Phase 10).
- Static files, TLS, HTTP/2, WebSockets (later phases).

## Known limitations

- One client at a time; a slow client blocks everyone.
- No timeouts; a client that sends bytes slowly can hold a connection open.
- No request body handling: any bytes after the header blank line are ignored.
- `HTTP/1.0` is accepted but treated like `HTTP/1.1` (no version-specific
  behaviour yet).
- Error responses are generic; no detail is echoed back to the client.
