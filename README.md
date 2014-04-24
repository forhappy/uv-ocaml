# Uv-ocaml #

OCaml binding for [joyent/libuv](https://github.com/joyent/libuv).

> (WARNING: NOT USABLE RIGHT NOW, JUST WAIT FOR A WHILE.)

Uv-OCaml is a OCaml module which provides an interface to libuv. Libuv is a
high performance asynchronous networking library, used as the platform layer
for [NodeJS](http://nodejs.org). Libuv is written and maintained by Joyent
Inc. and contributors. It’s built on top of epoll/kequeue/event ports/etc on
Unix and IOCP on Windows systems providing a consistent API on top of them.

# Uv-OCaml's features #

 * Full-featured event loop backed by epoll, kqueue, IOCP, event ports.
 * Asynchronous TCP and UDP sockets
 * Asynchronous DNS resolution
 * Asynchronous file and file system operations
 * File system events
 * ANSI escape code controlled TTY
 * IPC with socket sharing, using Unix domain sockets or named pipes (Windows)
 * Child processes
 * Thread pool
 * Signal handling
 * High resolution clock
 * Threading and synchronization primitives

# Dependencies #

 * findlib
 * oUnit2 (optional for tests)

# Install #

Run,
  make
  make install

To compile Tests,

  make tests

# Contributing #

If you'd like to contribute, fork the project, make a patch and send a pull
request. Have a look at the surrounding code and please, make yours look
alike :-)
