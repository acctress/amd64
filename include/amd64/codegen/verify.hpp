#pragma once

#include <algorithm>
#include <amd64/ir/ir.hpp>
#include <format>
#include <unordered_set>

using namespace amd64::ir;

namespace amd64::codegen
{
    class verify_error : public std::runtime_error
    {
    public:
        explicit verify_error( const std::string &msg ) : runtime_error( std::format( "[ir::verifier] {}", msg ) ) { }
    };

    inline void verify_function( const function_t &func, const module_t &mod )
    {
        std::unordered_set< Value > defined_args {};
        for ( const auto arg : func.args )
            defined_args.insert( arg );

        for ( auto bidx { 0uz }; bidx < func.blocks.size( ); bidx++ )
        {
            const auto &[ parameters, instructions ] = func.blocks[ bidx ];

            /* check for empty block */
            if ( instructions.empty( ) )
                throw verify_error( std::format( "block {} is empty", bidx ) );

            /* check for terminator instruction (last instruction) */
            if ( const auto &last = instructions.back( );
                 !( std::holds_alternative< i_ret >( last ) || std::holds_alternative< i_brif >( last )
                    || std::holds_alternative< i_jmp >( last ) ) )
            {
                throw verify_error( std::format( "bb{} does not end with a terminator (brif, jmp, ret)", bidx ) );
            }

            /* seed args set with block params */
            for ( const auto param : func.param_indices[ bidx ] ) defined_args.insert( param );

            for ( auto i_idx { 0uz }; i_idx < instructions.size( ); i_idx++ )
            {
                const auto& inst = instructions[ i_idx ];

                /* if there is a terminator inst in the middle of the block instructions (why????) throw error */
                if ( i_idx + 1 < instructions.size( ) )
                {
                    if ( std::holds_alternative<i_ret>( inst ) || std::holds_alternative<i_jmp>( inst )
                      || std::holds_alternative<i_brif>( inst ) )
                        throw verify_error( std::format( "{}:bb{}: terminator at instruction {} is not the last instruction", func.name, bidx, i_idx ) );
                }

                std::visit( [& ]< typename T0 >( const T0& i )
                {
                    using T = std::decay_t< T0 >;

                    /* check if all operands are in defined_set */
                    auto req = [&]( const Value v )
                    {
                        if ( !defined_args.contains( v ) )
                            throw verify_error( std::format( "{}:bb{}: value %{} used before definition", func.name, bidx, v ) );
                    };

                    if constexpr ( std::is_same_v< T, i_add > || std::is_same_v< T, i_sub >
                                || std::is_same_v< T, i_mul > || std::is_same_v< T, i_div >
                                || std::is_same_v< T, i_and > || std::is_same_v< T, i_or >
                                || std::is_same_v< T, i_xor > || std::is_same_v< T, i_shl >
                                || std::is_same_v< T, i_shr > || std::is_same_v< T, i_cmp >
                                || std::is_same_v< T, i_udiv > )
                    {
                        req( i.lhs ); req( i.rhs );
                    }
                    else if constexpr ( std::is_same_v<T, i_shl_imm> || std::is_same_v<T, i_shr_imm> )
                        req( i.lhs );
                    else if constexpr ( std::is_same_v< T, i_not > || std::is_same_v< T, i_neg > )
                        req( i.value );
                    else if constexpr ( std::is_same_v< T, i_load > )
                        req( i.base );
                    else if constexpr ( std::is_same_v< T, i_store > )
                        { req( i.value ); req( i.base ); }
                    else if constexpr ( std::is_same_v< T, i_ret > )
                        req( i.value );
                    else if constexpr ( std::is_same_v< T, i_brif > )
                    {
                        req( i.condition );
                        for ( const auto a : i.true_args )  req( a );
                        for ( const auto a : i.false_args ) req( a );
                    }
                    else if constexpr ( std::is_same_v< T, i_jmp > )
                        for ( const auto a : i.args ) req( a );
                    else if constexpr ( std::is_same_v< T, i_call > )
                    {
                        for ( const auto a : i.args ) req( a );

                    /* check i_call with a string target and it exists in the module */
                        if ( std::holds_alternative< std::string >( i.target.target ) )
                        {
                            const auto& sym = std::get< std::string >( i.target.target );
                            if ( const bool exists = std::ranges::any_of( mod.functions,
                                                                          [ & ]( const auto &f )
                                                                          {
                                                                              return f.name == sym;
                                                                          } );
                                 !exists )
                                throw verify_error( std::format( "{}:bb{}: call target '{}' not found in moduke", func.name, bidx, sym ) );
                        }
                    }

                    /* check if jmp/brif target indices and arg counts */
                    if constexpr ( std::is_same_v< T, i_jmp > )
                    {
                        if ( i.to_block >= func.blocks.size(  ) )
                            throw verify_error( std::format( "{}:bb{}: jmp target bb{} out of range", func.name, bidx, i.to_block ) );
                        if ( i.args.size( ) != func.param_indices[ i.to_block ].size( ) )
                            throw verify_error( std::format( "{}:bb:{} jmp to bb{} passes {} args but block expects {}", func.name,
                                bidx, i.to_block, i.args.size( ), func.param_indices[ i.to_block ].size( ) ) );
                    }
                    else if constexpr ( std::is_same_v<T, i_brif > )
                    {
                        for ( const auto [blk, args] : { std::pair{ i.true_blk, &i.true_args }, std::pair{ i.false_blk, &i.false_args } } )
                        {
                            if ( blk >= func.blocks.size(  ))
                                throw verify_error( std::format( "{}:bb{}: brif target bb{} out of range", func.name, bidx, blk ) );
                            if ( args->size( ) != func.param_indices[ blk ].size( ) )
                                throw verify_error( std::format( "{}:bb{}: brif to bb{} passes {} args but block expects {}", func.name,
                                    bidx, blk, args->size( ), func.param_indices[ bidx ].size( ) ) );
                        }
                    }

                    /* after every check insert instruction result into defined_set for results produced */
                    if constexpr ( std::is_same_v<T, i_const> || std::is_same_v<T, i_add> || std::is_same_v<T, i_sub>
                                || std::is_same_v<T, i_mul>   || std::is_same_v<T, i_div> || std::is_same_v<T, i_cmp>
                                || std::is_same_v<T, i_call>  || std::is_same_v<T, i_load>
                                || std::is_same_v<T, i_and>   || std::is_same_v<T, i_or>  || std::is_same_v<T, i_xor>
                                || std::is_same_v<T, i_not>   || std::is_same_v<T, i_neg> || std::is_same_v<T, i_shl>
                                || std::is_same_v<T, i_shr>   || std::is_same_v< T, i_udiv > || std::is_same_v< T, i_shl_imm >
                                || std::is_same_v< T, i_shr_imm > || std::is_same_v< T, i_sar_imm >)
                    {
                        defined_args.insert( i.result );
                    }
                }, inst );
            }
        }
    }

    inline void verify_module( const module_t &mod )
    {
        for ( const auto &func : mod.functions )
            verify_function( func, mod );
    }
} // namespace amd64::codegen