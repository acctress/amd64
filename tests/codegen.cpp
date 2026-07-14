#include <catch2/catch_test_macros.hpp>
#include <amd64/codegen/codegen.hpp> // Adjust if your code_gen_t is in a differently named header
#include <amd64/assembler/assembler.hpp>
#include <amd64/ir/ir.hpp>

#include <cstdint>
#include <string>

using namespace amd64::codegen;
using namespace amd64::ir;
using namespace amd64::assembler;

TEST_CASE( "codegen: compiles and executes a function returning a constant", "[codegen][execution]" )
{
    Assembler azm { 1024 };
    code_gen_t codegen( azm );
    module_t mod_ir;

    auto& fn = mod_ir.create_function( "ret_const", {}, type_t::i64 );

    const auto val = fn.iconst( 42 );
    fn.ret( val );

    const auto gen_mod = codegen.compile_module( mod_ir );
    auto fn_ptr = gen_mod.get_function<std::int64_t(*)()>( "ret_const" );

    REQUIRE( fn_ptr != nullptr );
    REQUIRE( fn_ptr() == 42 );
}

TEST_CASE( "codegen: compiles and executes addition with function arguments", "[codegen][execution]" )
{
    Assembler azm { 1024 };
    code_gen_t codegen( azm );
    module_t mod_ir;

    auto& fn = mod_ir.create_function( "add_args", { type_t::i64, type_t::i64 }, type_t::i64 );

    auto arg0 = fn.get_arg( 0 ).value();
    auto arg1 = fn.get_arg( 1 ).value();

    auto sum = fn.iadd( arg0, arg1 );
    fn.ret( sum );

    auto gen_mod = codegen.compile_module( mod_ir );
    auto fn_ptr = gen_mod.get_function<std::int64_t(*)(std::int64_t, std::int64_t)>( "add_args" );

    REQUIRE( fn_ptr != nullptr );
    REQUIRE( fn_ptr( 15, 25 ) == 40 );
    REQUIRE( fn_ptr( -10, 5 ) == -5 );
}

TEST_CASE( "codegen: conditionals and branching execution", "[codegen][execution]" )
{
    Assembler azm { 1024 };
    code_gen_t codegen( azm );
    module_t mod_ir;

    auto& fn = mod_ir.create_function( "is_equal", { type_t::i64, type_t::i64 }, type_t::i64 );

    auto arg0 = fn.get_arg( 0 ).value();
    auto arg1 = fn.get_arg( 1 ).value();

    auto cond = fn.icmp( set_cc_kind::eq, arg0, arg1 );
    fn.brif( cond, 1, 2, {}, {} ); /* true -> block 1, false -> block 2 */

    /* true */
    fn.create_block( {} );
    auto ret_true = fn.iconst( 1 );
    fn.ret( ret_true );

    /* false */
    fn.create_block( {} );
    auto ret_false = fn.iconst( 0 );
    fn.ret( ret_false );

    auto gen_mod = codegen.compile_module( mod_ir );
    auto fn_ptr = gen_mod.get_function<std::int64_t(*)(std::int64_t, std::int64_t)>( "is_equal" );

    REQUIRE( fn_ptr != nullptr );
    REQUIRE( fn_ptr( 99, 99 ) == 1 );
    REQUIRE( fn_ptr( 99, 100 ) == 0 );
}

TEST_CASE( "codegen: internal function calls within a module", "[codegen][execution]" )
{
    Assembler azm { 1024 };
    code_gen_t codegen( azm );
    module_t mod_ir;

    /* get_multiplier() returns 5 */
    auto& fn_get = mod_ir.create_function( "get_multiplier", {}, type_t::i64 );
    auto mult_val = fn_get.iconst( 5 );
    fn_get.ret( mult_val );

    /* multiply_by_five(arg) -> arg * get_multiplier() */
    auto& fn_mul = mod_ir.create_function( "multiply_by_five", { type_t::i64 }, type_t::i64 );
    auto arg0 = fn_mul.get_arg( 0 ).value();

    call_target_t target { .target = std::string( "get_multiplier" ) };
    auto call_res = fn_mul.call( {}, target );
    auto final_res = fn_mul.imul( arg0, call_res );

    fn_mul.ret( final_res );

    auto gen_mod = codegen.compile_module( mod_ir );
    auto fn_ptr = gen_mod.get_function<std::int64_t(*)(std::int64_t)>( "multiply_by_five" );

    REQUIRE( fn_ptr != nullptr );
    REQUIRE( fn_ptr( 10 ) == 50 );
    REQUIRE( fn_ptr( 0 ) == 0 );
    REQUIRE( fn_ptr( -2 ) == -10 );
}

TEST_CASE( "codegen: block parameter passing via jmp", "[codegen][execution]" )
{
    Assembler azm { 1024 };
    code_gen_t codegen( azm );
    module_t mod_ir;

    auto& fn = mod_ir.create_function( "pass_through_block", { type_t::i64 }, type_t::i64 );

    auto arg0 = fn.get_arg( 0 ).value();
    auto two = fn.iconst( 2 );
    auto mul_res = fn.imul( arg0, two );

    fn.jmp( 1, { mul_res } );

    fn.create_block( { type_t::i64 } );

    auto block_param = fn.param_indices[1][0];
    fn.ret( block_param );

    auto gen_mod = codegen.compile_module( mod_ir );
    auto fn_ptr = gen_mod.get_function<std::int64_t(*)(std::int64_t)>( "pass_through_block" );

    REQUIRE( fn_ptr != nullptr );
    REQUIRE( fn_ptr( 21 ) == 42 );
}