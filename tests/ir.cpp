#include <catch2/catch_test_macros.hpp>
#include <amd64/ir/ir.hpp>

using namespace amd64::ir;

TEST_CASE( "iconst mints an increasing value id and records i64 type", "[ir]" )
{
    function_t fn;
    fn.create_block( { } );
    auto v0 = fn.iconst( 42 );
    auto v1 = fn.iconst( 7 );

    REQUIRE( v0 == 0 );
    REQUIRE( v1 == 1 );
    REQUIRE( fn.types[v0] == type_t::i64 );
    REQUIRE( fn.types[v1] == type_t::i64 );
}

TEST_CASE( "iconst throws when no block exists", "[ir]" )
{
    function_t fn;
    REQUIRE_THROWS_AS( fn.iconst( 1 ), std::runtime_error );
}

TEST_CASE( "iadd throws when no block exists", "[ir]" )
{
    function_t fn;
    REQUIRE_THROWS_AS( fn.iadd( 0, 1 ), std::runtime_error );
}

TEST_CASE( "iadd records correct operands", "[ir]" )
{
    function_t fn;
    fn.create_block( {} );
    auto a = fn.iconst( 1 );
    auto b = fn.iconst( 2 );
    auto sum = fn.iadd( a, b );

    auto& inst = fn.blocks.back( ).instructions.back( );
    REQUIRE( std::holds_alternative< i_add >( inst ) );
    const auto &[result, lhs, rhs ] = std::get< i_add >( inst );
    REQUIRE( result == sum );
    REQUIRE( lhs == a );
    REQUIRE( rhs == b );
}

TEST_CASE( "isub records correct operands", "[ir]" )
{
    function_t fn;
    fn.create_block( { } );
    const auto a = fn.iconst( 5 );
    const auto b = fn.iconst( 3 );
    fn.isub( a, b );

    const auto& inst = fn.blocks.back( ).instructions.back( );
    REQUIRE( std::holds_alternative< i_sub >( inst ) );
}

TEST_CASE( "imul records correct operands", "[ir]" )
{
    function_t fn;
    fn.create_block( { } );
    const auto a = fn.iconst( 5 );
    const auto b = fn.iconst( 3 );
    fn.imul( a, b );

    const auto& inst = fn.blocks.back().instructions.back();
    REQUIRE( std::holds_alternative<i_mul>( inst ) );
}

TEST_CASE( "idiv records correct operands", "[ir]" )
{
    function_t fn;
    fn.create_block( { } );
    const auto a = fn.iconst( 6 );
    const auto b = fn.iconst( 2 );
    fn.idiv( a, b );

    const auto& inst = fn.blocks.back( ).instructions.back( );
    REQUIRE( std::holds_alternative< i_div >( inst ) );
}

TEST_CASE( "icmp records kind and pushes boolean type", "[ir]" )
{
    function_t fn;
    fn.create_block( { } );
    auto a = fn.iconst( 1 );
    auto b = fn.iconst( 2 );
    auto result = fn.icmp( set_cc_kind::eq, a, b );

    REQUIRE( fn.types[result] == type_t::boolean );

    const auto& inst = fn.blocks.back( ).instructions.back( );
    REQUIRE( std::holds_alternative< i_cmp >( inst ) );
    REQUIRE( std::get< i_cmp >( inst ).kind == set_cc_kind::eq );
}

TEST_CASE( "call with native target records the function pointer", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );
    int dummy = 0;
    call_target_t target { .target = static_cast< const void* >( &dummy ) };
    auto result = fn.call( { }, target );

    const auto& inst = fn.blocks.back( ).instructions.back( );

    REQUIRE( std::holds_alternative< i_call >( inst ) );

    const auto& call_inst = std::get< i_call >( inst );

    REQUIRE( call_inst.result == result );
    REQUIRE( std::holds_alternative< const void* >( call_inst.target.target ) );
}

TEST_CASE( "call with jit target records the name string", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );
    const call_target_t target { .target = std::string( "some_function" ) };
    fn.call( { }, target );

    const auto& inst = fn.blocks.back( ).instructions.back( );
    const auto& call_inst = std::get< i_call >( inst );

    REQUIRE( std::get< std::string >( call_inst.target.target ) == "some_function" );
}

TEST_CASE( "call forwards args correctly", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );
    auto a = fn.iconst( 1 );
    auto b = fn.iconst( 2 );
    call_target_t target { .target = std::string( "f" ) };

    fn.call( { a, b }, target );

    const auto& inst = fn.blocks.back( ).instructions.back( );
    const auto& call_inst = std::get< i_call >( inst );

    REQUIRE( call_inst.args.size( ) == 2 );
    REQUIRE( call_inst.args[0] == a );
    REQUIRE( call_inst.args[1] == b );
}

TEST_CASE( "ret does not mint a new value", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );

    auto v = fn.iconst( 9 );
    auto types_before = fn.types.size( );

    fn.ret( v );

    REQUIRE( fn.types.size() == types_before );

    const auto& inst = fn.blocks.back( ).instructions.back( );

    REQUIRE( std::holds_alternative< i_ret >( inst ) );
    REQUIRE( std::get< i_ret >( inst ).value == v );
}

TEST_CASE( "ret throws when no block exists", "[ir]" )
{
    function_t fn;
    REQUIRE_THROWS_AS( fn.ret( 0 ), std::runtime_error );
}

TEST_CASE( "jmp records target block and args", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );
    auto v = fn.iconst( 1 );
    fn.jmp( 3, { v } );

    const auto& inst = fn.blocks.back( ).instructions.back( );

    REQUIRE( std::holds_alternative< i_jmp >( inst ) );

    const auto &[to_block, args ] = std::get< i_jmp >( inst );

    REQUIRE( to_block == 3 );
    REQUIRE( args.size( ) == 1 );
    REQUIRE( args[0] == v );
}

TEST_CASE( "brif records both branch targets and both arg lists", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );
    auto cond = fn.icmp( set_cc_kind::eq, fn.iconst( 1 ), fn.iconst( 1 ) );
    auto t_arg = fn.iconst( 10 );
    auto f_arg = fn.iconst( 20 );
    fn.brif( cond, 1, 2, { t_arg }, { f_arg } );

    const auto& inst = fn.blocks.back( ).instructions.back( );

    REQUIRE( std::holds_alternative< i_brif >( inst ) );

    const auto &[condition, true_blk, false_blk, true_args, false_args ] = std::get< i_brif >( inst );

    REQUIRE( condition == cond );
    REQUIRE( true_blk == 1 );
    REQUIRE( false_blk == 2 );
    REQUIRE( true_args[0] == t_arg );
    REQUIRE( false_args[0] == f_arg );
}

TEST_CASE( "create_block appends a new block", "[ir]" )
{
    function_t fn;

    fn.create_block( { } );
    fn.create_block( { } );

    REQUIRE( fn.blocks.size( ) == 2 );
}

TEST_CASE( "create_block with parameters records one entry per param in param_indices", "[ir]" )
{
    function_t fn;
    fn.create_block( { type_t::i64, type_t::boolean } );

    REQUIRE( fn.param_indices.size( ) == 1 );
    REQUIRE( fn.param_indices[ 0 ].size( ) == 2 );
    REQUIRE( fn.types[ fn.param_indices[ 0 ][ 0 ] ] == type_t::i64 );
    REQUIRE( fn.types[ fn.param_indices[ 0 ][ 1 ] ] == type_t::boolean );
}

TEST_CASE( "create_block with no parameters still records an empty param_indices entry", "[ir]" )
{
    function_t fn;
    fn.create_block( { } );

    REQUIRE( fn.param_indices.size( ) == 1 );
    REQUIRE( fn.param_indices[ 0 ].empty( ) );
}

TEST_CASE( "each block's param_indices entry stays aligned with its block index", "[ir]" )
{
    function_t fn;
    fn.create_block( { type_t::i64 } );      // block 0
    fn.create_block( { type_t::boolean, type_t::pointer } ); // block 1

    REQUIRE( fn.param_indices[ 0 ].size( ) == 1 );
    REQUIRE( fn.param_indices[ 1 ].size( ) == 2 );
}

TEST_CASE( "get_arg returns nullopt for out-of-range index", "[ir]" )
{
    function_t fn;
    REQUIRE_FALSE( fn.get_arg( 0 ).has_value( ) );
}

TEST_CASE( "get_arg returns the correct value for a valid index", "[ir]" )
{
    function_t fn;
    fn.args = { 5, 6, 7 };
    REQUIRE( fn.get_arg( 1 ).value( ) == 6 );
}

TEST_CASE( "module create_function registers params and creates an entry block", "[ir]" )
{
    module_t mod;
    auto& fn = mod.create_function( "add", { type_t::i64, type_t::i64 }, type_t::i64 );

    REQUIRE( fn.name == "add" );
    REQUIRE( fn.return_type == type_t::i64 );
    REQUIRE( fn.args.size( ) == 2 );
    REQUIRE( fn.blocks.size( ) == 1 ); // entry block created automatically
    REQUIRE( mod.functions.size( ) == 1 );
}

TEST_CASE( "module get_function returns the same function by index", "[ir]" )
{
    module_t mod;
    mod.create_function( "f", { }, type_t::i64 );

    REQUIRE( mod.get_function( 0 ).name == "f" );
}