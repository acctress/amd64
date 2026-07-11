#include <amd64/memory/buffer.hpp>
#include <print>

std::int32_t main( )
{
    using namespace amd64::mem;


    try
    {
        buffer_t buffer { 1024 };

        // mov eax, 66
        // ret

        buffer.write_byte( 0xB8_b );
        buffer.write_imm32( 66 );
        buffer.write_byte( 0xC3_b );

        if ( !buffer.make_exec(  ) )
        {
            std::println( "amd64[error]: make_exec() failed" );
            return 1;
        }

        const auto fn = reinterpret_cast<int(*)()>( buffer.data(  ) );
        int res = fn( );

        std::println( "result: {}", res );
    } catch ( std::exception& e )
    {
        std::println( "amd64[error]: {}", e.what() );
        return 1;
    }

    return 0;
}