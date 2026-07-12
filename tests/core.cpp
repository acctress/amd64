#include <catch2/catch_test_macros.hpp>
#include <amd64/assembler//registers.hpp>
#include <amd64/core/encode.hpp>

using namespace amd64::core;
using namespace amd64::core::encode;

TEST_CASE( "enc() masks to 3 bits", "[register]" )
{
    REQUIRE( enc( Register::rax ) == 0b000 );
    REQUIRE( enc( Register::rdi ) == 0b111 );
    REQUIRE( enc( Register::r8  ) == 0b000 );
    REQUIRE( enc( Register::r15 ) == 0b111 );
}

TEST_CASE( "ext() detects extended registers", "[register]" )
{
    REQUIRE_FALSE( ext( Register::rax ) );
    REQUIRE_FALSE( ext( Register::rdi ) );
    REQUIRE( ext( Register::r8  ) );
    REQUIRE( ext( Register::r15 ) );
}

TEST_CASE( "rex() sets W/R/B bits correctly", "[encode]" )
{
    REQUIRE( rex( true, Register::rax, Register::rax ) == 0x48 );
    REQUIRE( rex( true, Register::r8, Register::rax )  == ( 0x40 | ( 1 << 3 ) | ( 1 << 2 ) ) );
    REQUIRE( rex( true, Register::rax, Register::r8 )  == ( 0x40 | ( 1 << 3 ) | ( 1 << 0 ) ) );
}

TEST_CASE( "mod_rm() encodes register-direct addressing", "[encode]" )
{
    REQUIRE( mod_rm( Register::rax, Register::rax ) == 0xC0 );
    REQUIRE( mod_rm( Register::rdi, Register::rsi ) == 0xFE );
}

TEST_CASE( "XmmRegister enc() masks to 3 bits", "[register]" )
{
    REQUIRE( enc( XmmRegister::xmm0 )  == 0b000 );
    REQUIRE( enc( XmmRegister::xmm7 )  == 0b111 );
    REQUIRE( enc( XmmRegister::xmm8 )  == 0b000 );
    REQUIRE( enc( XmmRegister::xmm15 ) == 0b111 );
}

TEST_CASE( "XmmRegister ext() detects extended registers", "[register]" )
{
    REQUIRE_FALSE( ext( XmmRegister::xmm0 ) );
    REQUIRE_FALSE( ext( XmmRegister::xmm7 ) );
    REQUIRE( ext( XmmRegister::xmm8 ) );
    REQUIRE( ext( XmmRegister::xmm15 ) );
}

TEST_CASE( "enc() and ext() agree across every GP register", "[register]" )
{
    for ( std::uint8_t v = 0; v <= 15; ++v )
    {
        const auto reg = static_cast<Register>( v );
        INFO( "register value = " << static_cast<int>( v ) );

        if ( v < 8 )
        {
            REQUIRE_FALSE( ext( reg ) );
            REQUIRE( enc( reg ) == v );
        }
        else
        {
            REQUIRE( ext( reg ) );
            REQUIRE( enc( reg ) == v - 8 );
        }
    }
}

TEST_CASE( "REX_NO_OPS matches a manually-built REX.W byte", "[encode]" )
{
    REQUIRE( REX_NO_OPS == rex( true, Register::rax, Register::rax ) );
}

TEST_CASE( "rex() sets both R and B when both operands are extended", "[encode]" )
{
    const auto byte = rex( true, Register::r15, Register::r15 );
    REQUIRE( byte == ( 0x40 | ( 1 << 3 ) | ( 1 << 2 ) | ( 1 << 0 ) ) );
}

TEST_CASE( "rex() without W clears bit 3", "[encode]" )
{
    const auto byte = rex( false, Register::rax, Register::rax );
    REQUIRE( byte == 0x40 );
}

TEST_CASE( "mod_rm() is symmetric under reg/rm swap only in encoding, not value", "[encode]" )
{
    REQUIRE( mod_rm( Register::rax, Register::rcx ) != mod_rm( Register::rcx, Register::rax ) );
    REQUIRE( mod_rm( Register::rax, Register::rax ) == mod_rm( Register::rax, Register::rax ) );
}

TEST_CASE( "mod_rm() reg field occupies bits 3-5, rm field occupies bits 0-2", "[encode]" )
{
    REQUIRE( ( mod_rm( Register::rdi, Register::rax ) & 0b00111000 ) == ( 7 << 3 ) );
    REQUIRE( ( mod_rm( Register::rax, Register::rdi ) & 0b00000111 ) == 7 );
}