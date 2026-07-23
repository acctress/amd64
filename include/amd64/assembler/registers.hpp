#pragma once
#include <cstdint>

namespace amd64::assembler::registers
{
    enum class Register : std::uint8_t
    {
        rax, rcx, rdx, rbx,
        rsp, rbp, rsi, rdi,
        r8,  r9,  r10, r11,
        r12, r13, r14, r15
    };

    enum class XmmRegister : std::uint8_t
    {
        xmm0,  xmm1,  xmm2,  xmm3,
        xmm4,  xmm5,  xmm6,  xmm7,
        xmm8,  xmm9,  xmm10, xmm11,
        xmm12, xmm13, xmm14, xmm15,
    };

    constexpr std::uint8_t enc( Register reg ) noexcept
    {
        return static_cast< std::uint8_t >( reg ) & 0b111;
    }

    constexpr bool ext( Register reg ) noexcept
    {
        return static_cast< std::uint8_t >( reg ) >= static_cast< std::uint8_t >( Register::r8 );
    }

    constexpr std::uint8_t enc( XmmRegister reg ) noexcept
    {
        return static_cast< std::uint8_t >( reg ) & 0b111;
    }

    constexpr bool ext( XmmRegister reg ) noexcept
    {
        return static_cast< std::uint8_t >( reg ) >= static_cast< std::uint8_t >( XmmRegister::xmm8 );
    }
}