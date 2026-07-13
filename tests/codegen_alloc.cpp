#include <algorithm>
#include <amd64/codegen/codegen.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace amd64::ir;
using namespace amd64::codegen;

TEST_CASE( "liveness: sequantial independant consts", "[codegen][liveness]" )
{
    Assembler  azm { 1024 };
    code_gen_t cg( azm );

    function_t fn;
    fn.create_block( { } );

    auto v0 = fn.iconst( 10 );
    auto v1 = fn.iconst( 20 );

    auto ranges = cg.compute_live_ranges( fn );

    auto r0 = *std::ranges::find_if( ranges,
                                     [ & ]( auto &r )
                                     {
                                         return r.value == v0;
                                     } );
    auto r1 = *std::ranges::find_if( ranges,
                                     [ & ]( auto &r )
                                     {
                                         return r.value == v1;
                                     } );

    REQUIRE( r0.start == 0 );
    REQUIRE( r1.start == 1 );

    /* since they are never used their end value stays at 0 */
    REQUIRE( cg.compute_max_live( ranges ) == 1 );
}

TEST_CASE( "liveness: overlapping intervals via binary ops", "[codegen][liveness]" )
{
    Assembler  azm { 1024 };
    code_gen_t cg( azm );

    function_t fn;
    fn.create_block( { } );

    auto a   = fn.iconst( 5 );  /* inst_idx: 0 */
    auto b   = fn.iconst( 10 ); /* inst_idx: 1 */
    auto sum = fn.iadd( a, b ); /* inst_idx: 2 -> reads a and b */

    auto ranges = cg.compute_live_ranges( fn );

    auto ra = *std::ranges::find_if( ranges,
                                     [ & ]( auto &r )
                                     {
                                         return r.value == a;
                                     } );
    auto rb = *std::ranges::find_if( ranges,
                                     [ & ]( auto &r )
                                     {
                                         return r.value == b;
                                     } );

    REQUIRE( ra.start == 0 );
    REQUIRE( ra.end == 2 );

    REQUIRE( rb.start == 1 );
    REQUIRE( rb.end == 2 );

    REQUIRE( cg.compute_max_live( ranges ) == 2 );
}

TEST_CASE( "liveness: function params and args extension", "[codegen][liveness]" )
{
    Assembler  azm { 1024 };
    code_gen_t cg( azm );

    module_t mod;

    auto &fn = mod.create_function( "test_func", { type_t::i64 }, type_t::i64 );

    /* entry block is created automatically with the args mapped */
    const auto v_const = fn.iconst( 100 );
    fn.ret( v_const );

    const auto ranges = cg.compute_live_ranges( fn );

    /* verify */
    REQUIRE_FALSE( ranges.empty( ) );
    REQUIRE( cg.compute_max_live( ranges ) >= 1 );
}

TEST_CASE( "liveness: control flow branchig (brif & jmp)", "[codegen][liveness]" )
{
    Assembler  azm { 1024 };
    code_gen_t cg( azm );

    function_t fn;
    fn.create_block( { } );
    auto c    = fn.iconst( 1 );
    auto cond = fn.icmp( set_cc_kind::eq, c, c );

    fn.brif( cond, 1, 2, { }, { } );

    auto ranges = cg.compute_live_ranges( fn );
    const auto r_cond = *std::ranges::find_if( ranges,
                                         [ & ]( auto &r )
                                         {
                                             return r.value == cond;
                                         } );

    REQUIRE( r_cond.end > r_cond.start );
}