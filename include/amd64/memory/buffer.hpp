#pragma once

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <sys/mman.h>
#endif

#include <array>
#include <memory>
#include <span>
#include <stdexcept>
#include <cstdint>
#include <cstring>


namespace amd64::mem
{
    /// A literal for casting generic integers to std::byte cleanly.
    constexpr std::byte operator""_b( unsigned long long value ) { return static_cast<std::byte>( value ); }

    /// Custom deleter for both Windows and Linux allocated memory for wrapping in smart pointers
    struct virtual_deleter
    {
        std::size_t size {};
        void operator()( std::byte* ptr ) const noexcept
        {
            if ( !ptr ) return;
#ifdef _WIN32
            VirtualFree( ptr, 0, MEM_RELEASE );
#else
            munmap( ptr, size );
#endif
        }
    };

    using BufferMemory = std::unique_ptr<std::byte[], virtual_deleter>;

    /// Buffer class, write and execute memory during runtime.
    class buffer_t
    {
    public:
        explicit buffer_t( const std::size_t size )
            : m_size( size ),
              m_memory( [&]() -> std::byte*
        {
#ifdef _WIN32
          return static_cast< std::byte* >( VirtualAlloc( nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE ) );
#else
          void* p = mmap( nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
          return p == MAP_FAILED ? nullptr : static_cast< std::byte* >( p );
#endif
        }( ), virtual_deleter{ size } )
        {
            if ( !m_memory ) throw std::bad_alloc( );
        }

        [[nodiscard]] std::byte*    data( ) const noexcept { return m_memory.get(  ); }
        [[nodiscard]] std::size_t   size( ) const noexcept { return m_size;           }
        [[nodiscard]] std::size_t   pos( )  const noexcept { return m_pos;            }
        [[nodiscard]] std::span<const std::byte> bytes() const noexcept
        {
            return { m_memory.get(), m_pos };
        }

        void write_byte( const std::byte byte )
        {
            write_bytes( { &byte, 1 } );
        }

        void write_bytes( const std::span<const std::byte> bytes )
        {
            if ( m_pos + bytes.size( ) > m_size ) throw std::runtime_error( "buffer overflow" );

            std::memcpy( m_memory.get(  ) + m_pos, bytes.data( ), bytes.size( ) );
            m_pos += bytes.size( );
        }

        void write_bytes( const std::initializer_list<std::byte> bytes )
        {
            write_bytes( std::span( bytes.begin(), bytes.size( ) ) );
        }

        template< typename T >
        requires std::integral< T >
        void write_imm( const T value )
        {
            if ( m_pos + sizeof( T ) > m_size ) throw std::runtime_error( "buffer overflow" );

            std::memcpy( m_memory.get(  ) + m_pos, &value, sizeof( T ) );
            m_pos += sizeof( T );
        }

        auto patch_at( const std::size_t pos, const std::span< const std::byte > bytes ) const
        {
            if ( pos + bytes.size(  ) > m_size ) throw std::runtime_error( "patch out of range" );
            std::memcpy( m_memory.get(  ) + pos, bytes.data( ), bytes.size( ) );
        }

        auto skip( const std::size_t amount )
        {
            if ( m_pos + amount > m_size ) throw std::runtime_error( "buffer overflow in skip()" );
            m_pos += amount;
        }

        [[nodiscard]] bool make_exec( ) const
        {
#ifdef _WIN32
            DWORD old{};
            return VirtualProtect( m_memory.get(), m_size, PAGE_EXECUTE_READ, &old );
#else
            return mprotect( m_memory.get(), m_size, PROT_READ | PROT_EXEC ) == 0;
#endif
        }
    private:
        std::size_t  m_size   { };
        std::size_t  m_pos    { };
        BufferMemory m_memory { };
    };
}