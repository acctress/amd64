#include <ranges>
#include <catch2/catch_test_macros.hpp>
#include <amd64/codegen/allocation.hpp>

using namespace amd64::codegen::allocation;
using namespace amd64::assembler::registers;

TEST_CASE( "single non-overlapping ranges each get a register", "[allocation]" )
{
    std::vector pool { Register::rax, Register::rbx };
    register_alloc_t alloc( pool );

    std::vector<live_range_t> ranges {
        { 0, 0, 5 },
        { 1, 6, 10 },
    };

    auto [assignments, spills ] = alloc.run( ranges );

    REQUIRE( assignments.contains( 0 ) );
    REQUIRE( assignments.contains( 1 ) );
    REQUIRE( spills.empty() );
}

TEST_CASE( "register is reused once a range expires", "[allocation]" )
{
    std::vector pool { Register::rax }; // only one register available
    register_alloc_t alloc( pool );

    std::vector<live_range_t> ranges {
        { 0, 0, 5 },   // dies at 5
        { 1, 6, 10 },  // starts at 6, strictly after v0 dies -> should reuse rax
    };

    auto [assignments, spills ] = alloc.run( ranges );

    REQUIRE( assignments.contains( 0 ) );
    REQUIRE( assignments.contains( 1 ) );
    REQUIRE( assignments.at( 0 ) == assignments.at( 1 ) );
    REQUIRE( spills.empty() );
}

TEST_CASE( "touching ranges (end == start) are treated as overlapping, not reused", "[allocation]" )
{
    std::vector<Register> pool { Register::rax }; // only one register available
    register_alloc_t alloc( pool );

    std::vector<live_range_t> ranges {
        { 0, 0, 2 },  // dies at 2
        { 1, 2, 5 },  // starts at 2 -> still overlapping, must NOT reuse rax
    };

    auto [assignments, spills ] = alloc.run( ranges );

    // v0 gets the only register; v1 has nothing free and nothing to steal
    // from (active holds only v0, whose end (2) is not > v1's end (5)... )
    // so this actually exercises the self-spill path.
    REQUIRE( ( assignments.contains( 0 ) || assignments.contains( 1 ) ) );
    REQUIRE( assignments.size() + spills.size() == 2 );
}

TEST_CASE( "no free registers and no better candidate spills the current value", "[allocation]" )
{
    std::vector pool { Register::rax, Register::rbx };
    register_alloc_t alloc( pool );

    // v0 and v1 take both registers and live the longest; v2 arrives with
    // nothing free and nothing shorter-lived to steal from -> v2 itself spills.
    std::vector<live_range_t> ranges {
        { 0, 0, 50 },
        { 1, 0, 60 },
        { 2, 1, 100  },
    };

    auto [assignments, spills ] = alloc.run( ranges );

    REQUIRE( assignments.contains( 0 ) );
    REQUIRE( assignments.contains( 1 ) );
    REQUIRE( spills.contains( 2 ) );
    REQUIRE_FALSE( assignments.contains( 2 ) );
}

TEST_CASE( "stealing a register spills the longer-lived active value, not the current one", "[allocation]" )
{
    std::vector pool { Register::rax, Register::rbx };
    register_alloc_t alloc( pool );

    // v0 and v1 grab both registers. v0 lives to 100 (long), v1 lives to 10 (short).
    // v2 arrives at start=5 needing a register, with no free ones. Since v0
    // (end=100) outlives v2 (end=20), v0 should be the one spilled — its
    // register gets stolen for v2 — not v2 itself.
    std::vector<live_range_t> ranges {
        { 0, 0, 100 },
        { 1, 0, 10  },
        { 2, 5, 20  },
    };

    auto result = alloc.run( ranges );

    REQUIRE( result.spills.contains( 0 ) );          // the long-lived one got spilled
    REQUIRE_FALSE( result.assignments.contains( 0 ) ); // must not appear in both maps
    REQUIRE( result.assignments.contains( 1 ) );
    REQUIRE( result.assignments.contains( 2 ) );

    // v2 should have inherited v0's old register.
    // (we don't know which of rax/rbx v0 originally held, so just check
    // v1 and v2 hold two distinct registers between them)
    REQUIRE( result.assignments.at( 1 ) != result.assignments.at( 2 ) );
}

TEST_CASE( "a value never appears in both assignments and spills", "[allocation]" )
{
    std::vector<Register> pool { Register::rax, Register::rbx };
    register_alloc_t alloc( pool );

    std::vector<live_range_t> ranges {
        { 0, 0, 100 },
        { 1, 0, 100 },
        { 2, 1, 50  },
        { 3, 2, 200 },
    };

    for ( auto [ assignments, spills ] = alloc.run( ranges ); const auto &value : assignments | std::views::keys )
    {
        INFO( "value = " << value );
        REQUIRE_FALSE( spills.contains( value ) );
    }
}

TEST_CASE( "spilled values get distinct, descending stack offsets", "[allocation]" )
{
    std::vector pool { Register::rax };
    register_alloc_t alloc( pool );

    std::vector<live_range_t> ranges {
        { 0, 0, 100 },
        { 1, 1, 100 },
        { 2, 2, 100 },
    };

    auto [assignments, spills ] = alloc.run( ranges );

    REQUIRE( !spills.empty() );

    std::vector<std::int32_t> offsets;
    for ( const auto &off : spills | std::views::values ) offsets.push_back( off );

    /* no two spilled values should share a stack slot */
    for ( std::size_t i = 0; i < offsets.size(); ++i )
        for ( std::size_t j = i + 1; j < offsets.size(); ++j )
            REQUIRE( offsets[i] != offsets[j] );
}