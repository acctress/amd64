#pragma once

#include <amd64/ir/ir.hpp>
#include <amd64/ir/parser.hpp>

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace amd64::ir::parser
{
    enum class token_type_t : std::uint8_t
    {
        kw_fn,
        kw_ret,
        kw_jmp,
        kw_brif,
        kw_call,
        kw_native,
        kw_iconst,
        kw_iadd,
        kw_isub,
        kw_imul,
        kw_idiv,
        kw_icmp,
        kw_iand,
        kw_ior,
        kw_ixor,
        kw_inot,
        kw_ishl,
        kw_ishr,
        kw_neg,
        kw_i64,
        kw_i32,
        kw_bool,
        kw_ptr,
        kw_bb,
        identifier,
        integer,
        percent,
        at,
        dot,
        comma,
        colon,
        arrow,
        lparen,
        rparen,
        lbrace,
        rbrace,
        equals,
        eof,
    };

    struct source_loc_t
    {
        std::size_t line;
        std::size_t column;
    };

    struct token_t
    {
        token_type_t                              type { };
        std::variant< std::string, std::int64_t > value;
        std::size_t                               pos { };
    };

    inline const std::unordered_map< std::string, token_type_t > keywords {
        {     "fn",     token_type_t::kw_fn },
        {    "ret",    token_type_t::kw_ret },
        {    "jmp",    token_type_t::kw_jmp },
        {   "brif",   token_type_t::kw_brif },
        {   "call",   token_type_t::kw_call },
        { "native", token_type_t::kw_native },
        { "iconst", token_type_t::kw_iconst },
        {   "iadd",   token_type_t::kw_iadd },
        {   "isub",   token_type_t::kw_isub },
        {   "imul",   token_type_t::kw_imul },
        {   "idiv",   token_type_t::kw_idiv },
        {   "icmp",   token_type_t::kw_icmp },
        {    "i64",    token_type_t::kw_i64 },
        {    "i32",    token_type_t::kw_i32 },
        {   "bool",   token_type_t::kw_bool },
        {    "ptr",    token_type_t::kw_ptr },
        {    "iand",    token_type_t::kw_iand },
        {    "ior",    token_type_t::kw_ior },
        {    "ixor",    token_type_t::kw_ixor },
        {    "inot",    token_type_t::kw_inot },
        {    "ishl",    token_type_t::kw_ishl },
        {    "ishr",    token_type_t::kw_ishr },
        {    "ineg",    token_type_t::kw_neg },
    };

    class lexer_t
    {
    public:
        explicit lexer_t( const std::string_view source ) : m_source( source ) { }

        token_t next( )
        {
            skip_ws( );
            if ( m_pos >= m_source.size( ) )
                return { token_type_t::eof, std::string( "" ), m_pos };

            const auto start = m_pos;
            const char c     = m_source[ m_pos ];

            if ( c == '%' )
            {
                ++m_pos;
                return process_int( token_type_t::percent, start );
            }
            if ( c == '@' )
            {
                ++m_pos;
                return { token_type_t::at, std::string( "@" ), start };
            }
            if ( c == '.' )
            {
                ++m_pos;
                return { token_type_t::dot, std::string( "." ), start };
            }
            if ( c == ',' )
            {
                ++m_pos;
                return { token_type_t::comma, std::string( "," ), start };
            }
            if ( c == ':' )
            {
                ++m_pos;
                return { token_type_t::colon, std::string( ":" ), start };
            }
            if ( c == '(' )
            {
                ++m_pos;
                return { token_type_t::lparen, std::string( "(" ), start };
            }
            if ( c == ')' )
            {
                ++m_pos;
                return { token_type_t::rparen, std::string( ")" ), start };
            }
            if ( c == '{' )
            {
                ++m_pos;
                return { token_type_t::lbrace, std::string( "{" ), start };
            }
            if ( c == '}' )
            {
                ++m_pos;
                return { token_type_t::rbrace, std::string( "}" ), start };
            }
            if ( c == '=' )
            {
                ++m_pos;
                return { token_type_t::equals, std::string( "=" ), start };
            }

            if ( c == '-' && start + 1 < m_source.size( ) && m_source[ start + 1 ] == '>' )
            {
                m_pos += 2;
                return { token_type_t::arrow, std::string( "->" ), start };
            }

            if ( c == '-' || is_digit( c ) )
                return process_int( token_type_t::integer, start );
            if ( is_ident( c ) )
                return process_ident( start );

            throw std::runtime_error( "unexpected character in source at position " + std::to_string( start ) );
        }

    private:
        std::string_view m_source;
        std::size_t      m_pos { };

        static bool is_digit( const char c ) noexcept
        {
            return c >= '0' && c <= '9';
        }

        static bool is_ident( const char c ) noexcept
        {
            return std::isalpha( static_cast< unsigned char >( c ) ) || c == '_';
        }

        static bool is_any( const char c ) noexcept
        {
            return std::isalnum( static_cast< unsigned char >( c ) ) || c == '_';
        }

        void skip_ws( )
        {
            while ( m_pos < m_source.size( ) && std::isspace( static_cast< unsigned char >( m_source[ m_pos ] ) ) )
                ++m_pos;
        }

        token_t process_int( const token_type_t typ, const std::size_t start )
        {
            const auto scan_start = m_pos;

            if ( m_pos < m_source.size( ) && m_source[ m_pos ] == '-' )
                ++m_pos;

            const bool is_hex = m_pos + 1 < m_source.size( ) && m_source[ m_pos ] == '0'
                             && ( m_source[ m_pos + 1 ] == 'x' || m_source[ m_pos + 1 ] == 'X' );

            if ( is_hex )
            {
                m_pos += 2;
                while ( m_pos < m_source.size( ) && std::isxdigit( static_cast< unsigned char >( m_source[ m_pos ] ) ) )
                    ++m_pos;
            }
            else
            {
                while ( m_pos < m_source.size( ) && is_digit( m_source[ m_pos ] ) )
                    ++m_pos;
            }

            const std::string text( m_source.substr( scan_start, m_pos - scan_start ) );

            std::int64_t value { };
            const auto   base_start = is_hex ? scan_start + ( text[ 0 ] == '-' ? 3 : 2 ) : scan_start;
            const auto [ ptr, ec ]  = std::from_chars( m_source.data( ) + base_start, m_source.data( ) + m_pos, value, is_hex ? 16 : 10 );

            if ( ec != std::errc( ) || ptr != m_source.data( ) + m_pos )
                throw std::runtime_error( "malformed integer literal at position " + std::to_string( start ) );

            return { typ, value, start };
        }

        token_t process_ident( const std::size_t start )
        {
            while ( m_pos < m_source.size( ) && is_any( m_source[ m_pos ] ) )
                ++m_pos;
            const std::string value( m_source.substr( start, m_pos - start ) );

            if ( const auto iter = keywords.find( value ); iter != keywords.end( ) )
                return { iter->second, value, start };

            if ( value.starts_with( "bb" ) && value.size( ) > 2
                 && std::all_of( value.data( ) + 2, value.data( ) + value.size( ), is_digit ) )
                return { token_type_t::kw_bb, ( std::stoll( value.substr( 2 ) ) ), start };

            return { token_type_t::identifier, value, start };
        }
    };

    class parse_error : public std::runtime_error
    {
    public:
        explicit parse_error( const std::string &msg ) : std::runtime_error( msg ) { }
    };

    class parser_t
    {
    public:
        explicit parser_t( std::string_view source ) : m_lexer( source ), m_current( m_lexer.next( ) ) { }

        module_t parse_module( )
        {
            module_t mod;
            while ( m_current.type != token_type_t::eof )
                parse_function( mod );
            return mod;
        }

    private:
        lexer_t                                   m_lexer;
        token_t                                   m_current;
        std::unordered_map< std::int64_t, Value > m_ids;

        void advance( )
        {
            m_current = m_lexer.next( );
        }

        token_t expect( const token_type_t t, const char *what )
        {
            if ( m_current.type != t )
                throw parse_error( std::string( "expected " ) + what + " at position " + std::to_string( m_current.pos ) );
            const token_t tok = m_current;
            advance( );
            return tok;
        }

        std::int64_t expect_percent( )
        {
            const auto tok = expect( token_type_t::percent, "%N" );
            return std::get< std::int64_t >( tok.value );
        }

        void check_id( const std::int64_t parsed_id, const Value real )
        {
            if ( static_cast< Value >( parsed_id ) != real )
                throw parse_error( "id mismatch: text says %" + std::to_string( parsed_id ) + " but builder produced %"
                                   + std::to_string( real ) );
            m_ids[ parsed_id ] = real;
        }

        Value resolve( const std::int64_t parsed_id )
        {
            const auto it = m_ids.find( parsed_id );
            if ( it == m_ids.end( ) )
                throw parse_error( "reference to undefined %" + std::to_string( parsed_id ) );
            return it->second;
        }

        type_t parse_type( )
        {
            switch ( m_current.type )
            {
                case token_type_t::kw_i64 : advance( ); return type_t::i64;
                case token_type_t::kw_i32 : advance( ); return type_t::i32;
                case token_type_t::kw_bool : advance( ); return type_t::boolean;
                case token_type_t::kw_ptr : advance( ); return type_t::pointer;
                default : throw parse_error( "expected a type at position " + std::to_string( m_current.pos ) );
            }
        }

        void parse_function( module_t &mod )
        {
            m_ids.clear( );

            expect( token_type_t::kw_fn, "'fn'" );
            expect( token_type_t::at, "'@'" );
            const auto name = std::get< std::string >( expect( token_type_t::identifier, "function name" ).value );

            expect( token_type_t::lparen, "'('" );

            std::vector< type_t >       param_types;
            std::vector< std::int64_t > param_ids;

            while ( m_current.type != token_type_t::rparen )
            {
                if ( !param_types.empty( ) )
                    expect( token_type_t::comma, "','" );
                param_types.push_back( parse_type( ) );
                param_ids.push_back( expect_percent( ) );
            }
            expect( token_type_t::rparen, "')'" );

            expect( token_type_t::arrow, "'->'" );
            const auto ret_type = parse_type( );

            auto &fn = mod.create_function( name, param_types, ret_type );

            if ( param_ids.size( ) != fn.args.size( ) )
                throw parse_error( "function declares " + std::to_string( param_ids.size( ) ) + " parameter(s) but has "
                                   + std::to_string( fn.args.size( ) ) );

            for ( std::size_t i = 0; i < fn.args.size( ); ++i )
                check_id( param_ids[ i ], fn.args[ i ] );

            expect( token_type_t::lbrace, "'{'" );

            bool first_block = true;
            while ( m_current.type != token_type_t::rbrace )
                parse_block( fn, first_block );

            expect( token_type_t::rbrace, "'}'" );
        }

        void parse_block( function_t &fn, bool &first_block )
        {
            const auto bb_tok       = expect( token_type_t::kw_bb, "'bbN'" );
            const auto declared_idx = std::get< std::int64_t >( bb_tok.value );

            expect( token_type_t::lparen, "'('" );

            std::vector< type_t >       param_types;
            std::vector< std::int64_t > param_ids;

            while ( m_current.type != token_type_t::rparen )
            {
                if ( !param_types.empty( ) )
                    expect( token_type_t::comma, "','" );
                param_types.push_back( parse_type( ) );
                param_ids.push_back( expect_percent( ) );
            }
            expect( token_type_t::rparen, "')'" );
            expect( token_type_t::colon, "':'" );

            const std::size_t block_idx = first_block ? 0 : fn.blocks.size( );
            first_block                 = false;

            if ( static_cast< std::size_t >( declared_idx ) != block_idx )
                throw parse_error( "block index mismatch: text says bb" + std::to_string( declared_idx ) + " but this is block "
                                   + std::to_string( block_idx ) );

            if ( block_idx != 0 )
                fn.create_block( param_types );

            if ( param_ids.size( ) != fn.param_indices[ block_idx ].size( ) )
                throw parse_error( "block " + std::to_string( block_idx ) + " declares " + std::to_string( param_ids.size( ) )
                                   + " parameter(s) but has " + std::to_string( fn.param_indices[ block_idx ].size( ) ) );

            for ( std::size_t i = 0; i < param_ids.size( ); ++i )
                check_id( param_ids[ i ], fn.param_indices[ block_idx ][ i ] );

            while ( m_current.type != token_type_t::kw_bb && m_current.type != token_type_t::rbrace )
                parse_instruction( fn );
        }

        std::vector< Value > parse_arg_list( )
        {
            std::vector< Value > args;
            expect( token_type_t::lparen, "'('" );
            while ( m_current.type != token_type_t::rparen )
            {
                if ( !args.empty( ) )
                    expect( token_type_t::comma, "','" );
                args.push_back( resolve( expect_percent( ) ) );
            }
            expect( token_type_t::rparen, "')'" );
            return args;
        }

        void parse_instruction( function_t &fn )
        {
            if ( m_current.type == token_type_t::kw_ret )
            {
                advance( );
                fn.ret( resolve( expect_percent( ) ) );
                return;
            }

            if ( m_current.type == token_type_t::kw_jmp )
            {
                advance( );
                const auto target = std::get< std::int64_t >( expect( token_type_t::kw_bb, "'bbN'" ).value );
                const auto args   = parse_arg_list( );
                fn.jmp( static_cast< std::size_t >( target ), args );
                return;
            }

            if ( m_current.type == token_type_t::kw_brif )
            {
                advance( );
                const auto cond = resolve( expect_percent( ) );
                expect( token_type_t::comma, "','" );
                const auto true_blk  = std::get< std::int64_t >( expect( token_type_t::kw_bb, "'bbN'" ).value );
                const auto true_args = parse_arg_list( );
                expect( token_type_t::comma, "','" );
                const auto false_blk  = std::get< std::int64_t >( expect( token_type_t::kw_bb, "'bbN'" ).value );
                const auto false_args = parse_arg_list( );
                fn.brif( cond, static_cast< std::size_t >( true_blk ), static_cast< std::size_t >( false_blk ), true_args, false_args );
                return;
            }

            const auto result_id = expect_percent( );
            expect( token_type_t::equals, "'='" );

            switch ( m_current.type )
            {
                case token_type_t::kw_iconst :
                {
                    advance( );
                    const auto imm = expect( token_type_t::integer, "integer literal" );
                    check_id( result_id, fn.iconst( std::get< std::int64_t >( imm.value ) ) );
                    return;
                }
                case token_type_t::kw_iadd :
                {
                    advance( );
                    const auto lhs = resolve( expect_percent( ) );
                    expect( token_type_t::comma, "','" );
                    const auto rhs = resolve( expect_percent( ) );
                    check_id( result_id, fn.iadd( lhs, rhs ) );
                    return;
                }
                case token_type_t::kw_isub :
                {
                    advance( );
                    const auto lhs = resolve( expect_percent( ) );
                    expect( token_type_t::comma, "','" );
                    const auto rhs = resolve( expect_percent( ) );
                    check_id( result_id, fn.isub( lhs, rhs ) );
                    return;
                }
                case token_type_t::kw_imul :
                {
                    advance( );
                    const auto lhs = resolve( expect_percent( ) );
                    expect( token_type_t::comma, "','" );
                    const auto rhs = resolve( expect_percent( ) );
                    check_id( result_id, fn.imul( lhs, rhs ) );
                    return;
                }
                case token_type_t::kw_iand :
                {
                    advance( );
                    const auto lhs = resolve( expect_percent( ) );
                    expect( token_type_t::comma, "','" );
                    const auto rhs = resolve( expect_percent( ) );
                    check_id( result_id, fn.iand( lhs, rhs ) );
                    return;
                }
                case token_type_t::kw_idiv :
                {
                    advance( );
                    const auto lhs = resolve( expect_percent( ) );
                    expect( token_type_t::comma, "','" );
                    const auto rhs = resolve( expect_percent( ) );
                    check_id( result_id, fn.idiv( lhs, rhs ) );
                    return;
                }
                case token_type_t::kw_icmp :
                {
                    advance( );
                    expect( token_type_t::dot, "'.'" );
                    const auto kind_tok = expect( token_type_t::identifier, "comparison kind" );
                    const auto kind     = parse_set_cc_kind( std::get< std::string >( kind_tok.value ) );
                    const auto lhs      = resolve( expect_percent( ) );
                    expect( token_type_t::comma, "','" );
                    const auto rhs = resolve( expect_percent( ) );
                    check_id( result_id, fn.icmp( kind, lhs, rhs ) );
                    return;
                }
                case token_type_t::kw_call :
                {
                    advance( );
                    call_target_t target = parse_call_target( );
                    const auto    args   = parse_arg_list( );
                    check_id( result_id, fn.call( args, target ) );
                    return;
                }
                default : throw parse_error( "unexpected instruction at position " + std::to_string( m_current.pos ) );
            }
        }

        static set_cc_kind parse_set_cc_kind( const std::string &s )
        {
            if ( s == "eq" )
                return set_cc_kind::eq;
            if ( s == "ne" )
                return set_cc_kind::ne;
            if ( s == "lt" )
                return set_cc_kind::lt;
            if ( s == "le" )
                return set_cc_kind::le;
            if ( s == "gt" )
                return set_cc_kind::gt;
            if ( s == "ge" )
                return set_cc_kind::ge;
            throw parse_error( "unknown comparison kind: " + s );
        }

        call_target_t parse_call_target( )
        {
            if ( m_current.type == token_type_t::kw_native )
            {
                advance( );
                expect( token_type_t::at, "'@'" );
                const auto addr_tok = expect( token_type_t::integer, "address literal" );
                const auto addr     = static_cast< std::uintptr_t >( std::get< std::int64_t >( addr_tok.value ) );
                return call_target_t { .target = reinterpret_cast< const void * >( addr ) };
            }

            expect( token_type_t::at, "'@'" );
            const auto name = std::get< std::string >( expect( token_type_t::identifier, "function name" ).value );
            return call_target_t { .target = name };
        }
    };
}