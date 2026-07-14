#include <print>
#include <cstdint>

#include <amd64/ir/ir.hpp>
#include <amd64/ir/fmt.hpp>

using namespace amd64::ir;

std::int32_t main( )
{
    module_t math_mod;

    /* define an "add" function in the "math" module */
    {
        auto      &add_fn = math_mod.create_function( "add", { type_t::i64, type_t::i64 }, type_t::i64 );

        auto [arg1, arg2] = add_fn.get_args< 2 >( );
        const auto result = add_fn.iadd( arg1, arg2 );
        add_fn.ret( result );
    }

    std::println("{}", to_string( math_mod ) );

    return 0;
}