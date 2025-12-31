#ifndef PARSER_HPP
#define PARSER_HPP

#include "utilities.hpp"
#include "dynarray.hpp"
#include "scanner.hpp"
#include "vmachine.hpp"
#include "object.hpp"
#include "hashmap.hpp"

namespace sting {

enum precedence {
    NONE,        // 0
    ASSIGNMENT,  // 1 =
    OR,          // 2 or
    AND,         // 3 and
    EQUALITY,    // 4 == !=
    COMPARISON,  // 5 < > <= >=
    TERM,        // 6 + -
    FACTOR,      // 7 * /
    UNARY,       // 8 ! -
    CALL,        // 9 . ()
    PRIMARY      // 10
};

class parser {
public:
    parser();
    parser(const std::string& name);
    bool parse();
    dynarray<token>& get_tokens();
    chunk& get_chunk();
    void error_at_token(const token& t, const std::string& msg);
    void check_current_token(const token_type expected, const std::string& mesg);
    void get_next_token();
    void consume(token_type type, const char* msg);
    void parse_precedence(precedence p);
    void declaration();
    void var_declaration();
    void variable();
    void named_variable(const token& name);
    void statement();
    void expression_statement();
    void expression();
    void number();
    void literal();
    void str();
    void grouping();
    void unary();
    void binary();
    void print();

    bool match(token_type type);

    token* prev;
    token* current;
    u64 index;
    dynarray<token> tokens;
    chunk chk;
    bool parse_error;
    bool panic;
};

using parse_fn = void (parser::*)();

struct parse_rule {
    parse_fn prefix;
    parse_fn infix;
    precedence prec;
};

parse_rule* get_rule(token_type type);

} // namespace sting

#endif
