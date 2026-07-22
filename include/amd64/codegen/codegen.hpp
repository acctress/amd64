#pragma once

#include "verify.hpp"

#include <amd64/assembler/assembler.hpp>
#include <amd64/codegen/allocation.hpp>
#include <amd64/ir/ir.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>
#include <string>
#include <unordered_map>

namespace amd64::codegen
{
    using namespace amd64::ir;
    using namespace amd64::assembler;
    using namespace amd64::assembler::registers;
    using namespace amd64::codegen::allocation;

    inline constexpr std::size_t GEN_REG_COUNT = 5;
    inline constexpr std::size_t PTR_SIZE      = 8;

    /// TODO: add system v (for linux) register lists
    inline constexpr std::array CALLEE_SAVED { Register::rbx, Register::rsi, Register::rdi, Register::r12,
                                               Register::r13, Register::r14, Register::r15 };

    inline constexpr std::array ARG_REGISTERS {
        Register::rcx,
        Register::rdx,
        Register::r8,
        Register::r9,
    };

    inline constexpr std::array GENERAL_PURPOSE {
        Register::rbx, Register::r12, Register::r13, Register::r14, Register::r15,
    };

    inline bool is_callee_register( const Register reg ) noexcept { return std::ranges::contains( CALLEE_SAVED, reg ); }

    struct gen_module_t
    {
        std::unordered_map< std::string, const void * > functions { };

        template < typename F > [[nodiscard]] F get_function( const std::string &sym ) const
        {
            const auto it = functions.find( sym );
            if ( it == functions.end( ) )
                return nullptr;
            return reinterpret_cast< F >( const_cast< void * >( it->second ) );
        }
    };

    class code_gen_t
    {
    public:
        explicit code_gen_t( Assembler &azm ) : m_asm( azm ) { }

        static std::vector< live_range_t > compute_live_ranges( const function_t &func )
        {
            std::unordered_map< Value, live_range_t > live_ranges { };
            std::uint32_t                             inst_idx { };

            for ( const auto arg_v : func.args )
            {
                live_ranges[ arg_v ] = live_range_t { arg_v, inst_idx, 0 };
                ++inst_idx;
            }

            for ( auto block_idx { 0uz }; block_idx < func.blocks.size( ); ++block_idx )
            {
                const auto &[ parameters, instructions ] = func.blocks[ block_idx ];

                for ( auto param_idx { 0uz }; param_idx < parameters.size( ); ++param_idx )
                {
                    const auto value_idx     = func.param_indices[ block_idx ][ param_idx ];
                    live_ranges[ value_idx ] = live_range_t { value_idx, inst_idx, 0 };
                    ++inst_idx;
                }

                for ( const auto &inst : instructions )
                {
                    std::visit(
                        [ & ]< typename T0 >( const T0 &i )
                        {
                            using T = std::decay_t< T0 >;

                            /// TODO: design a nicer way to handle this, it's terrible right
                            /// now

                            if constexpr ( std::is_same_v<T, i_const> || std::is_same_v<T, i_add> || std::is_same_v<T, i_sub>
                                        || std::is_same_v<T, i_mul>   || std::is_same_v<T, i_div> || std::is_same_v<T, i_cmp>
                                        || std::is_same_v<T, i_call>  || std::is_same_v<T, i_load> || std::is_same_v<T, i_udiv>
                                        || std::is_same_v<T, i_and>   || std::is_same_v<T, i_or>  || std::is_same_v<T, i_xor>
                                        || std::is_same_v<T, i_not>   || std::is_same_v<T, i_neg> || std::is_same_v<T, i_shl>
                                        || std::is_same_v<T, i_shr>   || std::is_same_v<T, i_shr_imm> || std::is_same_v<T, i_shr_imm>
                                        || std::is_same_v<T, i_sar_imm> )
                            {
                                live_ranges[ i.result ] = live_range_t{ i.result, inst_idx, 0 };
                            }
                        },
                        inst );

                    ++inst_idx;
                }
            }

            /**
             * walk every operand usage, now "end" is populated from the last
             * instruction index where each value was used
             */
            inst_idx = static_cast< std::uint32_t >( func.args.size( ) );

            for ( const auto &[ parameters, instructions ] : func.blocks )
            {
                inst_idx += static_cast< std::uint32_t >( parameters.size( ) );

                for ( const auto &inst : instructions )
                {
                    std::visit(
                        [ & ]< typename T0 >( const T0 &i )
                        {
                            using T = std::decay_t< T0 >;

                            /// TODO: design a nicer way to handle this, it's terrible right
                            /// now

                            if constexpr ( std::is_same_v< T, i_add > || std::is_same_v< T, i_sub > || std::is_same_v< T, i_mul >
                                           || std::is_same_v< T, i_div > || std::is_same_v< T, i_udiv > || std::is_same_v< T, i_cmp >
                                           || std::is_same_v< T, i_and > || std::is_same_v< T, i_or > || std::is_same_v< T, i_xor >
                                           || std::is_same_v< T, i_shl > || std::is_same_v< T, i_shr > )
                            {
                                live_ranges.at( i.lhs ).end = inst_idx;
                                live_ranges.at( i.rhs ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v< T, i_not > || std::is_same_v< T, i_neg > || std::is_same_v< T, i_ret > )
                            {
                                live_ranges.at( i.value ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v<T, i_shl_imm> || std::is_same_v<T, i_shr_imm> || std::is_same_v<T, i_sar_imm> )
                            {
                                live_ranges.at( i.lhs ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v< T, i_call > )
                            {
                                live_ranges.at( i.result ).end = inst_idx;
                                for ( const auto& arg : i.args ) live_ranges.at( arg ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v< T, i_brif > )
                            {
                                live_ranges.at( i.condition ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v< T, i_jmp > )
                            {
                                for ( const auto arg : i.args )
                                    live_ranges.at( arg ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v< T, i_load > )
                            {
                                live_ranges.at( i.base ).end   = inst_idx;
                                live_ranges.at( i.result ).end = inst_idx;
                            }
                            else if constexpr ( std::is_same_v< T, i_store > )
                            {
                                live_ranges.at( i.base ).end  = inst_idx;
                                live_ranges.at( i.value ).end = inst_idx;
                            }
                        },
                        inst );

                    ++inst_idx;
                }
            }

            std::vector< live_range_t > result;
            result.reserve( live_ranges.size( ) );

            for ( const auto &range : live_ranges | std::views::values )
                result.push_back( range );
            return result;
        }

        static std::size_t compute_max_live( const std::vector< live_range_t > &live_ranges )
        {
            std::vector< std::size_t > bounds { };
            for ( const auto &r : live_ranges )
            {
                bounds.push_back( r.start );
                bounds.push_back( r.end );
            }

            auto peak { 0uz };
            for ( const auto bound : bounds )
            {
                auto count { 0uz };
                for ( const auto &r : live_ranges )
                    if ( r.start <= bound && bound <= r.end )
                        ++count;
                peak = std::max( peak, count );
            }

            return peak;
        }

        void compile_function( function_t &func, gen_module_t &mod, const std::string &symbol )
        {
            auto ranges   = compute_live_ranges( func );
            auto set_hint = [ & ]( const Value val, const Register reg )
            {
                if ( const auto iter = std::ranges::find_if( ranges,
                                                             [ & ]( auto &r )
                                                             {
                                                                 return r.value == val;
                                                             } );
                     iter != ranges.end( ) )
                    iter->hint = reg;
            };

            for ( auto idx { 0uz }; idx < func.args.size( ); ++idx )
                set_hint( func.args[ idx ], ARG_REGISTERS[ idx ] );

            for ( auto block_idx { 0uz }; block_idx < func.blocks.size( ); ++block_idx )
            {
                const auto &params = func.param_indices[ block_idx ];
                for ( auto param_idx { 0uz }; param_idx < params.size( ); ++param_idx )
                    set_hint( params[ param_idx ], ARG_REGISTERS[ param_idx ] );
            }

            const auto peak_live = compute_max_live( ranges );

            register_alloc_t reg_alloc { GENERAL_PURPOSE };
            const auto       result = reg_alloc.run( ranges );

            const std::size_t slots         = peak_live > GEN_REG_COUNT ? peak_live - GEN_REG_COUNT : 0;
            const std::size_t stk_size      = slots * PTR_SIZE;
            const bool        frames_needed = stk_size > 0;

            std::vector< Register > used_callee_regs { };
            for ( const auto &reg : result.assignments | std::views::values )
            {
                if ( !is_callee_register( reg ) )
                    continue;
                if ( std::ranges::find( used_callee_regs, reg ) == used_callee_regs.end( ) )
                    used_callee_regs.push_back( reg );
            }

            const auto prologue = m_asm.pos( );
            m_asm.nop( static_cast< std::uint32_t >( used_callee_regs.size( ) * 3 ) );

            if ( frames_needed )
                m_asm.enter( stk_size );

            std::vector< std::size_t > labels { };
            for ( auto i { 0uz }; i < func.blocks.size( ); ++i )
                labels.push_back( m_asm.label( ) );

            for ( auto block_idx { 0uz }; block_idx < func.blocks.size( ); ++block_idx )
            {
                const auto &[ parameters, instructions ] = func.blocks[ block_idx ];
                m_asm.bind( labels[ block_idx ] );

                const auto &params = block_idx == 0 ? func.args : func.param_indices[ block_idx ];
                for ( auto param_idx { 0uz }; param_idx < params.size( ) && param_idx < ARG_REGISTERS.size( ); ++param_idx )
                {
                    const auto val  = params[ param_idx ];
                    const auto dest = dest_register( result, val );

                    if ( dest != ARG_REGISTERS[ param_idx ] )
                    {
                        m_asm.mov_reg_reg( dest, ARG_REGISTERS[ param_idx ] );
                    }

                    store_if_spilled( result, val, dest );
                }

                for ( auto &inst : instructions )
                {
                    std::visit(
                        [ & ]< typename T0 >( T0 &i )
                        {
                            using T = std::decay_t< T0 >;

                            if constexpr ( std::is_same_v< T, i_const > )
                            {
                                const Register reg = dest_register( result, i.result );
                                m_asm.mov_reg_imm64( reg, i.constant );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_add > )
                                emit_binary( result, i.lhs, i.rhs, i.result, &Assembler::add_reg_reg );
                            else if constexpr ( std::is_same_v< T, i_sub > )
                                emit_binary( result, i.lhs, i.rhs, i.result, &Assembler::sub_reg_reg );
                            else if constexpr ( std::is_same_v< T, i_mul > )
                                emit_binary( result, i.lhs, i.rhs, i.result, &Assembler::imul_reg_reg );
                            else if constexpr ( std::is_same_v< T, i_and > )
                                emit_binary( result, i.lhs, i.rhs, i.result, &Assembler::and_reg_reg );
                            else if constexpr ( std::is_same_v< T, i_or > )
                                emit_binary( result, i.lhs, i.rhs, i.result, &Assembler::or_reg_reg );
                            else if constexpr ( std::is_same_v< T, i_xor > )
                                emit_binary( result, i.lhs, i.rhs, i.result, &Assembler::xor_reg_reg );
                            else if constexpr ( std::is_same_v< T, i_not > )
                                emit_unary( result, i.value, i.result, &Assembler::not_reg );
                            else if constexpr ( std::is_same_v< T, i_neg > )
                                emit_unary( result, i.value, i.result, &Assembler::neg );
                            else if constexpr ( std::is_same_v< T, i_shl> )
                            {
                                const auto lhs = in_register( result, i.lhs, Register::r10 );
                                const auto rhs = in_register( result, i.rhs, Register::r11 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( Register::rcx, rhs );
                                m_asm.mov_reg_reg( reg, lhs );
                                m_asm.shl_reg( reg );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_shr > )
                            {
                                const auto lhs = in_register( result, i.lhs, Register::r10 );
                                const auto rhs = in_register( result, i.rhs, Register::r11 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( Register::rcx, rhs );
                                m_asm.mov_reg_reg( reg, lhs );
                                m_asm.shr_reg( reg );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_shl_imm > )
                            {
                                const auto src = in_register( result, i.lhs, Register::r10 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( reg, src );
                                m_asm.shl_reg_imm( reg, i.imm );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_shr_imm > )
                            {
                                const auto src = in_register( result, i.lhs, Register::r10 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( reg, src );
                                m_asm.shr_reg_imm( reg, i.imm );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_sar_imm > )
                            {
                                const auto src = in_register( result, i.lhs, Register::r10 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( reg, src );
                                m_asm.sar_reg_imm( reg, i.imm );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_div > )
                            {
                                const auto lhs = in_register( result, i.lhs, Register::r10 );
                                const auto rhs = in_register( result, i.rhs, Register::r11 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( Register::rax, lhs );
                                m_asm.mov_reg_reg( Register::r11, rhs );
                                m_asm.cqo( );
                                m_asm.idiv( Register::r11 );
                                m_asm.mov_reg_reg( reg, Register::rax );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_udiv > )
                            {
                                const auto lhs = in_register( result, i.lhs, Register::r10 );
                                const auto rhs = in_register( result, i.rhs, Register::r11 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.mov_reg_reg( Register::rax, lhs );
                                m_asm.mov_reg_reg( Register::r11, rhs );
                                m_asm.xor_rdx(  );
                                m_asm.udiv( Register::r11 );
                                m_asm.mov_reg_reg( reg, Register::rax );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_cmp > )
                            {
                                const auto lhs = in_register( result, i.lhs, Register::r10 );
                                const auto rhs = in_register( result, i.rhs, Register::r11 );
                                const auto reg = dest_register( result, i.result );
                                m_asm.cmp_reg_reg( lhs, rhs );
                                m_asm.set_cc( i.kind, Register::rax );
                                m_asm.movzx_reg_reg8( Register::rax, Register::rax );
                                m_asm.mov_reg_reg( reg, Register::rax );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_call > )
                            {
                                const auto reg = dest_register( result, i.result );
                                bind_call_args( result, i.args );

                                if ( std::holds_alternative< const void * >( i.target.target ) )
                                    m_asm.call( std::get< const void * >( i.target.target ) );
                                else
                                {
                                    const auto &name = std::get< std::string >( i.target.target );
                                    const auto  fn   = mod.get_function< const void * >( name );
                                    if ( !fn )
                                        throw std::runtime_error( "call target function not found: " + name );
                                    m_asm.call( fn );
                                }

                                m_asm.mov_reg_reg( reg, Register::rax );
                                store_if_spilled( result, i.result, reg );
                            }
                            else if constexpr ( std::is_same_v< T, i_brif > )
                            {
                                const auto true_guard  = m_asm.label( );
                                const auto false_guard = m_asm.label( );
                                const auto condition   = in_register( result, i.condition );

                                m_asm.cmp_reg_imm32( condition, 0 );
                                m_asm.jnz( true_guard );

                                m_asm.bind( false_guard );
                                bind_call_args( result, i.false_args );
                                m_asm.jmp( labels[ i.false_blk ] );

                                m_asm.bind( true_guard );
                                bind_call_args( result, i.true_args );
                                m_asm.jmp( labels[ i.true_blk ] );
                            }
                            else if constexpr ( std::is_same_v< T, i_jmp > )
                            {
                                bind_call_args( result, i.args );
                                m_asm.jmp( labels[ i.to_block ] );
                            }
                            else if constexpr ( std::is_same_v< T, i_ret > )
                            {
                                const Register value = in_register( result, i.value );
                                m_asm.mov_reg_reg( Register::rax, value );

                                for ( auto it = used_callee_regs.rbegin( ); it != used_callee_regs.rend( ); ++it )
                                    m_asm.pop( *it );

                                if ( frames_needed )
                                    m_asm.leave( );
                                m_asm.ret( );
                            }
                            else if constexpr ( std::is_same_v< T, i_load > )
                            {
                                const auto base = in_register( result, i.base, Register::rax );
                                const auto dest = dest_register( result, i.result );

                                switch ( i.width )
                                {
                                    case 1 :
                                        i.sign_extended ? m_asm.movsx_reg_mem8( dest, base, i.offset )
                                                        : m_asm.movzx_reg_mem8( dest, base, i.offset );
                                        break;
                                    case 2 :
                                        i.sign_extended ? m_asm.movsx_reg_mem16( dest, base, i.offset )
                                                        : m_asm.movzx_reg_mem16( dest, base, i.offset );
                                        break;
                                    default : m_asm.mov_reg_mem( dest, base, i.offset ); break;
                                }

                                store_if_spilled( result, i.result, dest );
                            }
                            else if constexpr ( std::is_same_v< T, i_store > )
                            {
                                const auto base = in_register( result, i.base, Register::r10 );
                                const auto src  = in_register( result, i.value, Register::r11 );

                                switch ( i.width )
                                {
                                    case 1 : m_asm.mov_mem_reg8( base, i.offset, src ); break;
                                    case 2 : m_asm.mov_mem_reg16( base, i.offset, src ); break;
                                    default : m_asm.mov_mem_reg( base, i.offset, src ); break;
                                }
                            }
                        },
                        inst );
                }
            }

            auto patch_pos = prologue;
            for ( const auto reg : used_callee_regs )
            {
                std::vector< std::byte > bytes { };
                if ( ext( reg ) )
                    bytes = { std::byte { rex( false, Register::rax, reg ) }, 0xFF_b,
                              std::byte { static_cast< std::uint8_t >( 0xC0 | 6 << 3 | enc( reg ) ) } };
                else
                    bytes = { 0xFF_b, std::byte { static_cast< std::uint8_t >( 0xC0 | 6 << 3 | enc( reg ) ) } };

                m_asm.patch_at( patch_pos, bytes );
                patch_pos += bytes.size( );
            }

            mod.functions[ symbol ] = m_asm.entry_at( prologue );
        }

        gen_module_t compile_module( module_t &mod_ir )
        {
            verify_module( mod_ir );

            gen_module_t mod;
            for ( auto &func : mod_ir.functions )
                compile_function( func, mod, func.name );

            auto _ = m_asm.commit< const void * >( );

            return mod;
        }

    private:
        Assembler &m_asm;

        [[nodiscard]] Register in_register( const alloc_result_t &result, const Value value, const Register scratch = Register::r10 ) const
        {
            if ( const auto iter = result.assignments.find( value ); iter != result.assignments.end( ) )
                return iter->second;

            if ( const auto iter = result.spills.find( value ); iter != result.spills.end( ) )
            {
                m_asm.mov_reg_mem( scratch, Register::rbp, iter->second );
                return scratch;
            }

            throw std::runtime_error( "value not found in assignments or spills" );
        }

        static Register dest_register( const alloc_result_t &result, const Value value )
        {
            if ( const auto it = result.assignments.find( value ); it != result.assignments.end( ) )
                return it->second;
            return Register::r10;
        }

        void store_if_spilled( const alloc_result_t &result, const Value value, const Register reg ) const
        {
            if ( const auto it = result.spills.find( value ); it != result.spills.end( ) )
                m_asm.mov_mem_reg( Register::rbp, it->second, reg );
        }

        void bind_call_args( const alloc_result_t &result, const std::vector< Value > &args ) const
        {
            const std::size_t n = std::min( args.size( ), ARG_REGISTERS.size( ) );

            for ( auto a { 0uz }; a < n; ++a )
                m_asm.push( in_register( result, args[ a ] ) );

            for ( auto a { n }; a-- > 0; )
                m_asm.pop( ARG_REGISTERS[ a ] );
        }

        void emit_binary( const alloc_result_t &result, const Value v_lhs, const Value v_rhs, const Value v_result,
                          void ( Assembler::*op )( Register, Register ) ) const
        {
            const auto lhs = in_register( result, v_lhs, Register::r10 );
            const auto rhs = in_register( result, v_rhs, Register::r11 );
            const auto reg = dest_register( result, v_result );
            m_asm.mov_reg_reg( reg, lhs );
            ( m_asm.*op )( reg, rhs );
            store_if_spilled( result, v_result, reg );
        }

        void emit_unary( const alloc_result_t &result, const Value v_operand, const Value v_result,
                         void ( Assembler::*op )( Register ) ) const
        {
            const auto operand = in_register( result, v_operand, Register::r10 );
            const auto reg     = dest_register( result, v_result );
            m_asm.mov_reg_reg( reg, operand );
            ( m_asm.*op )( reg );
            store_if_spilled( result, v_result, reg );
        }
    };
} // namespace amd64::codegen
