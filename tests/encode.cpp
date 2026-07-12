#include <catch2/catch_test_macros.hpp>
#include <amd64/assembler/registers.hpp>
#include <amd64/core/encode.hpp>

using namespace amd64::assembler::registers;
using namespace amd64::core::encode;

TEST_CASE( "rex R bit rax",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rcx",  "[encode]" ) { REQUIRE( ( rex( true, Register::rcx,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rdx",  "[encode]" ) { REQUIRE( ( rex( true, Register::rdx,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rbx",  "[encode]" ) { REQUIRE( ( rex( true, Register::rbx,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rsp",  "[encode]" ) { REQUIRE( ( rex( true, Register::rsp,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rbp",  "[encode]" ) { REQUIRE( ( rex( true, Register::rbp,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rsi",  "[encode]" ) { REQUIRE( ( rex( true, Register::rsi,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit rdi",  "[encode]" ) { REQUIRE( ( rex( true, Register::rdi,  Register::rax ) & 0b0100 ) == 0 ); }
TEST_CASE( "rex R bit r8",   "[encode]" ) { REQUIRE( ( rex( true, Register::r8,   Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r9",   "[encode]" ) { REQUIRE( ( rex( true, Register::r9,   Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r10",  "[encode]" ) { REQUIRE( ( rex( true, Register::r10,  Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r11",  "[encode]" ) { REQUIRE( ( rex( true, Register::r11,  Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r12",  "[encode]" ) { REQUIRE( ( rex( true, Register::r12,  Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r13",  "[encode]" ) { REQUIRE( ( rex( true, Register::r13,  Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r14",  "[encode]" ) { REQUIRE( ( rex( true, Register::r14,  Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex R bit r15",  "[encode]" ) { REQUIRE( ( rex( true, Register::r15,  Register::rax ) & 0b0100 ) != 0 ); }
TEST_CASE( "rex B bit rax",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rax ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rcx",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rcx ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rdx",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rdx ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rbx",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rbx ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rsp",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rsp ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rbp",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rbp ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rsi",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rsi ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit rdi",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::rdi ) & 0b0001 ) == 0 ); }
TEST_CASE( "rex B bit r8",   "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r8  ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r9",   "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r9  ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r10",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r10 ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r11",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r11 ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r12",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r12 ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r13",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r13 ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r14",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r14 ) & 0b0001 ) != 0 ); }
TEST_CASE( "rex B bit r15",  "[encode]" ) { REQUIRE( ( rex( true, Register::rax, Register::r15 ) & 0b0001 ) != 0 ); }

TEST_CASE( "mod_rm reg field rax",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rax ) & 0b00111000 ) == ( 0 << 3 ) ); }
TEST_CASE( "mod_rm reg field rcx",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rcx, Register::rax ) & 0b00111000 ) == ( 1 << 3 ) ); }
TEST_CASE( "mod_rm reg field rdx",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rdx, Register::rax ) & 0b00111000 ) == ( 2 << 3 ) ); }
TEST_CASE( "mod_rm reg field rbx",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rbx, Register::rax ) & 0b00111000 ) == ( 3 << 3 ) ); }
TEST_CASE( "mod_rm reg field rsp",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rsp, Register::rax ) & 0b00111000 ) == ( 4 << 3 ) ); }
TEST_CASE( "mod_rm reg field rbp",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rbp, Register::rax ) & 0b00111000 ) == ( 5 << 3 ) ); }
TEST_CASE( "mod_rm reg field rsi",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rsi, Register::rax ) & 0b00111000 ) == ( 6 << 3 ) ); }
TEST_CASE( "mod_rm reg field rdi",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rdi, Register::rax ) & 0b00111000 ) == ( 7 << 3 ) ); }
TEST_CASE( "mod_rm reg field r8",   "[encode]" ) { REQUIRE( ( mod_rm( Register::r8,  Register::rax ) & 0b00111000 ) == ( 0 << 3 ) ); }
TEST_CASE( "mod_rm reg field r9",   "[encode]" ) { REQUIRE( ( mod_rm( Register::r9,  Register::rax ) & 0b00111000 ) == ( 1 << 3 ) ); }
TEST_CASE( "mod_rm reg field r10",  "[encode]" ) { REQUIRE( ( mod_rm( Register::r10, Register::rax ) & 0b00111000 ) == ( 2 << 3 ) ); }
TEST_CASE( "mod_rm reg field r11",  "[encode]" ) { REQUIRE( ( mod_rm( Register::r11, Register::rax ) & 0b00111000 ) == ( 3 << 3 ) ); }
TEST_CASE( "mod_rm reg field r12",  "[encode]" ) { REQUIRE( ( mod_rm( Register::r12, Register::rax ) & 0b00111000 ) == ( 4 << 3 ) ); }
TEST_CASE( "mod_rm reg field r13",  "[encode]" ) { REQUIRE( ( mod_rm( Register::r13, Register::rax ) & 0b00111000 ) == ( 5 << 3 ) ); }
TEST_CASE( "mod_rm reg field r14",  "[encode]" ) { REQUIRE( ( mod_rm( Register::r14, Register::rax ) & 0b00111000 ) == ( 6 << 3 ) ); }
TEST_CASE( "mod_rm reg field r15",  "[encode]" ) { REQUIRE( ( mod_rm( Register::r15, Register::rax ) & 0b00111000 ) == ( 7 << 3 ) ); }

TEST_CASE( "mod_rm rm field rax",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rax ) & 0b00000111 ) == 0 ); }
TEST_CASE( "mod_rm rm field rcx",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rcx ) & 0b00000111 ) == 1 ); }
TEST_CASE( "mod_rm rm field rdx",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rdx ) & 0b00000111 ) == 2 ); }
TEST_CASE( "mod_rm rm field rbx",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rbx ) & 0b00000111 ) == 3 ); }
TEST_CASE( "mod_rm rm field rsp",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rsp ) & 0b00000111 ) == 4 ); }
TEST_CASE( "mod_rm rm field rbp",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rbp ) & 0b00000111 ) == 5 ); }
TEST_CASE( "mod_rm rm field rsi",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rsi ) & 0b00000111 ) == 6 ); }
TEST_CASE( "mod_rm rm field rdi",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::rdi ) & 0b00000111 ) == 7 ); }
TEST_CASE( "mod_rm rm field r8",   "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r8  ) & 0b00000111 ) == 0 ); }
TEST_CASE( "mod_rm rm field r9",   "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r9  ) & 0b00000111 ) == 1 ); }
TEST_CASE( "mod_rm rm field r10",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r10 ) & 0b00000111 ) == 2 ); }
TEST_CASE( "mod_rm rm field r11",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r11 ) & 0b00000111 ) == 3 ); }
TEST_CASE( "mod_rm rm field r12",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r12 ) & 0b00000111 ) == 4 ); }
TEST_CASE( "mod_rm rm field r13",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r13 ) & 0b00000111 ) == 5 ); }
TEST_CASE( "mod_rm rm field r14",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r14 ) & 0b00000111 ) == 6 ); }
TEST_CASE( "mod_rm rm field r15",  "[encode]" ) { REQUIRE( ( mod_rm( Register::rax, Register::r15 ) & 0b00000111 ) == 7 ); }