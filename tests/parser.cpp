#include <amd64/ir/fmt.hpp>
#include <amd64/ir/ir.hpp>
#include <amd64/ir/parser.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace amd64::ir;
using namespace amd64::ir::parser;

static module_t parse( const std::string& text )
{
    parser_t p( text );
    return p.parse_module( );
}

TEST_CASE( "round trip, single constant return", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "ret42", {}, type_t::i64 );
    fn.ret( fn.iconst( 42 ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, negative constant", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "retneg", {}, type_t::i64 );
    fn.ret( fn.iconst( -17 ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, iadd with two args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "add2", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.iadd( fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, isub with two args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "sub2", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.isub( fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, imul with two args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "mul2", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.imul( fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, idiv with two args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "div2", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.idiv( fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, icmp eq", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "eqtest", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, icmp ne", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "netest", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::ne, fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, icmp lt", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "lttest", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::lt, fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, icmp le", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "letest", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::le, fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, icmp gt", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "gttest", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::gt, fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, icmp ge", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "getest", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::ge, fn.args[0], fn.args[1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, call with native target", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "callnative", { type_t::i64 }, type_t::i64 );
    call_target_t target{ .target = reinterpret_cast<const void*>( 0x1234 ) };
    fn.ret( fn.call( { fn.args[0] }, target ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, call with jit target", "[parser]" )
{
    module_t mod;
    auto& helper = mod.create_function( "helper", { type_t::i64 }, type_t::i64 );
    helper.ret( helper.imul( helper.args[0], helper.iconst( 3 ) ) );
    auto& caller = mod.create_function( "caller", { type_t::i64 }, type_t::i64 );
    call_target_t target{ .target = std::string( "helper" ) };
    caller.ret( caller.call( { caller.args[0] }, target ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, call with zero args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "zeroargs", {}, type_t::i64 );
    call_target_t target{ .target = reinterpret_cast<const void*>( 0x9999 ) };
    fn.ret( fn.call( {}, target ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, call with multiple args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "multiargs", { type_t::i64, type_t::i64, type_t::i64 }, type_t::i64 );
    call_target_t target{ .target = reinterpret_cast<const void*>( 0x5555 ) };
    fn.ret( fn.call( { fn.args[0], fn.args[1], fn.args[2] }, target ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, jmp with args to second block", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "jmptest", { type_t::i64 }, type_t::i64 );
    auto doubled = fn.iadd( fn.args[0], fn.args[0] );
    fn.jmp( 1, { doubled } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, jmp with zero args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "jmpzero", {}, type_t::i64 );
    fn.jmp( 1, {} );
    fn.create_block( {} );
    fn.ret( fn.iconst( 7 ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, jmp with multiple args", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "jmpmulti", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.jmp( 1, { fn.args[0], fn.args[1] } );
    fn.create_block( { type_t::i64, type_t::i64 } );
    fn.ret( fn.iadd( fn.param_indices[1][0], fn.param_indices[1][1] ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, brif with empty arg lists", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "branchempty", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto cond = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( cond, 1, 2, {}, {} );
    fn.create_block( {} );
    fn.ret( fn.iconst( 1 ) );
    fn.create_block( {} );
    fn.ret( fn.iconst( 0 ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, brif with populated arg lists", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "branchargs", { type_t::i64 }, type_t::i64 );
    const auto zero = fn.iconst( 0 );
    auto cond = fn.icmp( set_cc_kind::ne, fn.args[0], zero );
    auto ten = fn.iconst( 10 );
    auto twenty = fn.iconst( 20 );
    fn.brif( cond, 1, 2, { ten }, { twenty } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[2][0] );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, chained arithmetic", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "chain", { type_t::i64, type_t::i64, type_t::i64, type_t::i64 }, type_t::i64 );
    const auto sum = fn.iadd( fn.args[0], fn.args[1] );
    const auto diff = fn.isub( fn.args[2], fn.args[3] );
    fn.ret( fn.imul( sum, diff ) );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, multiple functions in one module", "[parser]" )
{
    module_t mod;
    auto& f1 = mod.create_function( "f1", { type_t::i64 }, type_t::i64 );
    f1.ret( f1.iadd( f1.args[0], f1.iconst( 1 ) ) );
    auto& f2 = mod.create_function( "f2", { type_t::i64 }, type_t::i64 );
    f2.ret( f2.imul( f2.args[0], f2.iconst( 2 ) ) );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, function with no args and no params", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "noargs", {}, type_t::boolean );
    fn.ret( fn.iconst( 1 ) );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, i32 typed function", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "i32fn", { type_t::i32 }, type_t::i32 );
    fn.ret( fn.args[0] );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, ptr typed function", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "ptrfn", { type_t::pointer }, type_t::pointer );
    fn.ret( fn.args[0] );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, three block chain via jmp", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "threeblocks", { type_t::i64 }, type_t::i64 );
    fn.jmp( 1, { fn.args[0] } );
    fn.create_block( { type_t::i64 } );
    fn.jmp( 2, { fn.param_indices[1][0] } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[2][0] );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, many consts chained additively", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "manyconsts", {}, type_t::i64 );
    Value acc = fn.iconst( 1 );
    for ( std::int64_t i = 2; i <= 10; ++i )
        acc = fn.iadd( acc, fn.iconst( i ) );
    fn.ret( acc );
    auto text = to_string( mod );
    const auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "parsed function preserves name", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "specific_name_123", {}, type_t::i64 );
    fn.ret( fn.iconst( 0 ) );
    auto parsed = parse( to_string( mod ) );
    REQUIRE( parsed.get_function( 0 ).name == "specific_name_123" );
}

TEST_CASE( "parsed function preserves return type", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "rettest", {}, type_t::boolean );
    fn.ret( fn.iconst( 1 ) );
    auto parsed = parse( to_string( mod ) );
    REQUIRE( parsed.get_function( 0 ).return_type == type_t::boolean );
}

TEST_CASE( "parsed module preserves function count", "[parser]" )
{
    module_t mod;
    auto& a = mod.create_function( "a", {}, type_t::i64 ); a.ret( a.iconst( 0 ) );
    auto& b = mod.create_function( "b", {}, type_t::i64 ); b.ret( b.iconst( 0 ) );
    auto& c = mod.create_function( "c", {}, type_t::i64 ); c.ret( c.iconst( 0 ) );
    auto [functions ] = parse( to_string( mod ) );
    REQUIRE( functions.size( ) == 3 );
}

TEST_CASE( "parsed function preserves block count", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "blocktest", {}, type_t::i64 );
    fn.jmp( 1, {} );
    fn.create_block( {} );
    fn.ret( fn.iconst( 0 ) );
    auto parsed = parse( to_string( mod ) );
    REQUIRE( parsed.get_function( 0 ).blocks.size( ) == 2 );
}


TEST_CASE( "empty source parses to an empty module", "[parser]" )
{
    auto [functions ] = parse( "" );
    REQUIRE( functions.empty( ) );
}

TEST_CASE( "parse error: missing fn keyword throws", "[parser]" )
{
    REQUIRE_THROWS_AS( parse( "@foo() -> i64 {\n}\n" ), parse_error );
}

TEST_CASE( "parse error: missing arrow throws", "[parser]" )
{
    REQUIRE_THROWS_AS( parse( "fn @foo() i64 {\n}\n" ), parse_error );
}

TEST_CASE( "parse error: unknown comparison kind throws", "[parser]" )
{
    std::string text =
        "fn @foo(i64 %0, i64 %1) -> i64 {\n"
        "bb0():\n"
        "  %2 = icmp.egg %0, %1\n"
        "  ret %2\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: reference to undefined value throws", "[parser]" )
{
    std::string text =
        "fn @foo() -> i64 {\n"
        "bb0():\n"
        "  ret %99\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: mismatched result id throws", "[parser]" )
{
    std::string text =
        "fn @foo() -> i64 {\n"
        "bb0():\n"
        "  %5 = iconst 42\n"
        "  ret %5\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: wrong block index throws", "[parser]" )
{
    std::string text =
        "fn @foo() -> i64 {\n"
        "bb0():\n"
        "  jmp bb1()\n"
        "bb5():\n"
        "  ret %0\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: missing closing paren throws", "[parser]" )
{
    std::string text =
        "fn @foo( -> i64 {\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: missing closing brace throws", "[parser]" )
{
    std::string text =
        "fn @foo() -> i64 {\n"
        "bb0():\n"
        "  ret %0\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: bad type keyword throws", "[parser]" )
{
    std::string text = "fn @foo(weird %0) -> i64 {\n}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: malformed instruction throws", "[parser]" )
{
    std::string text =
        "fn @foo() -> i64 {\n"
        "bb0():\n"
        "  %0 = notarealinst 1, 2\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "parse error: call missing target throws", "[parser]" )
{
    std::string text =
        "fn @foo() -> i64 {\n"
        "bb0():\n"
        "  %0 = call ()\n"
        "  ret %0\n"
        "}\n";
    REQUIRE_THROWS_AS( parse( text ), parse_error );
}

TEST_CASE( "round trip, nested jit calls", "[parser]" )
{
    module_t mod;
    auto& inner = mod.create_function( "inner", { type_t::i64 }, type_t::i64 );
    inner.ret( inner.iadd( inner.args[0], inner.iconst( 1 ) ) );
    auto& middle = mod.create_function( "middle", { type_t::i64 }, type_t::i64 );
    call_target_t inner_target{ .target = std::string( "inner" ) };
    middle.ret( middle.call( { middle.args[0] }, inner_target ) );
    auto& outer = mod.create_function( "outer", { type_t::i64 }, type_t::i64 );
    call_target_t middle_target{ .target = std::string( "middle" ) };
    outer.ret( outer.call( { outer.args[0] }, middle_target ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, function returning a call result directly used in arithmetic", "[parser]" )
{
    module_t mod;
    auto& helper = mod.create_function( "get5", {}, type_t::i64 );
    helper.ret( helper.iconst( 5 ) );
    auto& caller = mod.create_function( "usefive", { type_t::i64 }, type_t::i64 );
    call_target_t target{ .target = std::string( "get5" ) };
    auto call_res = caller.call( {}, target );
    caller.ret( caller.imul( caller.args[0], call_res ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, block with two typed params", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "twoparams", {}, type_t::i64 );
    fn.jmp( 1, { fn.iconst( 1 ), fn.iconst( 2 ) } );
    fn.create_block( { type_t::i64, type_t::boolean } );
    fn.ret( fn.param_indices[1][0] );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, deeply chained divisions", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "divchain", {}, type_t::i64 );
    Value acc = fn.iconst( 1000 );
    acc = fn.idiv( acc, fn.iconst( 2 ) );
    acc = fn.idiv( acc, fn.iconst( 5 ) );
    fn.ret( acc );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, consecutive icmp results feeding brif", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "doublecmp", { type_t::i64, type_t::i64, type_t::i64 }, type_t::i64 );
    auto c1 = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    auto c2 = fn.icmp( set_cc_kind::lt, fn.args[1], fn.args[2] );
    auto both = fn.iadd( c1, c2 );
    fn.brif( both, 1, 2, {}, {} );
    fn.create_block( {} );
    fn.ret( fn.iconst( 1 ) );
    fn.create_block( {} );
    fn.ret( fn.iconst( 0 ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}

TEST_CASE( "round trip, idempotent double parse", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "idempotent", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.iadd( fn.args[0], fn.iconst( 100 ) ) );
    auto text1 = to_string( mod );
    auto parsed1 = parse( text1 );
    auto text2 = to_string( parsed1 );
    auto parsed2 = parse( text2 );
    auto text3 = to_string( parsed2 );
    REQUIRE( text1 == text2 );
    REQUIRE( text2 == text3 );
}

TEST_CASE( "round trip, large hex-like native address", "[parser]" )
{
    module_t mod;
    auto& fn = mod.create_function( "bigaddr", {}, type_t::i64 );
    call_target_t target{ .target = reinterpret_cast<const void*>( 0x7ffabc123456ULL ) };
    fn.ret( fn.call( {}, target ) );
    auto text = to_string( mod );
    auto parsed = parse( text );
    REQUIRE( to_string( parsed ) == text );
}