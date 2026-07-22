#pragma once

#include <amd64/assembler/assembler.hpp>

#include <print>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace amd64::ir
{
    using set_cc_kind = assembler::set_cc_kind;
    using Value       = std::uint32_t;

    enum class type_t
    {
        i64,
        i32,
        boolean,
        pointer
    };

    /*
     * Instruction definitions are prefixed with "i_"
     */

    struct i_const
    {
        Value        result;
        std::int64_t constant;
    };

    struct i_add
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_sub
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_mul
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_div
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_udiv
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_cmp
    {
        Value       result;
        set_cc_kind kind;
        Value       lhs;
        Value       rhs;
    };

    struct i_and
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_or
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_xor
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_not
    {
        Value result;
        Value value;
    };

    struct i_shl
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_shl_imm
    {
        Value        result;
        Value        lhs;
        std::uint8_t imm;
    };

    struct i_shr
    {
        Value result;
        Value lhs;
        Value rhs;
    };

    struct i_shr_imm
    {
        Value        result;
        Value        lhs;
        std::uint8_t imm;
    };

    struct i_sar_imm
    {
        Value        result;
        Value        lhs;
        std::uint8_t imm;
    };

    struct i_neg
    {
        Value result;
        Value value;
    };

    ///@brief Call a native function pointer, or an internal name
    struct call_target_t
    {
        std::variant< const void *, std::string > target;
    };

    struct i_call
    {
        Value                result;
        std::vector< Value > args;
        call_target_t        target;
    };

    struct i_brif
    {
        Value                condition;
        std::size_t          true_blk;
        std::size_t          false_blk;
        std::vector< Value > true_args;
        std::vector< Value > false_args;
    };

    struct i_jmp
    {
        std::size_t          to_block;
        std::vector< Value > args;
    };

    struct i_ret
    {
        Value value;
    };

    struct i_load
    {
        Value        result;
        Value        base;
        std::int32_t offset;
        std::uint8_t width;
        bool         sign_extended;
    };

    struct i_store
    {
        Value        value;
        Value        base;
        std::int32_t offset;
        std::uint8_t width;
    };

    using Inst = std::variant< i_const, i_add, i_sub, i_mul, i_div, i_udiv, i_cmp, i_call, i_brif, i_jmp, i_ret, i_and, i_or, i_xor, i_not, i_shl,
                               i_shr, i_shl_imm, i_shr_imm, i_sar_imm, i_neg, i_load, i_store >;

    struct basic_block_t
    {
        std::vector< type_t > parameters;
        std::vector< Inst >   instructions;
    };

    struct function_t
    {
        std::string                         name;
        type_t                              return_type {};
        std::vector< Value >                args;
        std::vector< type_t >               params;
        std::vector< std::vector< Value > > param_indices;
        std::vector< type_t >               types;
        std::vector< basic_block_t >        blocks;

        template < std::size_t... I > auto get_args_imp( std::index_sequence< I... > ) const
        {
            return std::tuple { get_arg( I ).value( )... };
        }

        template < std::size_t I > auto get_args( ) const { return get_args_imp( std::make_index_sequence< I > {} ); }

        Value iconst( const std::int64_t imm ) { return emit< i_const >( type_t::i64, imm ); }

        Value iadd( const Value lhs, const Value rhs ) { return emit< i_add >( type_t::i64, lhs, rhs ); }

        Value isub( const Value lhs, const Value rhs ) { return emit< i_sub >( type_t::i64, lhs, rhs ); }

        Value imul( const Value lhs, const Value rhs ) { return emit< i_mul >( type_t::i64, lhs, rhs ); }

        Value idiv( const Value lhs, const Value rhs ) { return emit< i_div >( type_t::i64, lhs, rhs ); }

        Value udiv( const Value lhs, const Value rhs ) { return emit< i_udiv >( type_t::i64, lhs, rhs ); }

        Value iand( const Value lhs, const Value rhs ) { return emit< i_and >( type_t::i64, lhs, rhs ); }

        Value ior( const Value lhs, const Value rhs ) { return emit< i_or >( type_t::i64, lhs, rhs ); }

        Value ixor( const Value lhs, const Value rhs ) { return emit< i_xor >( type_t::i64, lhs, rhs ); }

        Value ishl( const Value lhs, const Value rhs ) { return emit< i_shl >( type_t::i64, lhs, rhs ); }

        Value ishl_imm( const Value lhs, const std::uint8_t imm ) { return emit< i_shl_imm >( type_t::i64, lhs, imm ); }

        Value ishr( const Value lhs, const Value rhs ) { return emit< i_shr >( type_t::i64, lhs, rhs ); }

        Value ishr_imm( const Value lhs, const std::uint8_t imm ) { return emit< i_shl_imm >( type_t::i64, lhs, imm ); }

        Value isar_imm( const Value lhs, const std::uint8_t imm ) { return emit< i_sar_imm >( type_t::i64, lhs, imm ); }

        Value inot( const Value value ) { return emit< i_not >( type_t::i64, value ); }

        Value ineg( const Value value ) { return emit< i_neg >( type_t::i64, value ); }

        Value icmp( const set_cc_kind kind, const Value lhs, const Value rhs ) { return emit< i_cmp >( type_t::boolean, kind, lhs, rhs ); }

        Value call( std::vector< Value > call_args, const call_target_t &target )
        {
            return emit< i_call >( type_t::boolean, std::move( call_args ), target );
        }

        void ret( const Value value ) { emit_term< i_ret >( value ); }

        void jmp( const std::size_t to_block, std::vector< Value > blk_args ) { emit_term< i_jmp >( to_block, std::move( blk_args ) ); }

        void brif( const Value condition, const std::size_t true_blk, const std::size_t false_blk, std::vector< Value > true_args,
                   std::vector< Value > false_args )
        {
            emit_term< i_brif >( condition, true_blk, false_blk, std::move( true_args ), std::move( false_args ) );
        }

        void store( const Value value, const Value base, const std::int32_t offset, const std::uint8_t width )
        {
            if ( blocks.empty( ) )
                throw std::runtime_error( "no available blocks in function" );
            blocks.back( ).instructions.emplace_back( i_store { value, base, offset, width } );
        }

        Value load( const Value base, const std::int32_t offset, const std::uint8_t width, const bool sign_extend )
        {
            if ( blocks.empty( ) )
                throw std::runtime_error( "no available blocks in function" );

            const auto result = static_cast< Value >( types.size( ) );
            blocks.back( ).instructions.emplace_back( i_load { result, base, offset, width, sign_extend } );
            types.push_back( type_t::i64 );
            return result;
        }

        basic_block_t &create_block( std::vector< type_t > blk_params )
        {
            basic_block_t block { .parameters = std::move( blk_params ), .instructions = {} };

            std::vector< Value > indices { };
            for ( const auto &t : block.parameters )
            {
                types.push_back( t );
                indices.push_back( static_cast< Value >( types.size( ) - 1 ) );
            }

            param_indices.push_back( std::move( indices ) );
            blocks.push_back( std::move( block ) );

            return blocks.back( );
        }

        [[nodiscard]] std::optional< Value > get_arg( const std::size_t index ) const
        {
            if ( index >= args.size( ) )
                return std::nullopt;
            return args[ index ];
        }

        template < typename I, typename... Args > Value emit( const type_t result_type, Args &&...argz )
        {
            if ( blocks.empty( ) )
                throw std::runtime_error( "no available blocks in function" );

            const auto res = static_cast< Value >( types.size( ) );
            blocks.back( ).instructions.emplace_back( I { res, std::forward< Args >( argz )... } );
            types.push_back( result_type );
            return res;
        }

        template < typename I, typename... Args > void emit_term( Args &&...argz )
        {
            if ( blocks.empty( ) )
                throw std::runtime_error( "no available blocks in function" );
            blocks.back( ).instructions.emplace_back( I { std::forward< Args >( argz )... } );
        }
    };

    struct module_t
    {
        std::vector< function_t > functions;

        function_t &create_function( const std::string_view name, const std::vector< type_t > &params, const type_t return_type )
        {
            function_t function;
            function.name        = name ;
            function.return_type = return_type;
            function.params      = params;

            for ( const auto &p : params )
            {
                function.types.push_back( p );
                function.args.push_back( static_cast< Value >( function.types.size( ) - 1 ) );
            }

            functions.push_back( std::move( function ) );
            function_t &fn = functions.back( );

            fn.create_block( {} );

            return fn;
        }

        function_t &get_function( const std::size_t index ) { return functions[ index ]; }
    };
} // namespace amd64::ir