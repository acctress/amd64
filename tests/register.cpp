#include <catch2/catch_test_macros.hpp>
#include <amd64/assembler/registers.hpp>

using namespace amd64::assembler::registers;

TEST_CASE( "enc rax", "[register]" ) { REQUIRE( enc( Register::rax ) == 0 ); }
TEST_CASE( "enc rcx", "[register]" ) { REQUIRE( enc( Register::rcx ) == 1 ); }
TEST_CASE( "enc rdx", "[register]" ) { REQUIRE( enc( Register::rdx ) == 2 ); }
TEST_CASE( "enc rbx", "[register]" ) { REQUIRE( enc( Register::rbx ) == 3 ); }
TEST_CASE( "enc rsp", "[register]" ) { REQUIRE( enc( Register::rsp ) == 4 ); }
TEST_CASE( "enc rbp", "[register]" ) { REQUIRE( enc( Register::rbp ) == 5 ); }
TEST_CASE( "enc rsi", "[register]" ) { REQUIRE( enc( Register::rsi ) == 6 ); }
TEST_CASE( "enc rdi", "[register]" ) { REQUIRE( enc( Register::rdi ) == 7 ); }
TEST_CASE( "enc r8",  "[register]" ) { REQUIRE( enc( Register::r8  ) == 0 ); }
TEST_CASE( "enc r9",  "[register]" ) { REQUIRE( enc( Register::r9  ) == 1 ); }
TEST_CASE( "enc r10", "[register]" ) { REQUIRE( enc( Register::r10 ) == 2 ); }
TEST_CASE( "enc r11", "[register]" ) { REQUIRE( enc( Register::r11 ) == 3 ); }
TEST_CASE( "enc r12", "[register]" ) { REQUIRE( enc( Register::r12 ) == 4 ); }
TEST_CASE( "enc r13", "[register]" ) { REQUIRE( enc( Register::r13 ) == 5 ); }
TEST_CASE( "enc r14", "[register]" ) { REQUIRE( enc( Register::r14 ) == 6 ); }
TEST_CASE( "enc r15", "[register]" ) { REQUIRE( enc( Register::r15 ) == 7 ); }

TEST_CASE( "ext rax", "[register]" ) { REQUIRE_FALSE( ext( Register::rax ) ); }
TEST_CASE( "ext rcx", "[register]" ) { REQUIRE_FALSE( ext( Register::rcx ) ); }
TEST_CASE( "ext rdx", "[register]" ) { REQUIRE_FALSE( ext( Register::rdx ) ); }
TEST_CASE( "ext rbx", "[register]" ) { REQUIRE_FALSE( ext( Register::rbx ) ); }
TEST_CASE( "ext rsp", "[register]" ) { REQUIRE_FALSE( ext( Register::rsp ) ); }
TEST_CASE( "ext rbp", "[register]" ) { REQUIRE_FALSE( ext( Register::rbp ) ); }
TEST_CASE( "ext rsi", "[register]" ) { REQUIRE_FALSE( ext( Register::rsi ) ); }
TEST_CASE( "ext rdi", "[register]" ) { REQUIRE_FALSE( ext( Register::rdi ) ); }
TEST_CASE( "ext r8",  "[register]" ) { REQUIRE( ext( Register::r8  ) ); }
TEST_CASE( "ext r9",  "[register]" ) { REQUIRE( ext( Register::r9  ) ); }
TEST_CASE( "ext r10", "[register]" ) { REQUIRE( ext( Register::r10 ) ); }
TEST_CASE( "ext r11", "[register]" ) { REQUIRE( ext( Register::r11 ) ); }
TEST_CASE( "ext r12", "[register]" ) { REQUIRE( ext( Register::r12 ) ); }
TEST_CASE( "ext r13", "[register]" ) { REQUIRE( ext( Register::r13 ) ); }
TEST_CASE( "ext r14", "[register]" ) { REQUIRE( ext( Register::r14 ) ); }
TEST_CASE( "ext r15", "[register]" ) { REQUIRE( ext( Register::r15 ) ); }

TEST_CASE( "enc xmm0",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm0  ) == 0 ); }
TEST_CASE( "enc xmm1",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm1  ) == 1 ); }
TEST_CASE( "enc xmm2",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm2  ) == 2 ); }
TEST_CASE( "enc xmm3",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm3  ) == 3 ); }
TEST_CASE( "enc xmm4",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm4  ) == 4 ); }
TEST_CASE( "enc xmm5",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm5  ) == 5 ); }
TEST_CASE( "enc xmm6",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm6  ) == 6 ); }
TEST_CASE( "enc xmm7",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm7  ) == 7 ); }
TEST_CASE( "enc xmm8",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm8  ) == 0 ); }
TEST_CASE( "enc xmm9",  "[register]" ) { REQUIRE( enc( XmmRegister::xmm9  ) == 1 ); }
TEST_CASE( "enc xmm10", "[register]" ) { REQUIRE( enc( XmmRegister::xmm10 ) == 2 ); }
TEST_CASE( "enc xmm11", "[register]" ) { REQUIRE( enc( XmmRegister::xmm11 ) == 3 ); }
TEST_CASE( "enc xmm12", "[register]" ) { REQUIRE( enc( XmmRegister::xmm12 ) == 4 ); }
TEST_CASE( "enc xmm13", "[register]" ) { REQUIRE( enc( XmmRegister::xmm13 ) == 5 ); }
TEST_CASE( "enc xmm14", "[register]" ) { REQUIRE( enc( XmmRegister::xmm14 ) == 6 ); }
TEST_CASE( "enc xmm15", "[register]" ) { REQUIRE( enc( XmmRegister::xmm15 ) == 7 ); }

TEST_CASE( "ext xmm0",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm0  ) ); }
TEST_CASE( "ext xmm1",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm1  ) ); }
TEST_CASE( "ext xmm2",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm2  ) ); }
TEST_CASE( "ext xmm3",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm3  ) ); }
TEST_CASE( "ext xmm4",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm4  ) ); }
TEST_CASE( "ext xmm5",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm5  ) ); }
TEST_CASE( "ext xmm6",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm6  ) ); }
TEST_CASE( "ext xmm7",  "[register]" ) { REQUIRE_FALSE( ext( XmmRegister::xmm7  ) ); }
TEST_CASE( "ext xmm8",  "[register]" ) { REQUIRE( ext( XmmRegister::xmm8  ) ); }
TEST_CASE( "ext xmm9",  "[register]" ) { REQUIRE( ext( XmmRegister::xmm9  ) ); }
TEST_CASE( "ext xmm10", "[register]" ) { REQUIRE( ext( XmmRegister::xmm10 ) ); }
TEST_CASE( "ext xmm11", "[register]" ) { REQUIRE( ext( XmmRegister::xmm11 ) ); }
TEST_CASE( "ext xmm12", "[register]" ) { REQUIRE( ext( XmmRegister::xmm12 ) ); }
TEST_CASE( "ext xmm13", "[register]" ) { REQUIRE( ext( XmmRegister::xmm13 ) ); }
TEST_CASE( "ext xmm14", "[register]" ) { REQUIRE( ext( XmmRegister::xmm14 ) ); }
TEST_CASE( "ext xmm15", "[register]" ) { REQUIRE( ext( XmmRegister::xmm15 ) ); }