#pragma once

#include <amd64/memory/buffer.hpp>
#include <amd64/assembler/registers.hpp>
#include <amd64/core/encode.hpp>

#include <bit>
#include <optional>
#include <vector>

namespace amd64::assembler
{
    using namespace amd64::assembler::registers;
    using namespace amd64::core::encode;
    using namespace amd64::mem;

    struct label_t
    {
        std::optional< std::size_t >    target;
        std::vector< std::size_t >      fixups;
    };

    enum class set_cc_kind { eq, ne, lt, le, gt, ge };

    class Assembler
    {
    public:
        explicit Assembler( const std::size_t size )
            : m_size( size ), m_buffer( size )
        { }

        template <typename F>
        [[nodiscard]] F commit( ) const
        {
            if ( !m_buffer.make_exec( ) ) throw std::runtime_error( "make_exec() failed" );
            return reinterpret_cast<F>( m_buffer.data( ) );
        }

        [[nodiscard]] std::span<const std::byte> bytes() const noexcept { return m_buffer.bytes(); }

        std::size_t label( )
        {
            m_labels.emplace_back( label_t { .target = std::nullopt, .fixups = { } } );
            return m_labels.size( ) - 1;
        }

        auto bind( const std::size_t label )
        {
            m_labels[ label ].target = m_buffer.pos(  );

            for ( const auto& fix : m_labels[ label ].fixups )
            {
                const auto offset = static_cast<std::int32_t>(
                    static_cast< std::int64_t >( m_labels[ label ].target.value(  ) ) - ( static_cast< std::int64_t >( fix ) + 4 )
                );

                static_assert( std::endian::native == std::endian::little, "offset patch assumes little endiann" );

                std::memcpy( m_buffer.data(  ) + fix, &offset, sizeof( offset ) );
            }
        }

        auto nop( const std::uint32_t count )
        {
            for ( auto i { 0uz }; i < count; ++i ) m_buffer.write_byte( 0x90_b );
        }

        auto pop( const Register reg )
        {
            if ( ext( reg ) ) m_buffer.write_byte( std::byte{ rex( false, Register::rax, reg ) } );
            m_buffer.write_bytes( { 0x8F_b, std::byte{ static_cast<std::uint8_t>( 0xC0 | enc( reg ) ) } } );
        }

        auto push( const Register reg )
        {
            if ( ext( reg ) ) m_buffer.write_byte( std::byte{ rex( false, Register::rax, reg ) } );
            m_buffer.write_bytes( { 0xFF_b, std::byte{ static_cast<std::uint8_t>( 0xC0 | ( 6 << 3 ) | enc( reg ) ) } } );
        }

        auto ret( ) { m_buffer.write_byte( 0xC3_b ); }

        auto cqo( ) { m_buffer.write_bytes( { std::byte{ REX_NO_OPS }, 0x99_b } ); }

        auto mov_reg_imm64( const Register reg, const std::int64_t imm )
        {
            m_buffer.write_bytes( {
                static_cast< std::byte >( rex( true, Register::rax, reg ) ),
                static_cast< std::byte >( 0xB8 + enc( reg ) )
            } );

            m_buffer.write_imm( imm );
        }

        auto imul_reg_reg( const Register dst, const Register src )
        {
            m_buffer.write_bytes( { std::byte{ rex( true, dst, src ) }, 0x0F_b, 0xAF_b, std::byte{ mod_rm( dst, src ) } } );
        }

        auto movzx_reg_reg8( const Register dst, const Register src )
        {
            m_buffer.write_bytes( { std::byte{ rex( true, dst, src ) }, 0x0F_b, 0xB6_b, std::byte{ mod_rm( dst, src ) } } );
        }

        auto cmp_reg_reg( const Register dst, const Register src )
        {
            m_buffer.write_bytes( { std::byte{ rex( true, dst, src ) }, 0x3B_b, std::byte{ mod_rm( dst, src ) } } );
        }

        auto mov_reg_reg( const Register dst, const Register src ) { emit_rr( 0x89_b, src, dst ); }
        auto add_reg_reg( const Register dst, const Register src ) { emit_rr( 0x01_b, src, dst ); }
        auto sub_reg_reg( const Register dst, const Register src ) { emit_rr( 0x29_b, src, dst ); }
        auto and_reg_reg( const Register dst, const Register src ) { emit_rr( 0x21_b, src, dst ); }
        auto or_reg_reg ( const Register dst, const Register src ) { emit_rr( 0x09_b, src, dst ); }
        auto xor_reg_reg( const Register dst, const Register src ) { emit_rr( 0x31_b, src, dst ); }

        auto add_reg_imm32( const Register dst, const std::int32_t imm ) { emit_digit_rm_imm32( 0, dst, imm ); }
        auto sub_reg_imm32( const Register dst, const std::int32_t imm ) { emit_digit_rm_imm32( 5, dst, imm ); }
        auto cmp_reg_imm32( const Register dst, const std::int32_t imm ) { emit_digit_rm_imm32( 7, dst, imm ); }

        auto dec_reg( const Register reg ) { emit_digit_rm( 0xFF_b, 1, reg ); }
        auto not_reg( const Register reg ) { emit_digit_rm( 0xF7_b, 2, reg ); }
        auto neg    ( const Register reg ) { emit_digit_rm( 0xF7_b, 3, reg ); }
        auto shl_reg( const Register reg ) { emit_digit_rm( 0xD3_b, 4, reg ); }
        auto shr_reg( const Register reg ) { emit_digit_rm( 0xD3_b, 5, reg ); }
        auto idiv   ( const Register reg ) { emit_digit_rm( 0xF7_b, 7, reg ); }

        auto jmp( const std::size_t label ) { emit_jcc( { 0xE9_b }, label ); }
        auto jz ( const std::size_t label ) { emit_jcc( { 0x0F_b, 0x84_b }, label ); }
        auto jnz( const std::size_t label ) { emit_jcc( { 0x0F_b, 0x85_b }, label ); }
        auto jl ( const std::size_t label ) { emit_jcc( { 0x0F_b, 0x8C_b }, label ); }
        auto jge( const std::size_t label ) { emit_jcc( { 0x0F_b, 0x8D_b }, label ); }

        auto call( auto fptr )
        {
            sub_reg_imm32( Register::rsp, 40 );
            mov_reg_imm64( Register::rax, static_cast< std::int64_t >( reinterpret_cast< std::uintptr_t >( fptr ) ) );
            m_buffer.write_bytes( { 0xFF_b, 0xD0_b } );
            add_reg_imm32( Register::rsp, 40 );
        }

        auto mov_reg_mem( const Register src, const Register base, const std::int32_t offset )
        {
            emit_mem_disp32( 0x8B_b, src, base, offset );
        }

        auto mov_mem_reg( const Register base, const std::int32_t offset, const Register src)
        {
            emit_mem_disp32( 0x89_b, src, base, offset );
        }

        auto mov_mem_imm32( const Register base, const std::int32_t offset, const std::int32_t imm )
        {
            emit_mem_disp32( 0xC7_b, Register::rax, base, offset );
            m_buffer.write_imm( imm );
        }

        auto movsd_reg_reg( const XmmRegister dst, const XmmRegister src )
        {
            m_buffer.write_byte( 0xF2_b );
            if ( ext(dst) || ext(src) )
                m_buffer.write_byte( std::byte{ rex( true, src, dst ) } );
            m_buffer.write_bytes( { 0x0F_b, 0x10_b, std::byte{ mod_rm( src, dst ) } } );
        }

        auto enter( const std::size_t size )
        {
            const std::size_t aligned = ( size + 15 ) & ~std::size_t{ 15 };

            push( Register::rbp );
            mov_reg_reg( Register::rbp, Register::rsp );
            /// TODO: add support for linux, system v does not need the 32 windows shadow space
            sub_reg_imm32( Register::rsp, static_cast< std::int32_t >( aligned + 32 ) );
        }

        auto leave( )
        {
            mov_reg_reg( Register::rsp, Register::rbp );
            pop( Register::rbp );
        }

        auto set_cc( const set_cc_kind kind, const Register reg )
        {
            static constexpr std::array< std::uint8_t const, 6 > opcodes { 0x94, 0x95, 0x9C, 0x9E, 0x9F, 0x9D };
            const auto opcode = opcodes[ static_cast< std::size_t >( kind ) ];

            m_buffer.write_bytes( { 0x0F_b, std::byte{ opcode }, std::byte{ static_cast< std::uint8_t >( 0xC0 | enc( reg ) ) } } );
        }

    private:
        std::size_t m_size { };
        buffer_t m_buffer;
        std::vector< label_t > m_labels { };

        ///@brief Collapsed the rex+opcode+modrm pattern into a single function
        void emit_rr( std::byte opcode, const Register src, const Register dst )
        {
            m_buffer.write_bytes({ std::byte{ rex( true, src, dst ) }, opcode, std::byte{ mod_rm( src, dst ) } });
        }

        ///@brief Collapsed the rex+opcode+digit+modrm pattern into a single function
        void emit_digit_rm_imm32( const std::uint8_t digit, const Register reg, const std::int32_t imm )
        {
            m_buffer.write_bytes( { std::byte{ rex( true, Register::rax, reg ) }, 0x81_b,
                std::byte{ static_cast< std::uint8_t >( 0xC0 | digit << 3 | enc( reg ) ) }} );
            m_buffer.write_imm( imm );
        }

        ///@brief Collapsed the rex+opcode+digit+modrm pattern into a single function
        void emit_digit_rm( std::byte opcode, const std::uint8_t digit, const Register reg )
        {
            m_buffer.write_bytes( { std::byte{ rex( true, Register::rax, reg ) }, opcode,
                std::byte{ static_cast< std::uint8_t >( 0xC0 | digit << 3 | enc( reg ) ) }} );
        }

        ///@brief Collapsed the jcc with fixup pattern into a single function
        void emit_jcc( const std::initializer_list< std::byte > opcode, const std::size_t label )
        {
            m_buffer.write_bytes( opcode );

            const std::size_t fix_pos = m_buffer.pos( );
            if ( m_labels[ label ].target.has_value( ) )
            {
                const auto offset = static_cast<std::int32_t>(
                    static_cast<std::int64_t>( m_labels[label].target.value() ) - ( static_cast<std::int64_t>( fix_pos ) + 4 )
                );

                std::memcpy( m_buffer.data(  ) + fix_pos, &offset, sizeof( offset ) );
                m_buffer.skip( 4 );
            }
            else
            {
                m_buffer.write_imm( std::int32_t{ 0 } );
                m_labels[ label ].fixups.push_back( fix_pos );
            }
        }

        ///@brief Collapsed the rex+opcode+modrm(direct,disp32)+optional-sib pattern into a function
        void emit_mem_disp32( const std::byte opcode, const Register reg_field, const Register base, const std::int32_t offset )
        {
            m_buffer.write_bytes( { std::byte{ rex( true, reg_field, base ) }, opcode,
                std::byte{ static_cast<std::uint8_t>( 0x80 | enc(reg_field) << 3 | enc(base) ) } } );

            /// sib
            if ( base == Register::rsp ) m_buffer.write_byte( 0x24_b );

            m_buffer.write_imm( offset );
        }
    };
}