#include <catch2/catch_test_macros.hpp>
#include <amd64/assembler/assembler.hpp>

using namespace amd64::assembler;
using namespace amd64::assembler::registers;

TEST_CASE( "ret emits 0xC3", "[assembler]" )
{
    Assembler a{ 64 };
    a.ret();
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 1 );
    REQUIRE( bytes[0] == std::byte{ 0xC3 } );
}

TEST_CASE( "mov_reg_reg encodes rex+opcode+modrm", "[assembler]" )
{
    Assembler a{ 64 };
    a.mov_reg_reg( Register::rax, Register::rcx );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 3 );
    REQUIRE( bytes[0] == std::byte{ 0x48 } ); // REX.W, no ext regs
    REQUIRE( bytes[1] == std::byte{ 0x89 } );
    REQUIRE( bytes[2] == std::byte{ 0xC8 } ); // modrm: reg=rcx(1)<<3 | rm=rax(0) = 0xC8
}

TEST_CASE( "mov_reg_reg sets REX.R/B for extended registers", "[assembler]" )
{
    Assembler a{ 64 };
    a.mov_reg_reg( Register::r8, Register::r9 );
    auto bytes = a.bytes();
    // emit_rr(0x89, src=r9, dst=r8) -> rex(true, r9, r8): R from r9(ext), B from r8(ext)
    REQUIRE( bytes[0] == std::byte{ 0x48 | 0b0100 | 0b0001 } );
}

TEST_CASE( "cmp_reg_reg REX bits match modrm ordering for extended dst", "[assembler]" )
{
    Assembler a{ 64 };
    a.cmp_reg_reg( Register::r8, Register::rax );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 3 );
    REQUIRE( ( static_cast<std::uint8_t>( bytes[0] ) & 0b0100 ) != 0 ); // REX.R must be set
    REQUIRE( bytes[2] == std::byte{ 0xC0 } ); // modrm: reg=r8 enc(0)<<3 | rm=rax enc(0)
}

TEST_CASE( "add_reg_imm32 encodes digit 0 with imm32", "[assembler]" )
{
    Assembler a{ 64 };
    a.add_reg_imm32( Register::rax, 42 );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 7 ); // rex + 0x81 + modrm + 4-byte imm
    REQUIRE( bytes[1] == std::byte{ 0x81 } );
    REQUIRE( bytes[2] == std::byte{ 0xC0 } ); // digit 0, rm=rax
    std::int32_t imm{};
    std::memcpy( &imm, bytes.data() + 3, 4 );
    REQUIRE( imm == 42 );
}

TEST_CASE( "push/pop only emit REX for extended registers", "[assembler]" )
{
    Assembler a1{ 64 };
    a1.push( Register::rax );
    REQUIRE( a1.bytes().size() == 2 ); // no REX

    Assembler a2{ 64 };
    a2.push( Register::r8 );
    REQUIRE( a2.bytes().size() == 3 ); // REX + opcode + modrm
}

TEST_CASE( "jmp to unbound label reserves 4-byte placeholder and records fixup", "[assembler]" )
{
    Assembler a{ 64 };
    auto lbl = a.label();
    a.jmp( lbl );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 5 ); // 0xE9 + 4-byte placeholder
    REQUIRE( bytes[0] == std::byte{ 0xE9 } );
}

TEST_CASE( "jmp round-trip: forward jump patched correctly after bind", "[assembler]" )
{
    Assembler a{ 64 };
    auto lbl = a.label();
    a.jmp( lbl );           // jmp at pos 0, opcode(1) + placeholder(4) = 5 bytes
    a.nop( 3 );              // pad so target != fixup site
    a.bind( lbl );            // target should be pos 8

    auto bytes = a.bytes();
    std::int32_t patched{};
    std::memcpy( &patched, bytes.data() + 1, 4 ); // offset field starts right after 0xE9
    // offset = target - (fix_pos + 4) = 8 - (1 + 4) = 3
    REQUIRE( patched == 3 );
}

TEST_CASE( "mov_reg_mem and mov_mem_reg use indirect disp32 addressing", "[assembler]" )
{
    Assembler a{ 64 };
    a.mov_reg_mem( Register::rax, Register::rbx, 16 );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 7 ); // rex + opcode + modrm + 4-byte disp, no SIB (base != rsp)
    REQUIRE( bytes[1] == std::byte{ 0x8B } );
    REQUIRE( ( static_cast<std::uint8_t>( bytes[2] ) & 0xC0 ) == 0x80 ); // mod=10
}

TEST_CASE( "mov_reg_mem emits SIB byte when base is rsp", "[assembler]" )
{
    Assembler a{ 64 };
    a.mov_reg_mem( Register::rax, Register::rsp, 8 );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 8 ); // rex + opcode + modrm + SIB + 4-byte disp
    REQUIRE( bytes[3] == std::byte{ 0x24 } ); // SIB byte
}

TEST_CASE( "movsd_reg_reg omits REX for non-extended xmm regs", "[assembler]" )
{
    Assembler a{ 64 };
    a.movsd_reg_reg( XmmRegister::xmm0, XmmRegister::xmm1 );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 4 ); // 0xF2 + opcode(2) + modrm, no REX
    REQUIRE( bytes[0] == std::byte{ 0xF2 } );
}

TEST_CASE( "movsd_reg_reg includes REX when either operand is extended", "[assembler]" )
{
    Assembler a{ 64 };
    a.movsd_reg_reg( XmmRegister::xmm8, XmmRegister::xmm0 );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 5 ); // 0xF2 + REX + opcode(2) + modrm
}

TEST_CASE( "enter/leave round-trip via a callable function", "[assembler]" )
{
    Assembler a{ 128 };
    a.enter( 0 );
    a.mov_reg_imm64( Register::rax, 42 );
    a.leave();
    a.ret();
    auto fn = a.commit<std::int64_t(*)()>();
    REQUIRE( fn() == 42 );
}

TEST_CASE( "set_cc emits al-only form", "[assembler]" )
{
    Assembler a{ 64 };
    a.set_cc( set_cc_kind::eq, Register::rax );
    auto bytes = a.bytes();
    REQUIRE( bytes.size() == 3 );
    REQUIRE( bytes[0] == std::byte{ 0x0F } );
    REQUIRE( bytes[1] == std::byte{ 0x94 } );
}