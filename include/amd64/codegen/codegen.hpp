#pragma once

#include <amd64/ir/ir.hpp>
#include <amd64/assembler/assembler.hpp>
#include <amd64/codegen/allocation.hpp>

#include <array>
#include <cstdint>
#include <ranges>
#include <string>
#include <unordered_map>

#define max(a, b) (((a) > (b)) ? (a) : (b))

namespace amd64::codegen
{
    using namespace amd64::ir;
    using namespace amd64::assembler;
    using namespace amd64::assembler::registers;
    using namespace amd64::codegen::allocation;

    inline constexpr std::size_t GEN_REG_COUNT  = 8;
    inline constexpr std::size_t PTR_SIZE       = 8;

    /// TODO: add system v (for linux) register lists
    inline constexpr std::array CALLEE_SAVED {
        Register::rbx, Register::rsi, Register::rdi,
        Register::r12, Register::r13, Register::r14,
        Register::r15
    };

    inline constexpr std::array ARG_REGISTERS {
        Register::rcx, Register::rdx, Register::r8, Register::r9,
    };

    inline bool is_callee_register( const Register reg ) noexcept
    {
        for ( const auto& r : CALLEE_SAVED ) if ( reg == r ) return true;
        return false;
    }

    struct gen_module_t
    {
        std::unordered_map< std::string, const void* > functions { };

        template < typename F >
        F get_function( const std::string& sym ) const
        {
            const auto iter = functions.find( sym );
            return iter != functions.end( ) ? reinterpret_cast< F >( iter->second ) : nullptr;
        }
    };

    class code_gen_t
    {
    public:
        explicit code_gen_t( Assembler& azm ) : m_asm( azm ) { }

        std::vector< live_range_t > compute_live_ranges( const function_t& func )
        {
            std::unordered_map< Value, live_range_t > live_ranges { };
            std::uint32_t inst_idx { };

            for ( const auto arg_v : func.args )
            {
                live_ranges[ arg_v ] = live_range_t{ arg_v, inst_idx, 0 };
                ++inst_idx;
            }

            for ( auto block_idx { 0uz }; block_idx < func.blocks.size( ); ++block_idx )
            {
                const auto &[parameters, instructions ] = func.blocks[ block_idx ];

                for ( auto param_idx { 0uz }; param_idx < parameters.size( ); ++param_idx )
                {
                    const auto value_idx = func.param_indices[ block_idx ][ param_idx ];
                    live_ranges[ value_idx ] = live_range_t{ value_idx, inst_idx, 0 };
                    ++inst_idx;
                }

                for ( const auto& inst : instructions )
                {
                    std::visit( [& ]< typename T0 >( const T0& i )
                    {
                        using T = std::decay_t<T0>;

                        /// TODO: design a nicer way to handle this, it's terrible right now

                        if constexpr ( std::is_same_v<T, i_const> || std::is_same_v<T, i_add>  ||
                                       std::is_same_v<T, i_sub>   || std::is_same_v<T, i_mul>  ||
                                       std::is_same_v<T, i_div>   || std::is_same_v<T, i_cmp>  ||
                                       std::is_same_v<T, i_call> )
                        {
                            live_ranges[ i.result ] = live_range_t{ i.result, inst_idx, 0 };
                        }
                    }, inst );

                    ++inst_idx;
                }
            }

            /**
             * walk every operand usage, now "end" is populated from the last instruction
             * index where each value was used
             */
            inst_idx = static_cast< std::uint32_t >( func.args.size(  ) );

            for ( const auto &[parameters, instructions ] : func.blocks )
            {
                inst_idx += static_cast< std::uint32_t >( parameters.size( ) );

                for ( const auto& inst : instructions )
                {
                    std::visit( [& ]< typename T0 >( const T0& i )
                    {
                        using T = std::decay_t<T0>;

                        /// TODO: design a nicer way to handle this, it's terrible right now

                        if constexpr ( std::is_same_v<T, i_add> || std::is_same_v<T, i_sub> ||
                                       std::is_same_v<T, i_mul> || std::is_same_v<T, i_div> ||
                                       std::is_same_v<T, i_cmp> )
                        {
                            live_ranges.at( i.lhs ).end = inst_idx;
                            live_ranges.at( i.rhs ).end = inst_idx;
                        }
                        else if constexpr ( std::is_same_v<T, i_call> )
                        {
                            live_ranges.at( i.result ).end = inst_idx;
                        }
                        else if constexpr ( std::is_same_v<T, i_brif> )
                        {
                            live_ranges.at( i.condition ).end = inst_idx;
                        }
                        else if constexpr ( std::is_same_v<T, i_ret> )
                        {
                            live_ranges.at( i.value ).end = inst_idx;
                        }
                        else if constexpr ( std::is_same_v<T, i_jmp> )
                        {
                            for ( const auto arg : i.args ) live_ranges.at( arg ).end = inst_idx;
                        }
                    }, inst );

                    ++inst_idx;
                }
            }

            std::vector< live_range_t > result;
            result.reserve( live_ranges.size( ) );

            for ( const auto &range : live_ranges | std::views::values ) result.push_back( range );
            return result;
        }

        std::size_t compute_max_live( const std::vector< live_range_t >& live_ranges )
        {
            std::vector< std::size_t > bounds { };
            for ( const auto& r : live_ranges ) { bounds.push_back( r.start ); bounds.push_back( r.end ); }

            std::size_t peak { };
            for ( const auto bound : bounds )
            {
                std::size_t count { };
                for ( const auto& r : live_ranges ) if ( r.start <= bound && bound <= r.end ) ++count;
                peak = max( peak, count );
            }

            return peak;
        }
    private:
        Assembler& m_asm;
    };
}