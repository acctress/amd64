#include <amd64/assembler/assembler.hpp>
#include <windows.h>
#include <print>

std::int32_t main( )
{
    using namespace amd64::assembler;
    using namespace amd64::assembler::registers;

    static constexpr char title[] = "amd64";
    static constexpr char text[]  = "helo world";

    try
    {
        Assembler a{ 256 };

        a.mov_reg_imm64( Register::rcx, 0 );
        a.mov_reg_imm64( Register::rdx, reinterpret_cast< std::int64_t >( +text ) );
        a.mov_reg_imm64( Register::r8,  reinterpret_cast< std::int64_t >( +title ) );
        a.mov_reg_imm64( Register::r9,  MB_OK );
        a.call( &MessageBoxA );
        a.ret( );

        const auto fn = a.commit<int(*)()>( );
        fn( );
    }
    catch ( const std::exception& e )
    {
        std::println( "amd64[error]: {}", e.what() );
        return 1;
    }

    return 0;
}