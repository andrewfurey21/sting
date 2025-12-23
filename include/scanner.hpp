#include "sting.hpp"

namespace sting {

enum class token_type {
  // Single-character tokens.
  LEFT_PAREN, RIGHT_PAREN,
  LEFT_BRACE, RIGHT_BRACE,
  COMMA, DOT, MINUS, PLUS,
  SEMICOLON, SLASH, STAR,
  // One or two character tokens.
  BANG, BANG_EQUAL,
  EQUAL, EQUAL_EQUAL,
  GREATER, GREATER_EQUAL,
  LESS, LESS_EQUAL,
  // Literals.
  IDENTIFIER, STRING, NUMBER,
  // Keywords.
  AND, CLASS, ELSE, FALSE,
  FOR, FUN, IF, NIL, OR,
  PRINT, RETURN, SUPER, THIS,
  TRUE, VAR, WHILE,

  ERROR, END_OF_FILE
};

struct token {
    token_type type;
    u8* start;
    u64 length;
    u64 line;
};

// TODO: track column and do multi lines (nested) comments
class scanner {
public:
    scanner(u8* source, u64 size) :
        source(source),
        current(source),
        line(1),
        size(size) { }

    bool tokenize(dynamic_array<token>& tokens);

    token next_token();

    u8 get_char();

    u8 peek_next();

    void skip_whitespace();

    bool match_next_char(u8 next);

    token build_token(token_type type, u8* start);

    token error_token(u8* message);

    token string_token();

    token number_token();

    token identifier_token();

    void match_keyword(char* ident, u64 size, const char* keyword,
                       token_type type, token& tok);

    bool is_alpha(u8 ch);

    bool is_digit(u8 ch);

    bool at_end(u8* char_ptr);

private:
    u8* source;
    u8* current;
    u64 line;
    u64 size;
};


} // namespace sting
