#include <amd64/ir/parser.hpp>
#include <amd64/codegen/codegen.hpp>
#include <amd64/assembler/assembler.hpp>

#include <fstream>
#include <sstream>
#include <print>

using namespace amd64::ir;
using namespace amd64::codegen;
using namespace amd64::assembler;

static std::string read_file( const std::string& path )
{
    std::ifstream in( path );
    if ( !in ) throw std::runtime_error( "could not open file: " + path );

    std::ostringstream ss;
    ss << in.rdbuf( );
    return ss.str( );
}

int main( int argc, char** argv )
{
    const std::string path = argc > 1 ? argv[1] : "example.ir";

    try
    {
        const auto source = read_file( path );

        parser::parser_t parser( source );
        auto mod = parser.parse_module( );

        Assembler azm { 4096 };
        code_gen_t codegen( azm );
        auto gen = codegen.compile_module( mod );

        auto combined = gen.get_function<std::int64_t(*)( std::int64_t, std::int64_t )>( "combined" );
        if ( !combined ) throw std::runtime_error( "'combined' not found in module" );

        std::println( "combined(3, 7)  = {}", combined( 3, 7 ) );
        std::println( "combined(10, 2) = {}", combined( 10, 2 ) );
        std::println( "combined(-4, 9) = {}", combined( -4, 9 ) );
    }
    catch ( const parser::parse_error& e )
    {
        std::println( "parse error: {}", e.what( ) );
        return 1;
    }
    catch ( const std::exception& e )
    {
        std::println( "error: {}", e.what( ) );
        return 1;
    }

    return 0;
}