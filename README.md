# amd64

A C++ JIT compiler, a port of [zjit](https://github.com/acctress/zjit) targeting x86_64.

## Overview

amd64 is a just-in-time compiler emitting native x86-64 machine code at runtime, which is a port of my Zig JIT compiler [zjit](https://github.com/acctress/zjit).

## Features

- [ ] Executable memory allocation
    - [x] Windows
    - [ ] Linux
- [ ] x86-64 instruction encoding
- [ ] SSA IR
- [ ] Linear scan register allocation

## Building

### Requirements

- C++23 compiler
- CMake 3.2x

### Build
```sh
$ git clone https://github.com/acctress/amd64
$ cd amd64
$ cmake -B build
$ cmake --build build
```

## License

MIT