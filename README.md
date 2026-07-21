# amd64

A C++23 JIT compiler, SSA IR, register allocator, targeting x86_64.
Built as the backend for [liftmips](https://github.com/acctress/liftmips), a MIPS R3000A lifter for PS1 emulation.

## Features

- [x] Assembler
- [x] SSA IR
- [x] Linear scan register allocator
- [x] Codegen
- [x] Verifier

## Roadmap
- [x] Windows x86_64
- [x] Linux (system v)
- [ ] Immediate form arith
- [ ] Unsigned division
- [ ] 32 bit truncation semantics

## Building

Requirements: Cmake 3.28+, C++23 compiler, Ninja

### Build
```sh
$ git clone https://github.com/acctress/amd64
$ cd amd64
$ cmake -B build -G Ninja
$ cmake --build build
```

If you want to test amd64:
```sh
$ cmake --build build --target amd64_tests
$ ./build/bin/exe/amd64_tests
```

## Quick Start

### Using the API
```cpp
#include <amd64/ir/ir.hpp>
#include <amd64/codegen/codegen.hpp>
#include <amd64/assembler/assembler.hpp>

using namespace amd64::ir;
using namespace amd64::codegen;
using namespace amd64::assembler;

module_t mod;

auto& fn = mod.create_function( "add", { type_t::i64, type_t::i64 }, type::i64 );
fn.ret( fn.iadd( fn.args[0], fn.args[1] ) );

Assembler   azm { 4096 };
code_gen_t  cg( azm );

auto gen = cg.compile_module( mod );
auto add = gen.get_function( "add" );
add( 3, 4 ); /* 7 */
```

### Using the text IR
```
fn @add(i64 %0, i64 %1) -> i64 {
bb0():
    %2 = iadd %0, %1
    ret %2
}
```

```cpp
#include <amd64/ir/parser.hpp>

amd64::ir::parser::parser_t p( source_text );
auto mod = p.parse_module( );
```

## IR Reference

### Types
* `i64`
* `i32`
* `bool`
* `ptr`

### Instructions
| Instruction | Syntax                                                 |
|-------------|--------------------------------------------------------|
| Constant    | `%r = iconst <imm>`                                    |
| Arithmetic  | `%r = iadd/isub/imul/idiv %a, %b`                      |
| Bitwise     | `%r = iand/ior/ixor/ishl/ishr %a, %b`                  |
| Unary       | `%r = inot/ineg %a`                                    |
| Compare     | `%r = icmp.{eq,ne,lt,le,gt,ge,ult,ule,ugt,uge} %a, %b` |
| Load        | `%r = load.{i8,i8s,i16,i16s,i32,i64} %base, <offset>`  |
| Store       | `store.{i8,i16,i32,i64} %val, %base, <offset>`         |
| Call        | `%r = call @name(%a, %b)` or `call native@<addr>(...)` |
| Branch      | `brif %cond, bb1(%a), bb2(%b)`                         |
| Jump        | `jmp bb1(%a)`                                          |
| Return      | `ret %val`                                             |

## License

MIT