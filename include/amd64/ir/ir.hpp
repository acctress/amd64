#pragma once

#include <amd64/assembler/assembler.hpp>

#include <cstdint>
#include <utility>
#include <variant>
#include <string>
#include <vector>
#include <stdexcept>

namespace amd64::ir
{
    using set_cc_kind = assembler::set_cc_kind;
    using Value = std::uint32_t;

    enum class type_t { i64, i32, boolean, pointer };

    /*
     * Instruction definitions are prefixed with "i_"
     */

    struct i_const { Value result; std::int64_t constant; };
    struct i_add   { Value result; Value lhs;  Value rhs; };
    struct i_sub   { Value result; Value lhs;  Value rhs; };
    struct i_mul   { Value result; Value lhs;  Value rhs; };
    struct i_div   { Value result; Value lhs;  Value rhs; };

    struct i_cmp   { Value result; set_cc_kind kind; Value lhs; Value rhs; };

    ///@brief Call a native function pointer, or an internal name
    struct call_target_t
    {
        std::variant< const void*, std::string > target;
    };

    struct i_call
    {
        Value result;
        std::vector< Value > args;
        call_target_t target;
    };

    struct i_brif
    {
        Value condition;
        std::size_t true_blk;
        std::size_t false_blk;
        std::vector< Value > true_args;
        std::vector< Value > false_args;
    };

    struct i_jmp
    {
        std::size_t to_block;
        std::vector< Value > args;
    };

    struct i_ret
    {
        Value value;
    };

    using Inst = std::variant<
        i_const, i_add, i_sub, i_mul, i_div,
        i_cmp, i_call, i_brif, i_jmp, i_ret
    >;

    struct basic_block_t
    {
        std::vector< type_t > parameters;
        std::vector< Inst >   instructions;
    };

    struct function_t
    {
        std::string name;
        type_t return_type { };
        std::vector< Value > args;
        std::vector< type_t > params;
        std::vector< std::vector< Value > > param_indices;
        std::vector< type_t > types;
        std::vector< basic_block_t > blocks;

        auto iconst( const std::int64_t imm )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_const { result, imm } );
            types.push_back( type_t::i64 );
            return result;
        }

        auto iadd( const Value lhs, const Value rhs )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_add { result, lhs, rhs } );
            types.push_back( type_t::i64 );
            return result;
        }

        auto isub( const Value lhs, const Value rhs )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_sub { result, lhs, rhs } );
            types.push_back( type_t::i64 );
            return result;
        }

        auto imul( const Value lhs, const Value rhs )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_mul { result, lhs, rhs } );
            types.push_back( type_t::i64 );
            return result;
        }

        auto idiv( const Value lhs, const Value rhs )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_div { result, lhs, rhs } );
            types.push_back( type_t::i64 );
            return result;
        }

        auto icmp( const set_cc_kind kind, const Value lhs, const Value rhs )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_cmp { result, kind, lhs, rhs } );
            types.push_back( type_t::boolean );
            return result;
        }

        auto call( std::vector< Value > call_args, const call_target_t &target )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size(  ) );
            blocks.back(  ).instructions.emplace_back( i_call { result, std::move( call_args ), target } );
            types.push_back( type_t::boolean );
            return result;
        }

        void jmp( const std::size_t to_block, std::vector<Value> blk_args )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            blocks.back( ).instructions.emplace_back( i_jmp { to_block, std::move( blk_args ) } );
        }

        void brif( const Value condition, const std::size_t true_blk, const std::size_t false_blk,
                   std::vector<Value> true_args, std::vector<Value> false_args )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            blocks.back( ).instructions.emplace_back(
                i_brif { condition, true_blk, false_blk, std::move( true_args ), std::move( false_args ) }
            );
        }

        void ret( const Value value )
        {
            if ( blocks.empty( ) ) throw std::runtime_error( "no available blocks in function" );

            blocks.back( ).instructions.emplace_back( i_ret { value } );
        }

        basic_block_t& create_block( std::vector< type_t > blk_params )
        {
            basic_block_t block { .parameters = std::move( blk_params ), .instructions = { } };

            std::vector< Value > indices { };
            for ( const auto& t : block.parameters )
            {
                types.push_back( t );
                indices.push_back( static_cast< Value >( types.size(  ) - 1 ) );
            }

            param_indices.push_back( std::move( indices ) );
            blocks.push_back( std::move( block ) );

            return blocks.back(  );
        }

        [[nodiscard]] std::optional< Value > get_arg( const std::size_t index ) const
        {
            if ( index >= args.size( ) ) return std::nullopt;
            return args[ index ];
        }
    };

    struct module_t
    {
        std::vector< function_t > functions;

        function_t& create_function( std::string name, const std::vector< type_t >& params, const type_t return_type )
        {
            function_t function;
            function.name = std::move( name );
            function.return_type = return_type;
            function.params = params;

            for ( const auto& p : params )
            {
                function.types.push_back( p );
                function.args.push_back( static_cast< Value >( function.types.size( ) - 1 ) );
            }

            functions.push_back( std::move( function ) );
            function_t& fn = functions.back(  );

            fn.create_block( { } );

            return fn;
        }

        function_t& get_function( const std::size_t index )
        {
            return functions[ index ];
        }
    };
}