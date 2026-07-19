#include "amd64/ir/fmt.hpp"

#include <print>
#include <amd64/assembler/assembler.hpp>
#include <amd64/codegen/codegen.hpp>
#include <amd64/ir/ir.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

using namespace amd64::ir;
using namespace amd64::codegen;
using namespace amd64::assembler;

TEST_CASE( "iand executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.iand( fn.args[0], fn.args[1] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "f" );
    REQUIRE( f( 0xFF, 0x0F ) == 0x0F );
    REQUIRE( f( 0xFF, 0x00 ) == 0x00 );
}

TEST_CASE( "ior executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ior( fn.args[0], fn.args[1] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "f" );
    REQUIRE( f( 0xF0, 0x0F ) == 0xFF );
    REQUIRE( f( 0x00, 0x00 ) == 0x00 );
}

TEST_CASE( "ixor executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ixor( fn.args[0], fn.args[1] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "f" );
    REQUIRE( f( 0xFF, 0xFF ) == 0x00 );
    REQUIRE( f( 0xF0, 0x0F ) == 0xFF );
}

TEST_CASE( "inot executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.inot( fn.args[0] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );
    REQUIRE( f( 0 ) == ~0LL );
    REQUIRE( f( ~0LL ) == 0 );
}

TEST_CASE( "ishl executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishl( fn.args[0], fn.args[1] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "f" );
    REQUIRE( f( 1, 4 ) == 16 );
    REQUIRE( f( 1, 0 ) == 1 );
}

TEST_CASE( "ishr executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64, type_t::i64 }, type_t::i64 );
    fn.ret( fn.ishr( fn.args[0], fn.args[1] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "f" );
    REQUIRE( f( 256, 4 ) == 16 );
    REQUIRE( f( 1, 0 ) == 1 );
}

TEST_CASE( "ineg executes correctly", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::i64 }, type_t::i64 );
    fn.ret( fn.ineg( fn.args[0] ) );
    auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::int64_t )>( "f" );
    REQUIRE( f( 5 ) == -5 );
    REQUIRE( f( -3 ) == 3 );
}


TEST_CASE( "load.i64 reads from memory", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;

    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.ret( fn.load( fn.args[0], 0, 8, false ) );

    const auto gen = cg.compile_module( mod );

    const auto f = gen.get_function< std::int64_t(*)( std::int64_t* ) >( "f" );

    std::int64_t val = 42;
    REQUIRE( f( &val ) == 42 );
}



TEST_CASE( "load.i8 zero extends", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.ret( fn.load( fn.args[0], 0, 1, false ) );

    const auto gen = cg.compile_module( mod );
    auto f = gen.get_function<std::int64_t(*)( std::uint8_t* )>( "f" );
    std::uint8_t val = 0xFF;
    REQUIRE( f( &val ) == 0xFF );
}

TEST_CASE( "load.i8s sign extends", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.ret( fn.load( fn.args[0], 0, 1, true ) );
    auto gen = cg.compile_module( mod );

    auto bytes = azm.bytes();
    for ( auto b : bytes )
        std::print( "{:02x} ", static_cast<std::uint8_t>(b) );

    auto f = gen.get_function<std::int64_t(*)( std::int8_t* )>( "f" );
    std::int8_t val = -1;
    REQUIRE( f( &val ) == -1LL );
}

TEST_CASE( "store.i64 writes to memory", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer, type_t::i64 }, type_t::i64 );
    fn.store( fn.args[1], fn.args[0], 0, 8 );
    fn.ret( fn.args[1] );

    std::println("{}", to_string( mod ));


    auto gen = cg.compile_module( mod );

    auto bytes = azm.bytes();
    for ( auto b : bytes )
        std::print( "{:02x} ", static_cast<std::uint8_t>(b) );

    auto f = gen.get_function<std::int64_t(*)( std::int64_t*, std::int64_t )>( "f" );
    std::int64_t buf = 0;
    f( &buf, 99 );
    REQUIRE( buf == 99 );
}

TEST_CASE( "store.i8 writes one byte", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;

    auto& fn = mod.create_function( "f", { type_t::pointer, type_t::i64 }, type_t::i64 );

    auto [ptr, val] = fn.get_args< 2 >( );



    fn.store( val, ptr, 0, 1 );
    fn.ret( val );

    auto gen = cg.compile_module( mod );

    auto bytes = azm.bytes();
    for ( auto b : bytes )
        std::print( "{:02x} ", static_cast<std::uint8_t>(b) );

    auto f = gen.get_function<std::int64_t(*)( std::uint8_t*, std::int64_t )>( "f" );
    std::uint8_t buf[2] = { 0, 0 };
    f( buf, 0xAB );
    REQUIRE( buf[0] == 0xAB );
    REQUIRE( buf[1] == 0x00 );
}

TEST_CASE( "load with nonzero offset reads correct field", "[codegen_ext]" )
{
    Assembler azm{ 1024 };
    code_gen_t cg( azm );
    module_t mod;
    auto& fn = mod.create_function( "f", { type_t::pointer }, type_t::i64 );
    fn.ret( fn.load( fn.args[0], 8, 8, false ) );
    auto gen = cg.compile_module( mod );

    auto bytes = azm.bytes();
    for ( auto b : bytes )
        std::print( "{:02x} ", static_cast<std::uint8_t>(b) );

    auto f = gen.get_function<std::int64_t(*)( std::int64_t* )>( "f" );
    std::int64_t arr[2] = { 10, 20 };
    REQUIRE( f( arr ) == 20 );
}
