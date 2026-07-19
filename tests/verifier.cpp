#include "amd64/ir/fmt.hpp"

#include <print>
#include <amd64/codegen/verify.hpp>
#include <amd64/ir/ir.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace amd64::ir;
using namespace amd64::codegen;

static void ok( const module_t& mod )    { REQUIRE_NOTHROW( verify_module( mod ) ); }
static void bad( const module_t& mod )   { REQUIRE_THROWS_AS( verify_module( mod ), verify_error ); }

TEST_CASE( "verify valid constant return",                "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( fn.iconst( 42 ) );
    ok( mod );
}

TEST_CASE( "verify valid iadd",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.iadd( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid isub",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.isub( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid imul",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.imul( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid idiv",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.idiv( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid iand",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.iand( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid ior",                            "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ior( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid ixor",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ixor( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid ishl",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishl( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid ishr",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishr( fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid inot",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.inot( fn.args[0] ) );
    ok( mod );
}

TEST_CASE( "verify valid ineg",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ineg( fn.args[0] ) );
    ok( mod );
}

TEST_CASE( "verify valid icmp eq",                        "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] ) );
    ok( mod );
}

TEST_CASE( "verify valid load",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.ret( fn.load( fn.args[0], 0, 8, false ) );
    ok( mod );
}

TEST_CASE( "verify valid store then ret",                 "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer, type_t::i64 }, type_t::i64 );
    fn.store( fn.args[1], fn.args[0], 0, 8 );
    fn.ret( fn.args[1] );
    ok( mod );
}

TEST_CASE( "verify valid jmp with args",                  "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    auto v = fn.iadd( fn.args[0], fn.iconst( 1 ) );
    fn.jmp( 1, { v } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    ok( mod );
}

TEST_CASE( "verify valid jmp with no args",               "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.jmp( 1, {} );
    fn.create_block( {} );
    fn.ret( fn.iconst( 0 ) );
    ok( mod );
}

TEST_CASE( "verify valid brif",                           "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto cond = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( cond, 1, 2, {}, {} );
    fn.create_block( {} ); fn.ret( fn.iconst( 1 ) );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    ok( mod );
}

TEST_CASE( "verify valid brif with args",                 "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "valid_brif_with_args", { type_t::i64 }, type_t::i64 );
    auto cond = fn.icmp( set_cc_kind::ne, fn.args[0], fn.iconst( 0 ) );
    auto ten = fn.iconst( 10 ); auto twenty = fn.iconst( 20 );
    fn.brif( cond, 1, 2, { ten }, { twenty } );
    fn.create_block( { type_t::i64 } ); fn.ret( fn.param_indices[1][0] );
    fn.create_block( { type_t::i64 } ); fn.ret( fn.param_indices[2][0] );

    std::println("{}", to_string( mod ));

    ok( mod );
}

TEST_CASE( "verify valid call native",                    "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    call_target_t target { .target = reinterpret_cast<const void*>( 0x1234 ) };
    fn.ret( fn.call( { fn.args[0] }, target ) );
    ok( mod );
}

TEST_CASE( "verify valid jit call",                       "[verify]" )
{
    module_t mod;
    auto& helper = mod.create_function( "helper", { type_t::i64 }, type_t::i64 );
    helper.ret( helper.args[0] );
    auto& caller = mod.create_function( "caller", { type_t::i64 }, type_t::i64 );
    call_target_t target { .target = std::string( "helper" ) };
    caller.ret( caller.call( { caller.args[0] }, target ) );
    ok( mod );
}

TEST_CASE( "verify valid empty module",                   "[verify]" )
{
    module_t mod;
    ok( mod );
}

TEST_CASE( "verify valid multiple functions",             "[verify]" )
{
    module_t mod;
    auto& f1 = mod.create_function( "f1", {}, type_t::i64 );
    f1.ret( f1.iconst( 1 ) );
    auto& f2 = mod.create_function( "f2", {}, type_t::i64 );
    f2.ret( f2.iconst( 2 ) );
    ok( mod );
}

TEST_CASE( "verify valid chain of arithmetic",            "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto a = fn.iadd( fn.args[0], fn.args[1] );
    auto b = fn.imul( a, fn.args[0] );
    auto c = fn.isub( b, fn.args[1] );
    fn.ret( c );
    ok( mod );
}

TEST_CASE( "verify valid three block chain",              "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.jmp( 1, { fn.args[0] } );
    fn.create_block( { type_t::i64 } );
    fn.jmp( 2, { fn.param_indices[1][0] } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[2][0] );
    ok( mod );
}

TEST_CASE( "verify value defined in block 0 used in block 1", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    auto v = fn.iconst( 99 );
    fn.jmp( 1, { v } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    ok( mod );
}

TEST_CASE( "verify valid load with nonzero offset",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.ret( fn.load( fn.args[0], 16, 8, false ) );
    ok( mod );
}

TEST_CASE( "verify valid store with negative offset",     "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer, type_t::i64 }, type_t::i64 );
    fn.store( fn.args[1], fn.args[0], -8, 8 );
    fn.ret( fn.args[1] );
    ok( mod );
}

TEST_CASE( "verify empty entry block throws",             "[verify]" )
{
    module_t mod;
    mod.create_function( "f", {}, type_t::i64 );
    bad( mod );
}

TEST_CASE( "verify block with no terminator throws",      "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.iconst( 1 );
    bad( mod );
}

TEST_CASE( "verify second block missing terminator",      "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.jmp( 1, {} );
    fn.create_block( {} );
    fn.iconst( 0 );
    bad( mod );
}

TEST_CASE( "verify ret mid-block throws",                 "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( fn.iconst( 1 ) );
    fn.iconst( 2 );
    bad( mod );
}

TEST_CASE( "verify jmp mid-block throws",                 "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.jmp( 1, {} );
    fn.iconst( 0 );
    fn.create_block( {} );
    fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify jmp to nonexistent block throws",      "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.jmp( 99, {} );
    bad( mod );
}

TEST_CASE( "verify brif true_blk out of range throws",    "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 99, 1, {}, {} );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify brif false_blk out of range throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 1, 99, {}, {} );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify jmp passes too many args throws",      "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.jmp( 1, { fn.args[0], fn.iconst( 1 ) } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    bad( mod );
}

TEST_CASE( "verify jmp passes too few args throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.jmp( 1, {} );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    bad( mod );
}

TEST_CASE( "verify brif true_args count mismatch throws", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 1, 2, { fn.args[0], fn.args[1] }, {} );
    fn.create_block( { type_t::i64 } ); fn.ret( fn.param_indices[1][0] );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify brif false_args count mismatch throws","[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 1, 2, {}, { fn.args[0] } );
    fn.create_block( {} ); fn.ret( fn.iconst( 1 ) );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify ret referencing undefined value throws","[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( 99 );
    bad( mod );
}

TEST_CASE( "verify iadd with undefined lhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.iadd( 99, fn.args[0] ) );
    bad( mod );
}

TEST_CASE( "verify iadd with undefined rhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.iadd( fn.args[0], 99 ) );
    bad( mod );
}

TEST_CASE( "verify isub with undefined lhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.isub( 99, fn.args[0] ) );
    bad( mod );
}

TEST_CASE( "verify imul with undefined rhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.imul( fn.args[0], 99 ) );
    bad( mod );
}

TEST_CASE( "verify idiv with undefined lhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.idiv( 99, fn.args[0] ) );
    bad( mod );
}

TEST_CASE( "verify iand with undefined operand throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.iand( fn.args[0], 99 ) );
    bad( mod );
}

TEST_CASE( "verify ior with undefined operand throws",    "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ior( 99, fn.args[0] ) );
    bad( mod );
}

TEST_CASE( "verify ixor with undefined operand throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ixor( fn.args[0], 99 ) );
    bad( mod );
}

TEST_CASE( "verify ishl with undefined operand throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishl( 99, fn.args[0] ) );
    bad( mod );
}

TEST_CASE( "verify ishr with undefined operand throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishr( fn.args[0], 99 ) );
    bad( mod );
}

TEST_CASE( "verify inot with undefined operand throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( fn.inot( 99 ) );
    bad( mod );
}

TEST_CASE( "verify ineg with undefined operand throws",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( fn.ineg( 99 ) );
    bad( mod );
}

TEST_CASE( "verify icmp with undefined lhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::eq, 99, fn.args[0] ) );
    bad( mod );
}

TEST_CASE( "verify icmp with undefined rhs throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.icmp( set_cc_kind::eq, fn.args[0], 99 ) );
    bad( mod );
}

TEST_CASE( "verify load with undefined base throws",      "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( fn.load( 99, 0, 8, false ) );
    bad( mod );
}

TEST_CASE( "verify store with undefined base throws",     "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.store( fn.args[0], 99, 0, 8 );
    fn.ret( fn.args[0] );
    bad( mod );
}

TEST_CASE( "verify store with undefined value throws",    "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.store( 99, fn.args[0], 0, 8 );
    fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify brif with undefined condition throws", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.brif( 99, 1, 2, {}, {} );
    fn.create_block( {} ); fn.ret( fn.iconst( 1 ) );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify brif with undefined true_arg throws",  "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 1, 2, { 99 }, {} );
    fn.create_block( { type_t::i64 } ); fn.ret( fn.param_indices[1][0] );
    fn.create_block( {} ); fn.ret( fn.iconst( 0 ) );
    bad( mod );
}

TEST_CASE( "verify brif with undefined false_arg throws", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 1, 2, {}, { 99 } );
    fn.create_block( {} ); fn.ret( fn.iconst( 1 ) );
    fn.create_block( { type_t::i64 } ); fn.ret( fn.param_indices[2][0] );
    bad( mod );
}

TEST_CASE( "verify jmp with undefined arg throws",        "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.jmp( 1, { 99 } );
    fn.create_block( { type_t::i64 } );
    fn.ret( fn.param_indices[1][0] );
    bad( mod );
}

TEST_CASE( "verify call with undefined arg throws",       "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    call_target_t target { .target = reinterpret_cast<const void*>( 0x1234 ) };
    fn.ret( fn.call( { 99 }, target ) );
    bad( mod );
}

TEST_CASE( "verify jit call to nonexistent function throws", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    call_target_t target { .target = std::string( "does_not_exist" ) };
    fn.ret( fn.call( {}, target ) );
    bad( mod );
}

TEST_CASE( "verify jit call to existing function passes", "[verify]" )
{
    module_t mod;
    auto& helper = mod.create_function( "helper", {}, type_t::i64 );
    helper.ret( helper.iconst( 0 ) );
    auto& caller = mod.create_function( "caller", {}, type_t::i64 );
    call_target_t target { .target = std::string( "helper" ) };
    caller.ret( caller.call( {}, target ) );
    ok( mod );
}

TEST_CASE( "verify jit call to self passes",              "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "rec", { type_t::i64 }, type_t::i64 );
    call_target_t target { .target = std::string( "rec" ) };
    fn.ret( fn.call( { fn.args[0] }, target ) );
    ok( mod );
}

TEST_CASE( "verify value used before it is defined throws", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    const auto future_id = static_cast<Value>( fn.types.size( ) + 5 );
    fn.ret( future_id );
    bad( mod );
}

TEST_CASE( "verify brif targets same block both sides",   "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    auto c = fn.icmp( set_cc_kind::eq, fn.args[0], fn.args[1] );
    fn.brif( c, 1, 1, {}, {} );
    fn.create_block( {} );
    fn.ret( fn.iconst( 0 ) );
    ok( mod );
}

TEST_CASE( "verify multiple errors — first one reported",  "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    fn.ret( 99 );
    fn.iconst( 0 );
    bad( mod );
}

TEST_CASE( "verify block param used as operand is valid",  "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.jmp( 1, { fn.args[0] } );
    fn.create_block( { type_t::i64 } );
    auto p = fn.param_indices[1][0];
    fn.ret( fn.iadd( p, p ) );
    ok( mod );
}

TEST_CASE( "verify store then load same address is valid", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer, type_t::i64 }, type_t::i64 );
    fn.store( fn.args[1], fn.args[0], 0, 8 );
    fn.ret( fn.load( fn.args[0], 0, 8, false ) );
    ok( mod );
}

TEST_CASE( "verify chained loads are valid",               "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    auto v1 = fn.load( fn.args[0], 0, 8, false );
    auto v2 = fn.load( fn.args[0], 8, 8, false );
    fn.ret( fn.iadd( v1, v2 ) );
    ok( mod );
}

TEST_CASE( "verify result of load used as store base is valid", "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer, type_t::i64 }, type_t::i64 );
    auto ptr2 = fn.load( fn.args[0], 0, 8, false );
    fn.store( fn.args[1], ptr2, 0, 8 );
    fn.ret( fn.args[1] );
    ok( mod );
}

TEST_CASE( "verify many consts all valid",                 "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", {}, type_t::i64 );
    Value acc = fn.iconst( 0 );
    for ( std::int64_t i = 1; i <= 20; ++i )
        acc = fn.iadd( acc, fn.iconst( i ) );
    fn.ret( acc );
    ok( mod );
}

TEST_CASE( "verify deep block chain is valid",             "[verify]" )
{
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.jmp( 1, { fn.args[0] } );
    for ( std::size_t i = 1; i <= 5; ++i )
    {
        fn.create_block( { type_t::i64 } );
        if ( i < 5 ) fn.jmp( i + 1, { fn.param_indices[i][0] } );
        else         fn.ret( fn.param_indices[i][0] );
    }
    ok( mod );
}