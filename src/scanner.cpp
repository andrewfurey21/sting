#include "../include/scanner.hpp"

/*

Scanner as a class is probably no good. Should just be one function call,
that returns dynamic_array<token>.

Don't like my use of lambdas.

*/

namespace sting {

bool scanner::tokenize(dynarray<token>& tokens) {
    for (;;) {
        token t = next_token();
        tokens.push_back(t);
        if (t.type == sting::token_type::END_OF_FILE) {
            break;
        } else if (t.type == sting::token_type::ERROR) {
            return false;
        }
    }
    return true;
}

token scanner::next_token() {
    skip_whitespace();
    u8* start = current;
    u8 c = *current;
    if (at_end(start)) return build_token(token_type::END_OF_FILE, start);

    auto build_token_start = [=](token_type type) -> token {
        return this->build_token(type, start);
    };

    if (is_digit(c)) return number_token();
    if (is_alpha(c)) return identifier_token();

    current++;
    switch (c) {
        case '(': return build_token_start(token_type::LEFT_PAREN);
        case ')': return build_token_start(token_type::RIGHT_PAREN);
        case '{': return build_token_start(token_type::LEFT_BRACE);
        case '}': return build_token_start(token_type::RIGHT_BRACE);
        case ';': return build_token_start(token_type::SEMICOLON);
        case ',': return build_token_start(token_type::COMMA);
        case '.': return build_token_start(token_type::DOT);
        case '-': return build_token_start(token_type::MINUS);
        case '+': return build_token_start(token_type::PLUS);
        case '/': return build_token_start(token_type::SLASH);
        case '*': return build_token_start(token_type::STAR);
        case '!':
            return build_token_start(
                match_next_char('=') ? token_type::BANG_EQUAL : token_type::BANG);
        case '=':
            return build_token_start(
                match_next_char('=') ? token_type::EQUAL_EQUAL : token_type::EQUAL);
        case '<':
            return build_token_start(
                match_next_char('=') ? token_type::LESS_EQUAL : token_type::LESS);
        case '>':
            return build_token_start(
                match_next_char('=') ? token_type::GREATER_EQUAL : token_type::GREATER);
        case '\"':
            return string_token();
        default:
            return error_token(const_cast<u8*>("that's not a token i recognize mate."));
    }
}

u8 scanner::get_char() {
    u8 ret = *current;
    current++;
    return ret;
}

u8 scanner::peek_next() {
    if (at_end(current + 1)) return '\0';
    return current[1];
}

void scanner::skip_whitespace() {
    for (;;) {
        u8 check = *current;
        switch (check) {
            case '/': {
                if (peek_next() == '/') {
                    while (!at_end(current) && *current != '\n') {
                        current++;
                    }
                    // while (!at_end(current) && get_char() != '\n');
                    current++;
                    line++;
                } else {
                    return;
                }
                break;
            }
            case ' ':
            case '\t':
            case '\r':
                current++;
                break;
            case '\n':
                line++;
                current++;
                break;
            default:
                return;
        }
    }
}

bool scanner::match_next_char(u8 next) {
    u8 check = *current;
    if (at_end(current) || check != next)
        return false;
    current++;
    return true;
}

token scanner::build_token(token_type type, u8* start) {
    u64 len = current - start;
    return token {
        .type = type,
        .start = start,
        .length = len,
        .line = line
    };
}

token scanner::error_token(u8* message) {
    return token {
        .type = token_type::ERROR,
        .start = message,
        .length = strlen(message),
        .line = line
    };
}

token scanner::string_token() {
    u8* start = current++;
    while (peek_next() != '\"') {
        if (at_end(current)) return error_token(const_cast<u8*>("unterminated string."));
        current++;
    }
    current++;
    token t = build_token(token_type::STRING, start);
    current++;
    return t;
}

token scanner::number_token() {
    u8* start = current;
    while (is_digit(*current)) current++;

    if (*current == '.' && is_digit(peek_next()))
        current++;

    while (is_digit(*current)) current++;

    return build_token(token_type::NUMBER, start);
}

token scanner::identifier_token() {
    u8* start = current;
    while (is_alpha(*current) || is_digit(*current)) {
        current++;
    }

    token t = build_token(token_type::IDENTIFIER, start);

    auto match = [&t, start, this](const char* keyword, token_type type) {
        match_keyword(start, (this->current - start), keyword, type, t);
    };

    match("and", token_type::AND);
    match("class", token_type::CLASS);
    match("else", token_type::ELSE);
    match("false", token_type::FALSE);
    match("for", token_type::FOR);
    match("fun", token_type::FUN);
    match("if", token_type::IF);
    match("nil", token_type::NIL);
    match("or", token_type::OR);
    match("print", token_type::PRINT);
    match("return", token_type::RETURN);
    match("super", token_type::SUPER);
    match("this", token_type::THIS);
    match("true", token_type::TRUE);
    match("var", token_type::VAR);
    match("while", token_type::WHILE);

    return t;
}

void scanner::match_keyword(char* ident, u64 size, const char* keyword,
                       token_type type, token& tok) {
        if (strlen(keyword) != size) return;
        if (strncmp(ident, keyword, size) != 0) return;
        tok = build_token(type, ident);
}

bool scanner::is_alpha(u8 ch) {
    if (ch == '_') return true;
    if ('A' <= ch && ch <= 'Z') return true;
    if ('a' <= ch && ch <= 'z') return true;
    return false;
}

bool scanner::is_digit(u8 ch) { return '0' <= ch && ch <= '9'; }

bool scanner::at_end(u8* char_ptr) { return (char_ptr - source) == size; }

} // namespace sting
