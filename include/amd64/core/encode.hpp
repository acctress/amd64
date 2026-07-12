#pragma once

#include <amd64/assembler/registers.hpp>

namespace amd64::core::encode
{
    using namespace amd64::assembler::registers;

    template < typename T >
    concept x86reg = requires( T reg )
    {
        { enc( reg ) } -> std::same_as<std::uint8_t>;
        { ext( reg ) } -> std::same_as<bool>;
    };

    inline constexpr std::uint8_t REX_NO_OPS { 0x48 };

    template <x86reg R1, x86reg R2>
    constexpr std::uint8_t rex( const bool w, R1 reg1, R2 reg2 ) noexcept
    {
        std::uint8_t byte { 0x40 };
        byte |= static_cast<std::uint8_t>( w ) << 3;
        byte |= static_cast<std::uint8_t>( ext( reg1 ) ) << 2;
        byte |= static_cast<std::uint8_t>( ext( reg2 ) );
        return byte;
    }

    template <x86reg R1, x86reg R2>
    constexpr std::uint8_t mod_rm( R1 reg, R2 rm ) noexcept
    {
        std::uint8_t byte { 0xC0 };
        byte |= enc( reg ) << 3;
        byte |= enc( rm );
        return byte;
    }
}