#include <catch2/catch_test_macros.hpp>
#include <amd64/ir/parser.hpp>

using namespace amd64::ir::parser;

TEST_CASE( "empty source returns eof", "[lexer]" )
{
    lexer_t lex( "" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::eof );
}

TEST_CASE( "whitespace only returns eof", "[lexer]" )
{
    lexer_t lex( "   \t\n  " );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::eof );
}

TEST_CASE( "single char tokens", "[lexer]" )
{
    lexer_t lex( "@.,:(){}=" );
    REQUIRE( lex.next( ).type == token_type_t::at );
    REQUIRE( lex.next( ).type == token_type_t::dot );
    REQUIRE( lex.next( ).type == token_type_t::comma );
    REQUIRE( lex.next( ).type == token_type_t::colon );
    REQUIRE( lex.next( ).type == token_type_t::lparen );
    REQUIRE( lex.next( ).type == token_type_t::rparen );
    REQUIRE( lex.next( ).type == token_type_t::lbrace );
    REQUIRE( lex.next( ).type == token_type_t::rbrace );
    REQUIRE( lex.next( ).type == token_type_t::equals );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}

TEST_CASE( "arrow token", "[lexer]" )
{
    lexer_t lex( "->" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::arrow );
    REQUIRE( std::get<std::string>( t.value ) == "->" );
}

TEST_CASE( "percent with positive integer", "[lexer]" )
{
    lexer_t lex( "%42" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::percent );
    REQUIRE( std::get<std::int64_t>( t.value ) == 42 );
}

TEST_CASE( "percent with zero", "[lexer]" )
{
    lexer_t lex( "%0" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::percent );
    REQUIRE( std::get<std::int64_t>( t.value ) == 0 );
}

TEST_CASE( "plain positive integer", "[lexer]" )
{
    lexer_t lex( "12345" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::integer );
    REQUIRE( std::get<std::int64_t>( t.value ) == 12345 );
}

TEST_CASE( "negative integer", "[lexer]" )
{
    lexer_t lex( "-17" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::integer );
    REQUIRE( std::get<std::int64_t>( t.value ) == -17 );
}

TEST_CASE( "negative integer does not get confused with arrow", "[lexer]" )
{
    lexer_t lex( "-17 ->" );
    auto t1 = lex.next( );
    REQUIRE( t1.type == token_type_t::integer );
    REQUIRE( std::get<std::int64_t>( t1.value ) == -17 );
    auto t2 = lex.next( );
    REQUIRE( t2.type == token_type_t::arrow );
}

TEST_CASE( "all keywords lex correctly", "[lexer]" )
{
    lexer_t lex( "fn ret jmp brif call native iconst iadd isub imul idiv icmp i64 i32 bool ptr" );
    REQUIRE( lex.next( ).type == token_type_t::kw_fn );
    REQUIRE( lex.next( ).type == token_type_t::kw_ret );
    REQUIRE( lex.next( ).type == token_type_t::kw_jmp );
    REQUIRE( lex.next( ).type == token_type_t::kw_brif );
    REQUIRE( lex.next( ).type == token_type_t::kw_call );
    REQUIRE( lex.next( ).type == token_type_t::kw_native );
    REQUIRE( lex.next( ).type == token_type_t::kw_iconst );
    REQUIRE( lex.next( ).type == token_type_t::kw_iadd );
    REQUIRE( lex.next( ).type == token_type_t::kw_isub );
    REQUIRE( lex.next( ).type == token_type_t::kw_imul );
    REQUIRE( lex.next( ).type == token_type_t::kw_idiv );
    REQUIRE( lex.next( ).type == token_type_t::kw_icmp );
    REQUIRE( lex.next( ).type == token_type_t::kw_i64 );
    REQUIRE( lex.next( ).type == token_type_t::kw_i32 );
    REQUIRE( lex.next( ).type == token_type_t::kw_bool );
    REQUIRE( lex.next( ).type == token_type_t::kw_ptr );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}

TEST_CASE( "bb prefixed identifier becomes kw_bb with parsed number", "[lexer]" )
{
    lexer_t lex( "bb0 bb12 bb999" );
    auto t1 = lex.next( );
    REQUIRE( t1.type == token_type_t::kw_bb );
    REQUIRE( std::get<std::int64_t>( t1.value ) == 0 );
    auto t2 = lex.next( );
    REQUIRE( t2.type == token_type_t::kw_bb );
    REQUIRE( std::get<std::int64_t>( t2.value ) == 12 );
    auto t3 = lex.next( );
    REQUIRE( t3.type == token_type_t::kw_bb );
    REQUIRE( std::get<std::int64_t>( t3.value ) == 999 );
}

TEST_CASE( "bb with no trailing digits is a plain identifier", "[lexer]" )
{
    lexer_t lex( "bb" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::identifier );
    REQUIRE( std::get<std::string>( t.value ) == "bb" );
}

TEST_CASE( "bb followed by non digit is a plain identifier", "[lexer]" )
{
    lexer_t lex( "bblock" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::identifier );
    REQUIRE( std::get<std::string>( t.value ) == "bblock" );
}

TEST_CASE( "generic identifier", "[lexer]" )
{
    lexer_t lex( "get_multiplier" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::identifier );
    REQUIRE( std::get<std::string>( t.value ) == "get_multiplier" );
}

TEST_CASE( "identifier with leading underscore", "[lexer]" )
{
    lexer_t lex( "_private" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::identifier );
    REQUIRE( std::get<std::string>( t.value ) == "_private" );
}

TEST_CASE( "identifier with digits mixed in", "[lexer]" )
{
    lexer_t lex( "value2_final" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::identifier );
    REQUIRE( std::get<std::string>( t.value ) == "value2_final" );
}

TEST_CASE( "unexpected character throws", "[lexer]" )
{
    lexer_t lex( "$" );
    REQUIRE_THROWS_AS( lex.next( ), std::runtime_error );
}

TEST_CASE( "position is tracked correctly across tokens", "[lexer]" )
{
    lexer_t lex( "  %3" );
    auto t = lex.next( );
    REQUIRE( t.pos == 2 );
}

TEST_CASE( "full instruction line tokenizes correctly", "[lexer]" )
{
    lexer_t lex( "%3 = iadd %1, %2" );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::equals );
    REQUIRE( lex.next( ).type == token_type_t::kw_iadd );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::comma );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}

TEST_CASE( "full function header tokenizes correctly", "[lexer]" )
{
    lexer_t lex( "fn @add(i64 %0, i64 %1) -> i64 {" );
    REQUIRE( lex.next( ).type == token_type_t::kw_fn );
    REQUIRE( lex.next( ).type == token_type_t::at );
    REQUIRE( lex.next( ).type == token_type_t::identifier );
    REQUIRE( lex.next( ).type == token_type_t::lparen );
    REQUIRE( lex.next( ).type == token_type_t::kw_i64 );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::comma );
    REQUIRE( lex.next( ).type == token_type_t::kw_i64 );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::rparen );
    REQUIRE( lex.next( ).type == token_type_t::arrow );
    REQUIRE( lex.next( ).type == token_type_t::kw_i64 );
    REQUIRE( lex.next( ).type == token_type_t::lbrace );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}

TEST_CASE( "icmp with dot kind tokenizes correctly", "[lexer]" )
{
    lexer_t lex( "icmp.eq" );
    REQUIRE( lex.next( ).type == token_type_t::kw_icmp );
    REQUIRE( lex.next( ).type == token_type_t::dot );
    REQUIRE( lex.next( ).type == token_type_t::identifier );
}

TEST_CASE( "call with native target tokenizes correctly", "[lexer]" )
{
    lexer_t lex( "call native@0x1234(" );
    REQUIRE( lex.next( ).type == token_type_t::kw_call );
    REQUIRE( lex.next( ).type == token_type_t::kw_native );
    REQUIRE( lex.next( ).type == token_type_t::at );
    REQUIRE( lex.next( ).type == token_type_t::integer );
    REQUIRE( lex.next( ).type == token_type_t::lparen );
}

TEST_CASE( "brif full line tokenizes correctly", "[lexer]" )
{
    lexer_t lex( "brif %0, bb1(%2), bb2(%3)" );
    REQUIRE( lex.next( ).type == token_type_t::kw_brif );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::comma );
    REQUIRE( lex.next( ).type == token_type_t::kw_bb );
    REQUIRE( lex.next( ).type == token_type_t::lparen );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::rparen );
    REQUIRE( lex.next( ).type == token_type_t::comma );
    REQUIRE( lex.next( ).type == token_type_t::kw_bb );
    REQUIRE( lex.next( ).type == token_type_t::lparen );
    REQUIRE( lex.next( ).type == token_type_t::percent );
    REQUIRE( lex.next( ).type == token_type_t::rparen );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}

TEST_CASE( "repeated next calls past eof stay at eof", "[lexer]" )
{
    lexer_t lex( "%1" );
    lex.next( );
    REQUIRE( lex.next( ).type == token_type_t::eof );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}

TEST_CASE( "adjacent tokens with no whitespace still split correctly", "[lexer]" )
{
    lexer_t lex( "bb0():" );
    REQUIRE( lex.next( ).type == token_type_t::kw_bb );
    REQUIRE( lex.next( ).type == token_type_t::lparen );
    REQUIRE( lex.next( ).type == token_type_t::rparen );
    REQUIRE( lex.next( ).type == token_type_t::colon );
}

TEST_CASE( "hex integer in call native address does not lex as separate tokens", "[lexer]" )
{
    lexer_t lex( "0x1a2b3c" );
    auto t = lex.next( );
    REQUIRE( t.type == token_type_t::integer );
    REQUIRE( std::get<std::int64_t>( t.value ) == 0x1a2b3c );
    REQUIRE( lex.next( ).type == token_type_t::eof );
}