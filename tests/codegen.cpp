#include <catch2/catch_test_macros.hpp>
#include <amd64/codegen/codegen.hpp> // Adjust if your code_gen_t is in a differently named header
#include <amd64/assembler/assembler.hpp>
#include <amd64/ir/ir.hpp>
#include <amd64/ir/fmt.hpp>

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

TEST_CASE( "codegen: ishr_imm testfff", "[codegen][execution]" )
{
    Assembler azm { 1024 };code_gen_t codegen( azm );module_t mod_ir;
    auto& fn = mod_ir.create_function( "f", { type_t::i64 }, type_t::i64 );
    auto value = fn.get_arg( 0 ).value(  );
    auto res = fn.ishl_imm( value, 2 );
    fn.ret( res );
    auto gen_mod = codegen.compile_module( mod_ir );
    auto fn_ptr = gen_mod.get_function<std::int64_t(*)(std::int64_t)>( "f" );
    REQUIRE( fn_ptr != nullptr );
    REQUIRE( fn_ptr( 10 ) == 40 );
}

TEST_CASE( "ishl_imm shifts left by immediate", "[codegen][execution]" )
{
    Assembler azm{ 1024 }; code_gen_t cg( azm ); module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishl_imm( fn.args[0], 2 ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );
    REQUIRE( f( 10 ) == 40 );
    REQUIRE( f( 1  ) == 4  );
}

TEST_CASE( "ishr_imm shifts right by immediate", "[codegen][execution]" )
{
    Assembler azm{ 1024 }; code_gen_t cg( azm ); module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishr_imm( fn.args[0], 3 ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );
    REQUIRE( f( 64 ) == 8 );
    REQUIRE( f( 16 ) == 2 );
}

TEST_CASE( "isar_imm arithmetic shift preserves sign", "[codegen][execution]" )
{
    Assembler azm{ 1024 }; code_gen_t cg( azm ); module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.isar_imm( fn.args[0], 1 ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );
    REQUIRE( f( -8 ) == -4 );
    REQUIRE( f(  8 ) ==  4 );
}

TEST_CASE( "udiv unsigned division", "[codegen][execution]" )
{
    Assembler azm{ 1024 }; code_gen_t cg( azm ); module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.udiv( fn.args[0], fn.args[1] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "f" );
    REQUIRE( f( 10,  2 ) == 5  );
    REQUIRE( f( 100, 4 ) == 25 );
}

TEST_CASE( "itrunc32 truncates to 32 bits", "[codegen][execution]" )
{
    Assembler azm{ 1024 }; code_gen_t cg( azm ); module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.itrunc32( fn.args[0] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );
    REQUIRE( f( 0x1FFFFFFFFLL ) == 0xFFFFFFFFLL );
    REQUIRE( f( 0x100000001LL ) == 1LL          );
}

TEST_CASE( "isext32 sign extends 32 to 64 bits", "[codegen][execution]" )
{
    Assembler azm{ 1024 }; code_gen_t cg( azm ); module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.isext32( fn.args[0] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );

    std::println("{}", to_string(mod));

    REQUIRE( f( 0xFFFFFFFFLL ) == -1LL          );
    REQUIRE( f( 0x7FFFFFFFLL ) == 2147483647LL  );
    REQUIRE( f( 0x80000000LL ) == -2147483648LL );
}