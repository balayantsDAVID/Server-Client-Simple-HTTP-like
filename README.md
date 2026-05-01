# c-http-server

A multithreaded TCP server and interactive client written in C.
Uses a custom text protocol called CHLP/1.0

## Protocol: CHLP/1.0

CHLP/1.0 is not real HTTP. It is a simplified custom protocol with
similar structure: method, path, version, headers, body.

Supported methods:
  GET /path       - serve a file from ./www/
  POST /path      - save request body to a file in ./www/ (overwrites)
  ECHO /          - echo the request body back to the client

Request format:
  GET /hello.txt CHLP/1.0
  Body-Size: 0

  (blank line, then optional body)

## Features

  - Multithreaded: one thread per client connection (pthread)
  - Persistent connections: client can send multiple requests per session
  - Path traversal protection: rejects paths containing ..
  - Interactive client CLI with case-insensitive method input

## Files

  http.h       - structs, enums, function declarations
  http.c       - core logic: parsing, request handling, socket helpers
  server.c     - server entry point, accepts connections, spawns threads
  client.c     - interactive client CLI
  www/         - directory served by GET / written by POST

## Build

  # Server
  gcc server.c http.c -o server -lpthread

  # Client
  gcc client.c http.c -o client

## Run

  # Terminal 1
  mkdir -p www
  echo 'Hello!' > www/hello.txt
  ./server

  # Terminal 2
  ./client

## Quick test in client

  Enter method: GET
  Enter path: /hello.txt
  -> Response: Hello!

  Enter method: POST
  Enter path: /hello.txt
  Enter body: Barev!
  -> 200 OK

  Enter method: GET
  Enter path: /hello.txt
  -> Response: Barev!

  Enter method: ECHO
  Enter body: test message
  -> Response: test message
