#include <amd64/ir/ir.hpp>
#include <cstdint>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>
#include <amd64/ir/fmt.hpp>
#include <amd64/codegen/verify.hpp>
#include <amd64/codegen/codegen.hpp>
#include <amd64/assembler/assembler.hpp>
#include <fstream>
#include <sstream>
#include <print>
#include <Zydis/Zydis.h>

void print_disassembly(std::span<const std::byte> bytes, uint64_t runtime_address = 0x1000) {
    ZydisDisassembledInstruction instruction;
    size_t offset = 0;

    while (ZYAN_SUCCESS(ZydisDisassembleIntel(
        ZYDIS_MACHINE_MODE_LONG_64,
        runtime_address + offset,
        bytes.data() + offset,
        bytes.size() - offset,
        &instruction
    ))) {
        std::println("{:#x}: {}", runtime_address + offset, instruction.text);

        offset += instruction.info.length;
    }
}

namespace tinyc
{
    using namespace amd64::ir;

    enum class token_kind_t
    {
        kw_fn,
        kw_let,
        kw_return,
        kw_if,
        kw_else,
        ident,
        integer,
        plus,
        minus,
        star,
        slash,
        eq,
        ne,
        lt,
        le,
        gt,
        ge,
        assign,
        comma,
        semi,
        lparen,
        rparen,
        lbrace,
        rbrace,
        eof
    };

    struct token_t
    {
        token_kind_t kind;
        std::string  value;
        std::int64_t int_value {};
        std::size_t  pos {};
    };

    class lexer_t
    {
    public:
        explicit lexer_t( const std::string_view src ) : m_src( src ) { }

        token_t next( )
        {
            skip_ws( );
            if ( m_pos >= m_src.size( ) )
                return { token_kind_t::eof, "", 0, m_pos };

            const std::size_t start = m_pos;
            const char        chr   = m_src[ m_pos ];

            if ( chr == '+' )
            {
                ++m_pos;
                return { token_kind_t::plus, "+", 0, start };
            }
            if ( chr == '-' )
            {
                ++m_pos;
                return { token_kind_t::minus, "-", 0, start };
            }
            if ( chr == '*' )
            {
                ++m_pos;
                return { token_kind_t::star, "*", 0, start };
            }
            if ( chr == '/' )
            {
                ++m_pos;
                return { token_kind_t::slash, "/", 0, start };
            }
            if ( chr == ',' )
            {
                ++m_pos;
                return { token_kind_t::comma, ",", 0, start };
            }
            if ( chr == ';' )
            {
                ++m_pos;
                return { token_kind_t::semi, ";", 0, start };
            }
            if ( chr == '(' )
            {
                ++m_pos;
                return { token_kind_t::lparen, "(", 0, start };
            }
            if ( chr == ')' )
            {
                ++m_pos;
                return { token_kind_t::rparen, ")", 0, start };
            }
            if ( chr == '{' )
            {
                ++m_pos;
                return { token_kind_t::lbrace, "{", 0, start };
            }
            if ( chr == '}' )
            {
                ++m_pos;
                return { token_kind_t::rbrace, "}", 0, start };
            }

            if ( chr == '=' && peek( ) == '=' )
            {
                m_pos += 2;
                return { token_kind_t::eq, "==", 0, start };
            }
            if ( chr == '!' && peek( ) == '=' )
            {
                m_pos += 2;
                return { token_kind_t::ne, "!=", 0, start };
            }
            if ( chr == '<' && peek( ) == '=' )
            {
                m_pos += 2;
                return { token_kind_t::le, "<=", 0, start };
            }
            if ( chr == '>' && peek( ) == '=' )
            {
                m_pos += 2;
                return { token_kind_t::ge, ">=", 0, start };
            }
            if ( chr == '<' )
            {
                ++m_pos;
                return { token_kind_t::lt, "<", 0, start };
            }
            if ( chr == '>' )
            {
                ++m_pos;
                return { token_kind_t::gt, ">", 0, start };
            }
            if ( chr == '=' )
            {
                ++m_pos;
                return { token_kind_t::assign, "=", 0, start };
            }

            if ( std::isdigit( chr ) )
            {
                while ( m_pos < m_src.size( ) && std::isdigit( m_src[ m_pos ] ) )
                    ++m_pos;
                const std::string s( m_src.substr( start, m_pos - start ) );
                return { token_kind_t::integer, s, std::stoll( s ), start };
            }

            if ( std::isalpha( chr ) || chr == '_' )
            {
                while ( m_pos < m_src.size( ) && ( std::isalnum( m_src[ m_pos ] ) || m_src[ m_pos ] == '_' ) )
                    ++m_pos;
                const std::string s( m_src.substr( start, m_pos - start ) );
                if ( s == "fn" )
                    return { token_kind_t::kw_fn, s, 0, start };
                if ( s == "let" )
                    return { token_kind_t::kw_let, s, 0, start };
                if ( s == "return" )
                    return { token_kind_t::kw_return, s, 0, start };
                if ( s == "if" )
                    return { token_kind_t::kw_if, s, 0, start };
                if ( s == "else" )
                    return { token_kind_t::kw_else, s, 0, start };
                return { token_kind_t::ident, s, 0, start };
            }

            throw std::runtime_error( "unexpected character at pos " + std::to_string( start ) );
        }

    private:
        std::string_view m_src;
        std::size_t      m_pos {};

        char peek( ) const { return m_pos + 1 < m_src.size( ) ? m_src[ m_pos + 1 ] : '\0'; }

        void skip_ws( )
        {
            while ( m_pos < m_src.size( ) && std::isspace( m_src[ m_pos ] ) )
                m_pos++;
        }
    };

    struct expr;
    using expr_ptr = std::unique_ptr< expr >;

    struct lit_expr
    {
        std::int64_t value;
    };

    struct var_expr
    {
        std::string name;
    };

    struct bin_expr
    {
        std::string op;
        expr_ptr    lhs, rhs;
    };

    struct call_expr
    {
        std::string             name;
        std::vector< expr_ptr > args;
    };

    struct if_expr
    {
        expr_ptr cond, then_ex, else_ex;
    };

    struct expr
    {
        std::variant< lit_expr, var_expr, bin_expr, call_expr, if_expr > node;
    };

    struct stmt;
    using stmt_ptr = std::unique_ptr< stmt >;

    struct let_stmt
    {
        std::string name;
        expr_ptr    value;
    };

    struct ret_stmt
    {
        expr_ptr value;
    };

    struct expr_stmt
    {
        expr_ptr value;
    };

    struct stmt
    {
        std::variant< let_stmt, ret_stmt, expr_stmt > node;
    };

    struct fn_decl
    {
        std::string                name;
        std::vector< std::string > params;
        std::vector< stmt_ptr >    body;
    };

    class parser_t
    {
    public:
        explicit parser_t( const std::string_view src ) : m_lexer( src ), m_curr( m_lexer.next( ) ) { }

        std::vector< fn_decl > parse( )
        {
            std::vector< fn_decl > decls;
            while ( m_curr.kind != token_kind_t::eof )
                decls.push_back( parse_fn( ) );
            return decls;
        }

    private:
        lexer_t m_lexer;
        token_t m_curr;

        token_t eat( const token_kind_t k, std::string_view what )
        {
            if ( m_curr.kind != k )
                throw std::runtime_error( std::format( "expected {}", what ) );
            auto t = m_curr;
            m_curr = m_lexer.next( );
            return t;
        }

        token_t advance( )
        {
            auto t = m_curr;
            m_curr = m_lexer.next( );
            return t;
        }

        fn_decl parse_fn( )
        {
            eat( token_kind_t::kw_fn, "'fn'" );
            auto name = eat( token_kind_t::ident, "function name" ).value;
            eat( token_kind_t::lparen, "'('" );

            std::vector< std::string > params;
            while ( m_curr.kind != token_kind_t::rparen )
            {
                if ( !params.empty( ) )
                    eat( token_kind_t::comma, "','" );
                params.push_back( eat( token_kind_t::ident, "param" ).value );
            }

            eat( token_kind_t::rparen, "')'" );
            eat( token_kind_t::lbrace, "'{'" );

            std::vector< stmt_ptr > body {};
            while ( m_curr.kind != token_kind_t::rbrace )
                body.push_back( parse_stmt( ) );
            eat( token_kind_t::rbrace, "'}'" );

            return { std::move( name ), std::move( params ), std::move( body ) };
        }

        stmt_ptr parse_stmt( )
        {
            if ( m_curr.kind == token_kind_t::kw_let )
            {
                advance( );

                auto name = eat( token_kind_t::ident, "name" ).value;
                eat( token_kind_t::assign, "'='" );
                auto val = parse_expr( );
                eat( token_kind_t::semi, "';'" );
                return std::make_unique< stmt >( stmt {
                    let_stmt { std::move( name ), std::move( val ) }
                } );
            }

            if ( m_curr.kind == token_kind_t::kw_return )
            {
                advance( );
                auto val = parse_expr( );
                eat( token_kind_t::semi, "';'" );
                return std::make_unique< stmt >( stmt { ret_stmt { std::move( val ) } } );
            }

            auto e = parse_expr( );
            eat( token_kind_t::semi, "';'" );

            return std::make_unique< stmt >( stmt { expr_stmt { std::move( e ) } } );
        }

        expr_ptr parse_expr( ) { return parse_cmp( ); }

        expr_ptr parse_cmp( )
        {
            auto lhs = parse_add( );
            while ( m_curr.kind == token_kind_t::eq || m_curr.kind == token_kind_t::ne || m_curr.kind == token_kind_t::lt
                    || m_curr.kind == token_kind_t::le || m_curr.kind == token_kind_t::gt || m_curr.kind == token_kind_t::ge )
            {
                const auto op  = advance( ).value;
                auto       rhs = parse_add( );
                lhs            = std::make_unique< expr >( expr {
                    bin_expr { op, std::move( lhs ), std::move( rhs ) }
                } );
            }
            return lhs;
        }

        expr_ptr parse_add( )
        {
            auto lhs = parse_mul( );
            while ( m_curr.kind == token_kind_t::plus || m_curr.kind == token_kind_t::minus )
            {
                const auto op  = advance( ).value;
                auto       rhs = parse_mul( );
                lhs            = std::make_unique< expr >( expr {
                    bin_expr { op, std::move( lhs ), std::move( rhs ) }
                } );
            }
            return lhs;
        }

        expr_ptr parse_mul( )
        {
            auto lhs = parse_unary( );
            while ( m_curr.kind == token_kind_t::star || m_curr.kind == token_kind_t::slash )
            {
                const auto op  = advance( ).value;
                auto       rhs = parse_unary( );
                lhs            = std::make_unique< expr >( expr {
                    bin_expr { op, std::move( lhs ), std::move( rhs ) }
                } );
            }
            return lhs;
        }

        expr_ptr parse_unary( ) { return parse_primary( ); }

        expr_ptr parse_primary( )
        {
            if ( m_curr.kind == token_kind_t::integer )
            {
                const auto v = m_curr.int_value;
                advance( );
                return std::make_unique< expr >( expr { lit_expr { v } } );
            }
            if ( m_curr.kind == token_kind_t::ident )
            {
                auto name = advance( ).value;
                if ( m_curr.kind == token_kind_t::lparen )
                {
                    advance( );
                    std::vector< expr_ptr > args;

                    while ( m_curr.kind != token_kind_t::rparen )
                    {
                        if ( !args.empty( ) )
                            eat( token_kind_t::comma, "','" );
                        args.push_back( parse_expr( ) );
                    }

                    eat( token_kind_t::rparen, "')'" );
                    return std::make_unique< expr >( expr {
                        call_expr { std::move( name ), std::move( args ) }
                    } );
                }
                return std::make_unique< expr >( expr { var_expr { std::move( name ) } } );
            }
            if ( m_curr.kind == token_kind_t::kw_if )
            {
                advance( );

                auto cond = parse_expr( );
                eat( token_kind_t::lbrace, "'{'" );
                auto then_e = parse_expr( );

                eat( token_kind_t::rbrace, "'}'" );
                eat( token_kind_t::kw_else, "'else'" );
                eat( token_kind_t::lbrace, "'{'" );

                auto else_e = parse_expr( );
                eat( token_kind_t::rbrace, "'}'" );

                return std::make_unique< expr >( expr {
                    if_expr { std::move( cond ), std::move( then_e ), std::move( else_e ) }
                } );
            }

            if ( m_curr.kind == token_kind_t::lparen )
            {
                advance( );
                auto e = parse_expr( );
                eat( token_kind_t::rparen, "')'" );
                return e;
            }

            throw std::runtime_error( "unexpected token: " + m_curr.value );
        }
    };

    class lowerer_t
    {
    public:
        module_t lower( const std::vector< fn_decl >& fns )
        {
            for ( const auto& fn : fns )
                m_mod.create_function( fn.name, std::vector( fn.params.size(  ), type_t::i64 ), type_t::i64 );

            for ( auto i { 0uz }; i < fns.size(  ); ++i )
                lower_fn( fns[ i ], m_mod.functions[ i ] );

            return std::move( m_mod );
        }
    private:
        module_t                                    m_mod;
        std::unordered_map< std::string, Value >    m_locals;
        function_t*                                 m_fn { };

        void lower_fn( const fn_decl& decl, function_t& fn )
        {
            m_fn       = &fn;
            m_locals.clear(  );

            for ( auto i { 0uz }; i < decl.params.size(  ); ++i )
                m_locals[ decl.params[ i ] ] = fn.args[ i ];

            for ( const auto& s : decl.body ) lower_stmt( *s );
        }

        void lower_stmt( const stmt& s )
        {
            std::visit( [& ]< typename T0 >( const T0& st )
            {
                using T = std::decay_t< T0 >;
                if constexpr ( std::is_same_v< T, let_stmt > )
                {
                    m_locals[ st.name ] = lower_expr( *st.value );
                }
                else if constexpr ( std::is_same_v< T, ret_stmt > )
                {
                    m_fn->ret( lower_expr( *st.value ) );
                }
                else if constexpr ( std::is_same_v< T, expr_stmt > )
                {
                    lower_expr( *st.value );
                }
            }, s.node );
        }

        Value lower_expr( const expr& e )
        {
            return std::visit( [&]< typename T1 >( const T1& ex ) -> Value {
                using T = std::decay_t< T1 >;

                if constexpr ( std::is_same_v< T, lit_expr > )
                    return m_fn->iconst( ex.value );

                else if constexpr ( std::is_same_v< T, var_expr > )
                {
                    const auto it = m_locals.find( ex.name );
                    if ( it == m_locals.end() )
                        throw std::runtime_error( "undefined variable: " + ex.name );
                    return it->second;
                }

                else if constexpr ( std::is_same_v< T, bin_expr > )
                {
                    const Value lhs = lower_expr( *ex.lhs );
                    const Value rhs = lower_expr( *ex.rhs );

                    if ( ex.op == "+" ) return m_fn->iadd( lhs, rhs );
                    if ( ex.op == "-" ) return m_fn->isub( lhs, rhs );
                    if ( ex.op == "*" ) return m_fn->imul( lhs, rhs );
                    if ( ex.op == "/" ) return m_fn->idiv( lhs, rhs );

                    const auto kind = [&] -> set_cc_kind {
                        if ( ex.op == "==" ) return set_cc_kind::eq;
                        if ( ex.op == "!=" ) return set_cc_kind::ne;
                        if ( ex.op == "<"  ) return set_cc_kind::lt;
                        if ( ex.op == "<=" ) return set_cc_kind::le;
                        if ( ex.op == ">"  ) return set_cc_kind::gt;
                        if ( ex.op == ">=" ) return set_cc_kind::ge;
                        throw std::runtime_error( "unknown op: " + ex.op );
                    }();

                    return m_fn->icmp( kind, lhs, rhs );
                }

                else if constexpr ( std::is_same_v< T, call_expr > )
                {
                    std::vector<Value> args;

                    for ( const auto& a : ex.args ) args.push_back( lower_expr( *a ) );
                    const call_target_t target { .target = ex.name };

                    return m_fn->call( std::move(args), target );
                }

                else if constexpr ( std::is_same_v< T, if_expr > )
                {
                    const Value cond = lower_expr( *ex.cond );

                    const std::size_t then_idx  = m_fn->blocks.size();
                    const std::size_t else_idx  = then_idx + 1;
                    const std::size_t merge_idx = then_idx + 2;

                    m_fn->brif( cond, then_idx, else_idx, {}, {} );

                    m_fn->create_block( {} );
                    const Value then_val = lower_expr( *ex.then_ex );
                    m_fn->jmp( merge_idx, { then_val } );

                    m_fn->create_block( {} );
                    const Value else_val = lower_expr( *ex.else_ex );
                    m_fn->jmp( merge_idx, { else_val } );

                    m_fn->create_block( { type_t::i64 } );
                    return m_fn->param_indices[merge_idx][0];
                }

                throw std::runtime_error( "unhandled expr" );
            }, e.node );
        }
    };
} // namespace tinyc

std::int32_t main( int argc, char** argv )
{
    const std::string path = argc > 1 ? argv[ 1 ] : "example.tc";

    std::ifstream f { path };
    if ( !f ) { std::println( "error: cannot open file {} ", path ); return 1;
    }

    const std::string src { (std::istreambuf_iterator( f ) ), { } };

    try
    {
        tinyc::parser_t pa( src );
        auto            decls = pa.parse(  );

        tinyc::lowerer_t low;
        auto             mod = low.lower( decls );

        amd64::codegen::verify_module( mod );

        std::println("[out] generated ir\n{}", to_string( mod ));

        amd64::assembler::Assembler azm{ 65536 };
        amd64::codegen::code_gen_t  cg( azm );
        auto gen = cg.compile_module( mod );

        const auto main_fn = gen.get_function<std::int64_t(*)()>( "main" );
        if ( !main_fn ) { std::println( "error: no 'main' function" ); return 1; }

        std::print( "{}", main_fn() );

        print_disassembly( azm.bytes(  ) );
    }
    catch ( const std::exception& e )
    {
        std::println( "error: {}", e.what() );
        return 1;
    }
}
