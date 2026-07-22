#pragma once

#include <amd64/ir/ir.hpp>

#include <format>
#include <string>
#include <string_view>

namespace amd64::ir
{
    inline std::string_view fmt_set_cc_kind( const set_cc_kind kind )
    {
        switch ( kind )
        {
            case set_cc_kind::eq : return "eq";
            case set_cc_kind::ne : return "ne";
            case set_cc_kind::lt : return "lt";
            case set_cc_kind::le : return "le";
            case set_cc_kind::gt : return "gt";
            case set_cc_kind::ge : return "ge";
            case set_cc_kind::ult : return "ult";
            case set_cc_kind::ule : return "ule";
            case set_cc_kind::ugt : return "ugt";
            case set_cc_kind::uge : return "uge";
        }

        return "?";
    }

    inline std::string_view fmt_type( const type_t ty )
    {
        switch ( ty )
        {
            case type_t::i64 : return "i64";
            case type_t::i32 : return "i32";
            case type_t::boolean : return "bool";
            case type_t::pointer : return "ptr";
        }

        return "?";
    }

    inline std::string_view fmt_load_kind( const std::uint8_t width, const bool sign_extended )
    {
        if ( width == 1 ) return sign_extended ? "i8s" : "i8";
        if ( width == 2 ) return sign_extended ? "i16s" : "i16";
        if ( width == 4 ) return "i32";
        return "i64";
    }

    inline void fmt_inst( const Inst &inst, std::string &fmt )
    {
        std::visit(
            [ & ]< typename T0 >( const T0 &i )
            {
                using T = std::decay_t< T0 >;

                if constexpr ( std::is_same_v< T, i_const > )
                    fmt += std::format( "\t%{} = iconst {}\n", i.result, i.constant );
                else if constexpr ( std::is_same_v< T, i_ret > )
                    fmt += std::format( "\tret %{}\n", i.value );
                else if constexpr ( std::is_same_v< T, i_add > )
                    fmt += std::format( "\t%{} = iadd %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_sub > )
                    fmt += std::format( "\t%{} = isub %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_mul > )
                    fmt += std::format( "\t%{} = imul %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_div > )
                    fmt += std::format( "\t%{} = idiv %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_udiv > )
                    fmt += std::format( "\t%{} = udiv %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_and > )
                    fmt += std::format( "\t%{} = iand %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_or > )
                    fmt += std::format( "\t%{} = ior %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_xor > )
                    fmt += std::format( "\t%{} = ixor %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_not > )
                    fmt += std::format( "\t%{} = inot %{}\n", i.result, i.value );
                else if constexpr ( std::is_same_v< T, i_shl > )
                    fmt += std::format( "\t%{} = ishl %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_shr > )
                    fmt += std::format( "\t%{} = ishr %{}, %{}\n", i.result, i.lhs, i.rhs );
                else if constexpr ( std::is_same_v<T, i_shl_imm> )
                    fmt += std::format( "\t%{} = ishl_imm %{}, {}\n", i.result, i.lhs, i.imm );
                else if constexpr ( std::is_same_v<T, i_shr_imm> )
                    fmt += std::format( "\t%{} = ishr_imm %{}, {}\n", i.result, i.lhs, i.imm );
                else if constexpr ( std::is_same_v<T, i_sar_imm> )
                    fmt += std::format( "\t%{} = isar_imm %{}, {}\n", i.result, i.lhs, i.imm );
                else if constexpr ( std::is_same_v< T, i_neg > )
                    fmt += std::format( "\t%{} = ineg %{}\n", i.result, i.value );
                else if constexpr ( std::is_same_v< T, i_cmp > )
                    fmt += std::format( "\t%{} = icmp.{} %{}, %{}\n", i.result, fmt_set_cc_kind( i.kind ), i.lhs, i.rhs );
                else if constexpr ( std::is_same_v< T, i_load > )
                    fmt += std::format( "\t%{} = load.{} %{}, {}\n", i.result, fmt_load_kind( i.width, i.sign_extended ), i.base, i.offset );
                else if constexpr ( std::is_same_v< T, i_store > )
                    fmt += std::format( "\tstore.{} %{}, %{}, {}\n", fmt_load_kind( i.width, false ), i.value, i.base, i.offset );
                else if constexpr ( std::is_same_v< T, i_call > )
                {
                    if ( std::holds_alternative< const void * >( i.target.target ) )
                        fmt += std::format( "\t%{} = call native@{}(", i.result, std::get< const void * >( i.target.target ) );
                    else
                        fmt += std::format( "\t%{} = call @{}(", i.result, std::get< std::string >( i.target.target ) );

                    for ( auto idx { 0uz }; idx < i.args.size( ); ++idx )
                    {
                        if ( idx != 0 )
                            fmt += ", ";
                        fmt += std::format( "%{}", i.args[ idx ] );
                    }

                    fmt += ")\n";
                }
                else if constexpr ( std::is_same_v< T, i_brif > )
                {
                    fmt += std::format( "\tbrif %{}, bb{}(", i.condition, i.true_blk );

                    for ( auto idx { 0uz }; idx < i.true_args.size( ); ++idx )
                    {
                        if ( idx != 0 )
                            fmt += ", ";
                        fmt += std::format( "%{}", i.true_args[ idx ] );
                    }

                    fmt += std::format( "), bb{}(", i.false_blk );

                    for ( auto idx { 0uz }; idx < i.false_args.size( ); ++idx )
                    {
                        if ( idx != 0 )
                            fmt += ", ";
                        fmt += std::format( "%{}", i.false_args[ idx ] );
                    }

                    fmt += ")\n";
                }
                else if constexpr ( std::is_same_v< T, i_jmp > )
                {
                    fmt += std::format( "\tjmp bb{}(", i.to_block );

                    for ( auto idx { 0uz }; idx < i.args.size( ); ++idx )
                    {
                        if ( idx != 0 )
                            fmt += ", ";
                        fmt += std::format( "%{}", i.args[ idx ] );
                    }

                    fmt += ")\n";
                }
            },
            inst );
    }

    inline void fmt_block( const basic_block_t &block, const std::size_t block_idx, const std::vector< Value > &param_indices,
                           std::string &fmt ) noexcept
    {
        fmt += std::format( "bb{}(", block_idx );

        for ( auto i { 0uz }; i < block.parameters.size( ); ++i )
        {
            if ( i != 0 )
                fmt += ", ";
            fmt += std::format( "{} %{}", fmt_type( block.parameters[ i ] ), param_indices[ i ] );
        }

        fmt += "):\n";

        for ( const auto &inst : block.instructions )
            fmt_inst( inst, fmt );
    }

    inline void fmt_function( const function_t &func, std::string &fmt )
    {
        fmt += std::format( "fn @{}(", func.name );

        for ( auto i { 0uz }; i < func.params.size( ); ++i )
        {
            if ( i != 0 )
                fmt += ", ";
            fmt += std::format( "{} %{}", fmt_type( func.params[ i ] ), func.args[ i ] );
        }

        fmt += std::format( ") -> {} {{\n", fmt_type( func.return_type ) );

        for ( auto bi { 0uz }; bi < func.blocks.size( ); ++bi )
        {
            fmt_block( func.blocks[ bi ], bi, func.param_indices[ bi ], fmt );
            if ( bi + 1 != func.blocks.size( ) )
                fmt += "\n";
        }

        fmt += "}\n";
    }

    inline void fmt_module( const module_t &mod, std::string &fmt )
    {
        for ( auto i { 0uz }; i < mod.functions.size( ); ++i )
        {
            if ( i != 0 )
                fmt += "\n";
            fmt_function( mod.functions[ i ], fmt );
            fmt += "\n";
        }
    }

    inline std::string to_string( const module_t &mod )
    {
        std::string fmt;
        fmt_module( mod, fmt );
        return fmt;
    }

    inline std::string to_string( const function_t &func )
    {
        std::string fmt;
        fmt_function( func, fmt );
        return fmt;
    }
} // namespace amd64::ir