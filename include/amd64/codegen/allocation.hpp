#pragma once

#include <amd64/assembler/registers.hpp>
#include <amd64/ir/ir.hpp>

#include <algorithm>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
#include <optional>

namespace amd64::codegen::allocation
{
    using namespace amd64::assembler::registers;
    using namespace amd64::ir;


    ///@brief A value's live range, its usage within an instruction index range which
    ///it must hold a register or stack slot
    struct live_range_t
    {
        Value           value;
        std::size_t     start;
        std::size_t     end;
        std::optional
        < Register >    hint { std::nullopt };
    };

    ///@brief Result of the allocator, a map of where values resulted at.
    struct alloc_result_t
    {
        std::unordered_map< Value, Register >       assignments { };
        std::unordered_map< Value, std::int32_t >   spills { };
    };

    class register_alloc_t
    {
    public:
        explicit register_alloc_t( std::span< const Register > available_registers )
            : m_free( available_registers.begin(  ), available_registers.end( ) )
        { }

        [[nodiscard]] alloc_result_t run( std::vector< live_range_t > ranges )
        {
            std::ranges::sort( ranges, { }, &live_range_t::start );

            alloc_result_t res;

            std::vector< live_range_t > active;
            std::int32_t stack_offset = -8;

            for ( const auto& interval : ranges )
            {
                expire_old_intervals( interval, active, res );

                /* use hint register if available */
                if ( interval.hint.has_value( ) )
                {
                    const auto reg = *interval.hint;
                    if ( const auto iter = std::ranges::find( m_free, reg ); iter != m_free.end( ) )
                    {
                        m_free.erase( iter );
                        res.assignments[ interval.value ] = reg;
                        insert_sorted_by_end( active, interval );
                        continue;
                    }
                }

                /* if there are no more available register, spill to the stack */
                if ( m_free.empty(  ) )
                {
                    spill_at_interval( interval, active, res, stack_offset );
                }
                else
                {
                    const auto reg = m_free.back(  );

                    m_free.pop_back(  );
                    res.assignments[ interval.value ] = reg;

                    insert_sorted_by_end( active, interval );
                }
            }

            return res;
        }
    private:
        std::vector< Register > m_free { };

        static void insert_sorted_by_end( std::vector< live_range_t >& active, const live_range_t& interval )
        {
            const auto pos = std::ranges::upper_bound( active, interval.end, { }, &live_range_t::end );
            active.insert( pos, interval );
        }

        void expire_old_intervals( const live_range_t& current, std::vector< live_range_t >& active, const alloc_result_t & res )
        {
            auto iter = active.begin(  );
            while ( iter != active.end(  ) && iter->end < current.start )
            {
                m_free.push_back( res.assignments.at( iter->value ) );
                iter = active.erase( iter );
            }
        }

        static void  spill_at_interval(
            const live_range_t& current,
            std::vector< live_range_t >& active,
            alloc_result_t& res,
            std::int32_t& stack_offset
        )
        {
            if ( !active.empty( ) && active.back(  ).end > current.end )
            {
                const live_range_t spill_cand = active.back(  );
                const Register reg = res.assignments.at( spill_cand.value );

                res.spills[ spill_cand.value ] = stack_offset;
                stack_offset -= 8;

                res.assignments.erase( spill_cand.value );
                active.pop_back(  );

                res.assignments[ current.value ] = reg;
                insert_sorted_by_end( active, current );
            }
            else
            {
                res.spills[ current.value ] = stack_offset;
                stack_offset -= 8;
            }
        }
    };
}