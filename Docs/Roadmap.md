# HTTP Server From Scratch
## Project Roadmap and Technical Design

| Property | Details |
| :--- | :--- |
| **Project** | HTTP-Server-From-Scratch |
| **Language** | Modern C++ |
| **Compiler** | Clang++ |
| **Build System** | CMake |
| **Networking** | Raw operating-system sockets |
| **Initial Platform** | Linux / POSIX |
| **Protocol Target** | HTTP/1.1 |
| **Initial Application** | REST API server |
| **Long-Term Goal** | A reusable, secure, high-performance C++ HTTP server library and framework |

---

## Table of Contents
1. [Vision](#1-vision)
2. [Guiding Principles](#2-guiding-principles)
3. [Initial Technical Decisions](#3-initial-technical-decisions)
4. [Architecture Direction](#4-architecture-direction)
5. [Progressive Roadmap](#5-progressive-roadmap)
   - [Phase 0 — Development Foundation](#phase-0--development-foundation)
   - [Phase 1 — First TCP Server](#phase-1--first-tcp-server)
   - [Phase 2 — First Valid HTTP Response](#phase-2--first-valid-http-response)
   - [Phase 3 — Request Parsing](#phase-3--request-parsing)
   - [Phase 4 — First REST API](#phase-4--first-rest-api)
   - [Phase 5 — Correct HTTP Message Framing](#phase-5--correct-http-message-framing)
   - [Phase 6 — Connection Management and Keep-Alive](#phase-6--connection-management-and-keep-alive)
   - [Phase 7 — Static Files and Practical Server Features](#phase-7--static-files-and-practical-server-features)
   - [Phase 8 — Testing and Security Hardening](#phase-8--testing-and-security-hardening)
   - [Phase 9 — Reusable C++ Library API](#phase-9--reusable-c-library-api)
   - [Phase 10 — Concurrency and Performance](#phase-10--concurrency-and-performance)
   - [Phase 11 — Production-Oriented Readiness](#phase-11--production-oriented-readiness)
6. [Testing Strategy](#6-testing-strategy)
7. [Security Requirements](#7-security-requirements)
8. [CMake and Repository Organization](#8-cmake-and-repository-organization)
9. [Documentation Requirements](#9-documentation-requirements)
10. [Definition of Done](#10-definition-of-done)
11. [Deferred Features](#11-deferred-features)
12. [Immediate Next Steps](#12-immediate-next-steps)
13. [Final Principle](#13-final-principle)

---

## 1. Vision
Build an HTTP server from the ground up to understand how networking, HTTP, operating systems, memory management, concurrency, and server architecture work together.

The project begins as a small educational REST API server and progressively evolves into a reusable HTTP server library.

The intention is not to implement every feature immediately. Each feature must be introduced when its prerequisites are understood, implemented in a manageable form, and validated through tests and experiments.

The final library should aim to be:
* **Correct:** Handle supported HTTP behavior according to the protocol.
* **Secure:** Reject malformed input, enforce resource limits, and avoid common server vulnerabilities.
* **Reliable:** Handle client disconnects, socket errors, partial I/O, and resource exhaustion.
* **Understandable:** Have explicit abstractions and a documented architecture.
* **Reusable:** Allow other C++ applications to create and configure HTTP servers.
* **Testable:** Support automated unit, integration, protocol, and stress testing.
* **Performant:** Demonstrate measurable throughput, latency, and concurrency characteristics.
* **Maintainable:** Use modern C++, clear ownership, a modular design, and a consistent public API.

*Performance is a goal to measure, not an assumption to make.*

---

## 2. Guiding Principles

### 2.1. No Networking Framework
Do not use Asio, Boost.Asio, libuv, or an HTTP server library to implement the core networking and HTTP protocol. Use operating-system socket APIs directly.

Third-party tools may be evaluated later for testing, benchmarking, or comparison. Any such dependency must be documented and must not replace the core implementation being learned.

### 2.2. Learn by Implementation
For every milestone:
1. Study the necessary concepts.
2. Write a small implementation.
3. Compile it with Clang++.
4. Run it and observe its behavior.
5. Deliberately test failure cases.
6. Write automated tests.
7. Debug and fix the implementation.
8. Document the behavior and design decisions.

Move forward only after the completion criteria are satisfied. Avoid copying a complete server implementation. The purpose is to understand *why* the implementation works.

### 2.3. Prefer the Simplest Correct Implementation
Start with a small synchronous server. Do not introduce asynchronous I/O, complex allocators, lock-free data structures, or custom thread pools merely because they are associated with high performance. Introduce complexity only when the underlying concepts are understood and measurements justify it.

### 2.4. Separate Learning Exercises from Production Requirements
A minimal educational implementation may deliberately support only a subset of HTTP/1.1. Every limitation must be documented.

The server must not claim general HTTP/1.1 compliance until its supported behavior, parser, connection handling, and relevant protocol requirements have been validated. Never expose an intentionally incomplete server to untrusted networks.

---

## 3. Initial Technical Decisions

### 3.1. Platform
Recommended starting point: **Linux using POSIX sockets**.

This fits the current Arch Linux development environment and exposes the fundamental networking APIs directly.

#### Initial Socket Interfaces
```text
socket()
setsockopt()
bind()
listen()
accept()
recv()
send()
shutdown()
close()
```

#### Potential Linux-Specific Interfaces
Introduce these later, when needed:
```text
fcntl()
poll()
epoll()
accept4()
eventfd()
```

Portability remains an open architectural concern. The initial implementation should keep platform-specific socket operations behind an internal boundary so a future Windows implementation can use Winsock without rewriting HTTP parsing and routing. Do not build a cross-platform abstraction prematurely.

### 3.2. Language and Compiler
Use **modern C++** with **Clang++**. Adopt **C++20** as the provisional language baseline, subject to confirming the installed compiler and CMake versions. C++20 offers useful language and library features without forcing the project to use every modern feature.

#### Recommended Practices
* Use RAII for resource ownership.
* Prefer standard containers and strings.
* Define explicit ownership and lifetime rules.
* Use smart pointers where shared or dynamic ownership is justified.
* Use `std::span` where appropriate for non-owning byte ranges.
* Clearly separate owning and non-owning data.
* Avoid raw owning pointers and manual memory management when RAII provides a simpler solution.
* Use raw pointers for non-owning references where appropriate.

### 3.3. Build System
Use **CMake** from the beginning. The build should support:
* A server executable for early experiments.
* A reusable library target.
* Unit tests.
* Integration tests.
* Optional sanitizers.
* Debug and optimized builds.
* Benchmarks as a later optional target.

Keep generated build artifacts out of version control.

### 3.4. Dependencies
The initial core should rely on the C++ standard library and operating-system APIs. Potential later dependencies should be evaluated individually:

| Dependency Type | Purpose | Introduction Point |
| :--- | :--- | :--- |
| **JSON library** | JSON parsing and serialization | REST API implementation |
| **Testing framework** | Simplify automated tests | When a custom test harness becomes cumbersome |
| **Benchmarking tool** | Repeatable performance measurements | Performance phase |

Dependencies must not obscure the HTTP implementation. JSON parsing is an application-layer concern, not a fundamental requirement of HTTP.

### 3.5. Protocol Scope
Target **HTTP/1.1** first. Start with a deliberately limited subset and expand it progressively.

Important protocol topics include:
* Request lines and status lines.
* Header fields.
* Message framing.
* Request and response bodies.
* `Content-Length`.
* Chunked transfer coding.
* Persistent connections.
* Connection closure.
* Request-target forms.
* HTTP methods and status codes.
* Header field semantics.
* `Host`.
* `Expect: 100-continue`.
* Request validation and malformed messages.

HTTP/2 and HTTP/3 are future explorations, not initial requirements.

---

## 4. Architecture Direction
The architecture will evolve with the implementation. The following is a target direction, not a requirement to create every component immediately.

```text
       [ Application ]
              │
    [ Public Server API ]
              │
 [ Router & Request Handlers ]
              │
 [ HTTP Request/Response Model ]
              │
  [ HTTP Parser & Serializer ]
              │
   [ Connection Management ]
              │
      [ Socket Transport ]
              │
     [ Operating System ]
```

Cross-cutting concerns will eventually include configuration, logging, error reporting, resource limits, testing, and observability.

### 4.1. Architectural Responsibilities

| Layer | Responsibility |
| :--- | :--- |
| **Transport** | Socket creation, accepting connections, sending and receiving bytes, and reporting transport errors. |
| **HTTP Protocol** | Parsing requests, validating message framing, representing requests and responses, and serializing responses. |
| **Application** | Routing, REST handlers, JSON payloads, and application-specific behavior. |
| **Runtime** | Connection lifecycle, concurrency, timeouts, and server shutdown. |
| **Public Library API** | Interfaces that applications use to configure and operate the server. |

Do not implement these layers all at once. Establish boundaries only when they solve a real problem in the current milestone.

---

## 5. Progressive Roadmap

### Phase 0 — Development Foundation
#### Learn
* Compilation and linking.
* CMake targets and build directories.
* Header files and source files.
* Debug and optimized build configurations.
* Basic command-line debugging.

#### Implement
* Verify the existing repository builds with Clang++ and CMake.
* Create a minimal executable.
* Establish a repeatable build command.
* Add a basic test target when appropriate.

#### Validate
* Configure and build from a clean build directory.
* Confirm the expected compiler is being used.
* Run the executable.
* Verify compiler warnings are visible.
* Confirm build output is not committed to Git.

#### Completion Criteria
The project builds reliably and has a documented development workflow.

---

### Phase 1 — First TCP Server
#### Learn
* Client-server networking.
* IP addresses and ports.
* TCP connections and byte streams.
* File descriptors.
* Socket lifecycle.
* Blocking system calls.
* Basic error handling.
* The difference between TCP and HTTP.

#### Implement
Create a server that:
1. Creates a TCP socket.
2. Binds to a configurable local address and port.
3. Listens for incoming connections.
4. Accepts one client.
5. Receives bytes.
6. Sends a simple response.
7. Closes the connection correctly.

At first, the response can be plain text. The objective is to understand TCP before introducing HTTP.

#### Test
* Connect using a command-line TCP client.
* Connect using a browser or a basic client.
* Disconnect normally.
* Disconnect abruptly.
* Attempt to start two servers on the same port.
* Test an invalid bind address.
* Check system-call failures.

#### Completion Criteria
The server accepts a connection, receives bytes, sends bytes, and handles ordinary errors without crashing.

---

### Phase 2 — First Valid HTTP Response
#### Learn
* HTTP message structure.
* Start lines and header fields.
* CRLF line endings.
* Header termination.
* Status codes.
* Response body framing.
* `Content-Length`.
* `Content-Type`.

#### Implement
Return a minimal valid HTTP response, for example:

```http
HTTP/1.1 200 OK
Content-Type: text/plain
Content-Length: 12
Connection: close

Hello World!
```

The body length must be computed from the actual bytes sent. Implement a response representation rather than assembling increasingly complex responses through scattered string concatenations.

#### Test
* Request the server with `curl -v`.
* Verify the status code and headers.
* Verify the body length.
* Test an empty body.
* Test a client disconnect during the response.

#### Completion Criteria
The server reliably returns correctly framed responses for the cases it supports.

---

### Phase 3 — Request Parsing
#### Learn
* HTTP request-line syntax.
* Methods and request targets.
* HTTP versions.
* Header field syntax.
* Header field case-insensitivity.
* Request boundaries.
* TCP partial reads.
* The difference between a TCP read and a complete HTTP message.
* Input validation and parser state.

#### Implement
Introduce a request model and an incremental parser. The parser must **not** assume that:
* One `recv()` call contains a complete request.
* A request arrives in one packet.
* Headers arrive together.
* The body arrives with the headers.
* A single read corresponds to one HTTP message.

Start with the request line and headers. Add request bodies only after header parsing is stable.

Define explicit limits for:
* Request-line size.
* Header size.
* Header count.
* Total bytes accepted.

#### Test
* Valid requests.
* Missing request components.
* Unsupported HTTP versions.
* Empty input.
* Incomplete request lines.
* Incomplete headers.
* Multiple headers.
* Header names with different capitalization.
* Very large headers.
* Deliberately fragmented input.
* Malformed line endings.

#### Completion Criteria
The parser can handle fragmented input, distinguish incomplete input from invalid input, enforce limits, and produce a predictable result.

---

### Phase 4 — First REST API
#### Learn
* HTTP methods.
* Routes and path matching.
* Status-code semantics.
* JSON as a representation format.
* Request and response bodies.
* Application errors versus protocol errors.

#### Implement
Create a minimal API with a few endpoints:

| Method | Endpoint | Purpose |
| :--- | :--- | :--- |
| `GET` | `/health` | Report server health |
| `GET` | `/api/items` | Return a collection of items |
| `GET` | `/api/items/{id}` | Retrieve an item by ID |

Initially, the data may be stored in memory. Introduce a small handler interface and a simple route registry. Do not implement authentication, databases, or a sophisticated routing DSL yet.

JSON support can initially be minimal, but a maintained JSON library is preferable once actual JSON parsing and serialization are needed.

#### Test
* Known route vs. unknown route.
* Supported and unsupported methods.
* Missing resource.
* Invalid route parameter.
* Valid and invalid JSON.
* Empty request body.
* Appropriate status codes.
* Correct response content types.

#### Completion Criteria
The server can expose a useful REST API while keeping HTTP protocol handling separate from application logic.

---

### Phase 5 — Correct HTTP Message Framing
#### Learn
* `Content-Length`.
* Chunked transfer coding.
* Persistent connections.
* Connection semantics.
* Message completion.
* Conflicting or ambiguous framing.
* HTTP request smuggling risks.

#### Implement
Expand the parser to handle the chosen message-framing rules correctly.
* Do not treat `Transfer-Encoding` and `Content-Length` as interchangeable.
* Define how unsupported transfer codings and malformed or conflicting framing are rejected.
* Support request bodies with strict size limits.
* Add chunked decoding when its prerequisites are understood.

#### Test
* Fixed-length bodies.
* Empty bodies.
* Bodies split across multiple reads.
* Chunked bodies.
* Malformed chunks.
* Conflicting framing headers.
* Missing or invalid framing.
* Bodies larger than configured limits.
* Extra bytes following a request.

#### Completion Criteria
The server has a documented and tested message-framing implementation that does not silently interpret ambiguous input.

---

### Phase 6 — Connection Management and Keep-Alive
#### Learn
* Persistent HTTP connections.
* Multiple requests on one TCP connection.
* Connection closure.
* Read and write timeouts.
* Client disconnects.
* Partial writes.
* Backpressure.

#### Implement
Introduce a connection state machine. Handle the complete lifecycle of each connection, including reading, parsing, responding, and closing.

Add persistent connections only after the parser can identify message boundaries correctly. Use explicit connection limits and timeouts.

#### Test
* Multiple requests on one connection.
* `Connection: close`.
* Clients that stop sending data.
* Clients that disconnect during a response.
* Partial writes.
* Idle connections.
* Slow clients.
* Connection-limit exhaustion.

#### Completion Criteria
Connections terminate predictably, resources are bounded, and persistent requests cannot corrupt one another's parsing state.

---

### Phase 7 — Static Files and Practical Server Features
#### Learn
* File I/O.
* MIME types.
* URL path normalization.
* Directory traversal.
* File size limits.
* Streaming responses.

#### Implement
Add optional static-file serving. Keep file serving separate from REST routing so the feature can be omitted or disabled by applications using the library.

#### Test
* Existing files vs. missing files.
* Empty files vs. large files.
* URL-encoded paths.
* Path traversal attempts.
* Permission failures.
* Incorrect or missing MIME types.

#### Completion Criteria
Static-file serving is optional, bounded, and cannot access files outside its configured root.

---

### Phase 8 — Testing and Security Hardening
#### Learn
* Unit and integration testing.
* Fuzz testing.
* Sanitizers.
* Threat modeling.
* Denial-of-service risks.
* Resource exhaustion.
* Secure defaults.

#### Implement
Create a regression test suite for the parser, serializer, connection lifecycle, and routing.

Add:
* AddressSanitizer and UndefinedBehaviorSanitizer builds.
* Fuzz testing for parser inputs.
* Limits for headers, bodies, connections, and time spent reading requests.
* Safe error handling.
* Structured logging that avoids leaking sensitive request data.

Evaluate ThreadSanitizer when multithreading is introduced.

#### Test
Include malformed inputs, boundary values, disconnects, resource exhaustion, and repeated requests. Every discovered bug should receive a regression test.

#### Completion Criteria
Known bugs have tests, resource limits are enforced, and malformed input does not cause uncontrolled allocation or crashes.

---

### Phase 9 — Reusable C++ Library API
#### Learn
* Public versus private interfaces.
* API stability.
* Object ownership.
* Pimpl as an optional technique.
* Error-reporting design.
* Library installation and consumption.
* Semantic versioning.

#### Implement
Separate the library from the example executable. Design an initial public interface that allows an application to:
* Construct a server.
* Configure the listening address and port.
* Register handlers.
* Configure relevant resource limits.
* Start the server.
* Request a controlled shutdown.
* Inspect errors.

Provide a small example application that uses the library without depending on internal implementation headers.

#### Test
* Build the library independently.
* Build an example consumer.
* Verify public headers do not require internal source files.
* Test installation and consumption from another CMake project.

#### Completion Criteria
A separate C++ program can use the library through a documented public API.

---

### Phase 10 — Concurrency and Performance
#### Learn
* Threads and synchronization.
* Blocking versus non-blocking I/O.
* Event loops.
* `poll()` and Linux `epoll`.
* Thread pools.
* Work queues.
* Backpressure.
* Connection scheduling.
* Memory allocation costs.
* Benchmark methodology.

#### Implement Progressively
First, establish a correct synchronous baseline. Then compare possible concurrency models:
1. One connection at a time.
2. A thread per connection, with explicit resource limits.
3. A bounded worker pool.
4. A non-blocking event loop using Linux APIs.

Choose the eventual model based on measurements and maintainability. Keep the transport abstraction small enough that different implementations can be compared without rewriting the HTTP parser.

#### Benchmark
Measure the following:

| Metric | Purpose |
| :--- | :--- |
| **Requests per second** | Throughput |
| **Median latency** | Typical response time |
| **p95 and p99 latency** | Tail latency |
| **CPU utilization** | Processor cost |
| **Memory usage** | Resource consumption |
| **Concurrent connections** | Concurrency capacity |
| **Error rates under load** | Reliability under stress |

Use repeatable workloads and compare against a known baseline.

#### Completion Criteria
Performance claims are backed by reproducible measurements, and concurrency does not introduce data races or resource leaks.

---

### Phase 11 — Production-Oriented Readiness
#### Learn
* TLS and certificate management.
* Deployment and configuration.
* Graceful shutdown.
* Operational logging and metrics.
* Dependency management.
* Compatibility and release processes.
* Security review.

#### Implement
Evaluate production requirements independently:
* HTTPS through a carefully designed TLS integration.
* Graceful shutdown and request draining.
* Configuration validation.
* Structured logs and metrics.
* Operational resource limits.
* Release and installation documentation.
* Security and protocol compliance testing.

Do not implement cryptographic primitives from scratch. Learning TLS internals is a separate educational exercise; real deployment should use a reviewed TLS implementation or a properly configured TLS-terminating proxy.

#### Completion Criteria
The project has documented operational limitations, a repeatable release process, and evidence for the claims it makes about security and protocol behavior.

---

## 6. Testing Strategy
Testing begins in Phase 1 and grows with the implementation.

### 6.1. Unit Tests
Test components independently:
* Request-line parser.
* Header parser.
* HTTP message-framing logic.
* Response serializer.
* Route matching.
* Configuration validation.

### 6.2. Integration Tests
Run a real server and send requests through a TCP client. Verify:
* Response status.
* Headers.
* Body.
* Connection behavior.
* Error handling.

### 6.3. Protocol Tests
Exercise valid and malformed HTTP messages, boundary conditions, fragmented input, persistent connections, and framing ambiguities.

### 6.4. Fuzz Tests
Generate arbitrary parser inputs and verify that parsing does not:
* Crash.
* Hang indefinitely.
* Allocate unbounded memory.

### 6.5. Stress Tests
Test:
* Concurrent clients.
* Slow clients.
* Abrupt disconnects.
* Connection limits.
* Large request volumes.

### 6.6. Regression Tests
Every fixed defect should be preserved as a test.

---

## 7. Security Requirements
Security is part of correctness, not a final optional feature. The implementation must progressively address:
* Bounded request sizes.
* Bounded header sizes and counts.
* Bounded body sizes.
* Connection limits.
* Read and write timeouts.
* Malformed request rejection.
* Ambiguous message framing.
* Slow-client attacks.
* Resource exhaustion.
* Path traversal if static files are supported.
* Safe error handling.
* Thread safety.
* Avoiding secrets in logs.
* Safe TLS integration when HTTPS is introduced.

Bind development servers to loopback by default. Do not expose incomplete protocol implementations to public networks. A successful basic demonstration does not establish production readiness.

---

## 8. CMake and Repository Organization
The existing repository structure:

```text
HTTP-Server-From-Scratch/
├── Include/
├── Src/
├── Build/
├── .github/
├── Makefile
└── README.md
```

Preserve that setup while its purpose remains clear. Do not reorganize the entire repository before the implementation needs it.

As the project grows, consider evolving toward:

```text
HTTP-Server-From-Scratch/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── LICENSE
├── Include/
├── Src/
├── tests/
├── examples/
├── benchmarks/
├── docs/
│   ├── ROADMAP.md
│   ├── ARCHITECTURE.md
│   ├── HTTP-SCOPE.md
│   ├── SECURITY.md
│   └── TESTING.md
└── .github/
    └── workflows/
```

### Repository Guidelines
* Use an out-of-source build directory.
* Ensure build artifacts are ignored by Git.
* Keep public headers in `Include/`.
* Keep implementation details in `Src/`.
* Refine naming and directory conventions when there is enough code to justify them.
* Preserve existing project conventions unless there is a clear reason to change them.

---

## 9. Documentation Requirements
Keep these documents synchronized with the implementation:

| Document | Responsibility |
| :--- | :--- |
| `README.md` | Overview, build instructions, current capabilities, and a quick example. |
| `docs/ROADMAP.md` | Progressive learning plan and milestones. |
| `docs/ARCHITECTURE.md` | Current architecture and important design decisions. |
| `docs/HTTP-SCOPE.md` | Implemented HTTP behavior and known limitations. |
| `docs/SECURITY.md` | Threat model, limits, and known security gaps. |
| `docs/TESTING.md` | Test strategy and commands. |
| `CHANGELOG.md` | Meaningful changes between versions, when release management begins. |

Document what exists, what is planned, and what is intentionally unsupported. Do not describe planned features as implemented.

---

## 10. Definition of Done for Every Milestone
A milestone is complete only when:
* The implementation builds with the configured toolchain.
* The intended behavior is demonstrated.
* Relevant tests pass.
* Error cases have been considered.
* Resource ownership is clear.
* Important limitations are documented.
* The next milestone's prerequisites are understood.

*Passing one successful browser request is not sufficient evidence of correctness.*

---

## 11. Deferred Features
Do not prioritize these until the relevant earlier phases are complete:
* HTTP/2.
* HTTP/3 and QUIC.
* WebSockets.
* Advanced routing DSLs.
* Plugin systems.
* Lock-free data structures.
* Custom memory allocators.
* Kernel-bypass networking.
* Distributed load balancing.
* Advanced caching.
* Production TLS internals.

These are potential future directions, not commitments.

---

## 12. Immediate Next Steps
The next milestone is **Phase 0**, followed by **Phase 1**.

### Step 1 — Inspect the Existing Repository
* Inspect the existing CMake configuration.
* Inspect source files and headers.
* Inspect the Makefile and README.
* Check the Git working tree.
* Identify existing build conventions.

### Step 2 — Verify the Development Environment
* Confirm the Clang++ version.
* Confirm the CMake version.
* Verify the current build works.
* Test a clean, out-of-source build.
* Confirm compiler warnings are visible.

### Step 3 — Plan the First TCP Server
* Identify the smallest implementation that fits the current codebase.
* Study the socket lifecycle.
* Understand each system call before implementing it.
* Bind to loopback by default.
* Keep the server synchronous and single-client initially.

### Step 4 — Implement Incrementally
* Create the socket.
* Bind the socket.
* Listen for connections.
* Accept one client.
* Receive bytes.
* Send a simple response.
* Close resources correctly.
* Handle system-call failures.

*Compile and test after every meaningful change.*

### Step 5 — Validate and Document
* Test a normal client connection.
* Test client disconnects.
* Test port conflicts.
* Check error handling.
* Record the results.
* Document the implementation and its limitations.

Do not start by implementing a complete HTTP parser, router, asynchronous runtime, or framework API. First understand the path from a TCP socket to a working connection. Then build HTTP on top of that foundation.

---

## 13. Final Principle
The objective is not to finish the largest number of features as quickly as possible.

The objective is to understand each layer well enough to implement it correctly, explain its design, test its failure modes, and eventually turn the accumulated work into a useful C++ HTTP server library.

**Build incrementally. Test deliberately. Measure objectively. Document honestly.**