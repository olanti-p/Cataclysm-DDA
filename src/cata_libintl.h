#pragma once
#ifndef CATA_SRC_CATA_LIBINTL_H
#define CATA_SRC_CATA_LIBINTL_H

#include <memory>
#include <string>
#include <vector>

namespace cata_libintl
{
using u8 = uint8_t;
using u32 = uint32_t;

/**
 * Plural forms AST node type.
 * Also used for token identification during parsing.
 */
enum class PlfOp : int {
    Mod,       // a % b
    Eq,        // a == b
    NotEq,     // a != b
    GreaterEq, // a >= b
    Greater,   // a > b
    LessEq,    // a <= b
    Less,      // a < b
    And,       // a && b
    Or,        // a || b
    TerCond,   // ?

    Literal,   // numeric literal
    Variable,  // the variable (n)

    BrOpen,    // (
    BrClose,   // )
    TerDelim,  // :

    NumOps,
};

struct PlfNode;
using PlfNodePtr = std::unique_ptr<PlfNode>;

/** Plural forms AST node */
struct PlfNode {
    unsigned long literal_val;
    PlfNodePtr a;
    PlfNodePtr b;
    PlfNodePtr c;
    PlfOp op = PlfOp::NumOps;

    unsigned long eval( unsigned long n ) const;

    std::string debug_dump() const;
};

/**
 * Parse plural rules expression and build AST.
 * @throws on failure.
 */
PlfNodePtr parse_plural_rules( const std::string &s );

/**
 * Translation catalogue. Corresponds to single MO file.
 *
 * For reference on MO files, see 'GNU gettext utilities' manual:
 * https://www.gnu.org/software/gettext/manual/html_node/MO-Files.html
 */
class trans_catalogue
{
    private:
        // ======== DECLARATIONS =========
        struct string_info {
            u32 length;
            u32 address;
        };
        struct plf_header_data {
            unsigned long num;
            PlfNodePtr rules;
        };
        using meta_headers = std::vector<std::string>;

        // =========== MEMBERS ===========

        bool is_little_endian = true; // File endianness
        std::string buf; // Data buffer
        unsigned long num_plural_forms = 0; // Number of plural forms
        PlfNodePtr plf_rules = nullptr; // Plural forms expression
        u32 number_of_strings = 0; // Number of strings (ids-translations pairs)
        u32 offs_orig_table = 0; // Offset of table with original strings
        u32 offs_trans_table = 0; // Offset of table with translated strings

        // =========== METHODS ===========

        explicit trans_catalogue( std::string buffer );

        inline u32 buf_size() const {
            return static_cast<u32>( buf.size() );
        }

        u8 get_u8( u32 addr ) const;
        inline u8 get_u8_unsafe( u32 addr ) const {
            return static_cast<u8>( buf[addr] );
        }

        u32 get_u32( u32 addr ) const;
        inline u32 get_u32_unsafe( u32 addr ) const {
            if( is_little_endian ) {
                return get_u8_unsafe( addr ) |
                       get_u8_unsafe( addr + 1 ) << 8 |
                       get_u8_unsafe( addr + 2 ) << 16 |
                       get_u8_unsafe( addr + 3 ) << 24;
            } else {
                return get_u8_unsafe( addr + 3 ) |
                       get_u8_unsafe( addr + 2 ) << 8 |
                       get_u8_unsafe( addr + 1 ) << 16 |
                       get_u8_unsafe( addr ) << 24;
            }
        }

        inline const char *addr_to_cstr( u32 addr ) const {
            return &buf[addr];
        }

        string_info get_string_info( u32 addr ) const;
        string_info get_string_info_unsafe( u32 addr ) const;

        // MO loading
        inline void set_buffer( std::string buffer ) {
            buf = std::move( buffer );
        }
        void process_file_header();
        void check_string_terminators();
        void check_string_plurals();
        std::string get_metadata() const;
        static void check_encoding( const meta_headers &headers );
        static plf_header_data parse_plf_header( const meta_headers &headers );

    public:
        /**
         * Load translation catalogue from given MO file.
         * @throws on failure.
         */
        static trans_catalogue load_from_file( const std::string &file_path );

        inline u32 get_num_strings() const {
            return number_of_strings;
        }

        const char *get_nth_translation( u32 n ) const;
        const char *get_nth_pl_translation( u32 n, unsigned long num ) const;

        // Debug
        const char *get_nth_orig_string( u32 n ) const;
};

/**
 * Translation library.
 * Represents collection of catalogues merged into a single pool ready for use.
 */
class trans_library
{
    private:
        // ======== DECLARATIONS =========
        // Describes single string within the library
        struct string_descriptor {
            u32 catalogue;
            u32 entry;
        };

        // =========== MEMBERS ===========

        // Full index of loaded strings
        std::vector<string_descriptor> string_vec;

        // Full index of loaded catalogues
        std::vector<trans_catalogue> catalogues;

        // =========== METHODS ===========

        std::vector<string_descriptor>::const_iterator find_in_table( const char *id ) const;

        void build_string_table();

        const char *lookup_string_in_table( const char *id ) const;
        const char *lookup_pl_string_in_table( const char *id, unsigned long n ) const;

    public:
        static trans_library create( std::vector<trans_catalogue> catalogues );

        const char *get( const char *msgid ) const;
        const char *get_pl( const char *msgid, const char *msgid_pl, unsigned long n ) const;
        const char *get_ctx( const char *ctx, const char *msgid ) const;
        const char *get_ctx_pl( const char *ctx, const char *msgid, const char *msgid_pl,
                                unsigned long n ) const;
};
} // cata_libintl

#endif // CATA_SRC_CATA_LIBINTL_H
