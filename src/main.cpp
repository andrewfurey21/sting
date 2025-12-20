// TODO: use precompiled headers
#include <cmath>
#include <cstdint>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <filesystem>
#include <string>

using u64 = uint64_t;
using u32 = uint32_t;
using u8 = char;
using i64 = int64_t;
using i32 = int32_t;
using f32 = float_t;

namespace sting {

const u64 DEFAULT_SIZE = 1 << 8;

void panic_if(bool expr, const std::string& msg, const i32 code) {
    if (!expr) return;
    std::cerr << "---------------- ERROR ----------------\n" << "Code: "
        << code << "\n" << msg << "\n"
        << "---------------------------------------\n";
    exit(code);
}

template <typename T, typename U = T>
T exchange(T& other, U&& newval) {
    T old = other;
    other = newval;
    return old;
}

namespace type_traits {

template <typename T>
struct remove_reference {
    typedef T type;
};

template <typename T>
struct remove_reference<T&> {
    typedef T type;
};

template <typename T>
struct remove_reference<T&&> {
    typedef T type;
};

}

// identical to std::move, but steal sounds better
// question: why do we need the remove_reference indirection?
// is decltype auto because we don't know the return type
// until we get to return?
template <typename T>
decltype(auto) steal(T&& value) {
    return static_cast<typename type_traits::remove_reference<T>::type&&>(value);
}

// currently just a stack, can only push/pop.
// look into semistable::vector
template <typename T>
class dynamic_array {
public:
    dynamic_array() : dynamic_array(DEFAULT_SIZE) {}

    dynamic_array(u64 capacity) : _capacity(capacity), _size(0) {
        _data = allocate_capacity();
    }

    dynamic_array(const std::initializer_list<T>& list) :
        _capacity(list.size()),
        _size(0)
    {
        _data = allocate_capacity();
        for (auto&& item : list)
            push_back(item);
    }

    dynamic_array(const dynamic_array& other) {
        this->_capacity = other._capacity;
        this->_size = other._size;
        _data = copy_array(other);
    }

    dynamic_array(dynamic_array&& other) {
        steal_array(sting::steal(other));
    }

    dynamic_array& operator=(const dynamic_array& other) {
        if (this != &other) {
            free_array(_data);
            _size = other._size;
            _capacity = other._capacity;
            _data = copy_array(other);
        }
        return *this;
    }

    dynamic_array& operator=(dynamic_array&& other) {
        if (this != &other) {
            steal_array(sting::steal(other));
        }
        return *this;
    }

    ~dynamic_array() {
        free_array(_data);
    }

    // TODO: impl std::expected
    T& at(u64 index) const {
        panic_if(index >= _size, "Index out of bounds.", -1);
        return _data[index];
    }

    u64 size() const { return _size; }

    u64 capacity() const { return _capacity; }

    void push_back(const T& x) {
        _size++;
        if (_capacity <= _size) {
            _capacity *= 2;
            resize_array();
        }
        at(_size - 1) = x;
    }

    // TODO: impl std::expected
    T pop_back() {
        T ret = at(_size - 1);
        _size--;
        _data[_size].~T();
        return ret;
    }

    T* data() const { return _data; }

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const dynamic_array<U>& other);

private:
    T* allocate_capacity() {
        return static_cast<T*>(malloc(sizeof(T) * this->_capacity)); // try reinterpret/dynamic
    }

    T* copy_array(const dynamic_array<T>& other) {
        T* data = allocate_capacity();
        for (u64 i{0}; i < this->size(); i++) {
            new (&data[i]) T(other.at(i));
        }
        return data;
    }

    void resize_array() {
        T* new_data = copy_array(*this);
        free_array(_data);
        _data = new_data;
    }

    void free_array(T* data) {
        for (u64 i{0}; i < this->size(); i++) {
            data[i].~T();
        }
        free(data);
        data = nullptr;
    }

    void steal_array(dynamic_array&& other) {
        this->_capacity = exchange(other._capacity, 1);
        this->_size = exchange(other._size, 0);
        this->_data = exchange(other._data, nullptr);
    }

    u64 _capacity;
    u64 _size;
    T* _data;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const dynamic_array<T>& other) {
    for (u64 i{0}; i < other.size(); i++) {
        os << std::setw(4) << std::setfill('0') << i << ' ';
        os << other.at(i);
        if (i != other.size() - 1) {
            os << "\n";
        }
    }
    return os;
}

enum class opcode {
    RETURN,
    LOAD_CONST,
    NEGATE,
    ADD,
    MULTIPLY,
    DIVIDE,
    SUBTRACT,
};

// should be std::expected
std::string opcode_to_string(opcode op) {
    switch (op) {
        case opcode::RETURN:
            return "RETURN";
        case opcode::LOAD_CONST:
            return "CONST";
        case opcode::NEGATE:
            return "NEGATE";
        case opcode::ADD:
            return "ADD";
        case opcode::MULTIPLY:
            return "MULTIPLY";
        case opcode::DIVIDE:
            return "DIVIDE";
        case opcode::SUBTRACT:
            return "SUBTRACT";
        default:
            return "UNKNOWN";
    }
}

// NOTE: could possibly be a base class whose derivatives point to memory of some size
struct value {
    f32 data;

    value operator+(const value& other) const {
        return value { .data = this->data + other.data };
    }

    value operator-(const value& other) const {
        return value { .data = this->data - other.data };
    }

    value operator*(const value& other) const {
        return value { .data = this->data * other.data };
    }

    value operator/(const value& other) const {
        return value { .data = this->data / other.data };
    }
};

// NOTE: might just have a std::array impl? (heap allocated though)
struct instruction {
    opcode op;
    u32 a;
    u32 b;
    u32 c;
};

std::ostream& operator<<(std::ostream& os, const instruction& instr) {
    os << opcode_to_string(instr.op) << ": " << instr.a << ", " << instr.b << ", " << instr.c << ", ";
    return os;
}

struct chunk {
    chunk(const std::string& name) : name(name) {}
    std::string name;
    dynamic_array<instruction> bytecode;
    dynamic_array<value> constant_pool;
    dynamic_array<u64> lines;

    void write_instruction(const opcode op, u64 line, u32 a = 0, u32 b = 0, u32 c = 0) {
        instruction instr = {
            .op = op,
            .a = a,
            .b = b,
            .c = c
        };

        // lines.size() == bytecode.size();
        lines.push_back(line);
        bytecode.push_back(instr);
    }

    u32 load_constant(const value& val) {
        u32 index = constant_pool.size();
        constant_pool.push_back(val);
        return index;
    }

    friend std::ostream& operator<<(std::ostream& os, const chunk& chk);
};

std::ostream& operator<<(std::ostream& os, const chunk& chk) {
    os << "---- CHUNK: " << chk.name << " ---- \n";
    for (u64 i{0}; i < chk.bytecode.size(); i++) {
        const instruction& instr = chk.bytecode.at(i);
        os << instr << "\t";
        switch (instr.op) {
            case opcode::LOAD_CONST:
                os << "Value(" << chk.constant_pool.at(instr.a).data << ")";
            default:
                os << "\t";
        }
        os << "\tline: " << chk.lines.at(i) << "\n";
    }
    return os;
}


enum class vm_result { // just result?
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class virtual_machine {
public:
    virtual_machine(const chunk& chk) : chk(chk) {
        pc = chk.bytecode.data();
    }

    vm_result run_chunk() {
        for (;;) {
            const instruction& current = *pc;
            pc++;

            switch(current.op) {
                case opcode::RETURN: {
                    if (value_stack.size() > 0) {
                        const value& val = value_stack.at(value_stack.size() - 1);
                        std::cerr << val.data << "\n";
                    }
                    return vm_result::OK;
                }

                case opcode::LOAD_CONST: {
                    u64 index = current.a;
                    const value& val = chk.constant_pool.at(index);
                    value_stack.push_back(val);
                    break;
                }

                case opcode::NEGATE: {
                    value a = value_stack.pop_back();
                    a.data *= -1;
                    value_stack.push_back(a);
                    break;
                }

                case opcode::ADD: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = a + b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::MULTIPLY: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = a * b;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::DIVIDE: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = b / a;
                    value_stack.push_back(c);
                    break;
                }

                case opcode::SUBTRACT: {
                    const value a = value_stack.pop_back();
                    const value b = value_stack.pop_back();
                    const value c = a - b;
                    value_stack.push_back(c);
                    break;
                }


                default: {
                    std::stringstream errMessage;
                    errMessage << "Unknown opcode: " << static_cast<i32>(current.op);
                    panic_if(true, errMessage.str(), -1);
                }
            }
        }
    }

private:
    chunk chk;
    instruction* pc;
    dynamic_array<value> value_stack;
};


std::string read_file(const std::filesystem::path& path) {
    const u64 size = std::filesystem::file_size(path);
    u8* data = (u8*)malloc(size * sizeof(u8*));
    std::ifstream f(path);
    f.read(data, size);
    std::string str(data, size);
    free(data);
    return str;
}

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

// TODO: inheritance for different tokens
struct token {
    token_type type;
    u8* start;
    u64 length;
    u64 line;
};

class scanner {
public:
    scanner(u8* source, u64 size) : source(source), current(source), line(1), size(size) {

    }

    bool tokenize(dynamic_array<token>& tokens) {
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

    token next_token() {
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

    u8 get_char() {
        u8 ret = *current;
        current++;
        return ret;
    }

    u8 peek_next() {
        if (at_end(current + 1)) return '\0';
        return current[1];
    }

    void skip_whitespace() {
        for (;;) {
            u8 check = *current;
            switch (check) {
                case '/': {
                    if (peek_next() == '/') {
                        current += 2;
                        while (!at_end(current) && get_char() != '\n'); // lol
                        line++;
                    }
                    return;
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

    bool match_next_char(u8 next) {
        u8 check = *current;
        if (at_end(current) || check != next)
            return false;
        current++;
        return true;
    }

    token build_token(token_type type, u8* start) {
        u64 len = current - start;
        return token {
            .type = type,
            .start = start,
            .length = len,
            .line = line
        };
    }

    token error_token(u8* message) { // null terminated message
        return token {
            .type = token_type::ERROR,
            .start = message,
            .length = strlen(message),
            .line = line
        };
    }

    token string_token() {
        u8* start = current++;
        while (peek_next() != '\"') {
            if (at_end(current)) return error_token(const_cast<u8*>("unterminated string."));
            current++;
        }
        token t = build_token(token_type::STRING, start);
        current += 2;
        return t;
    }

    token number_token() {
        u8* start = current;
        while (is_digit(*current)) current++;

        if (*current == '.' && is_digit(peek_next()))
            current++;

        while (is_digit(*current)) current++;

        return build_token(token_type::NUMBER, start);
    }

    token identifier_token() {
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

    void match_keyword(char* ident, u64 size, const char* keyword,
                       token_type type, token& tok) {
        if (strlen(keyword) != size) return;
        if (strncmp(ident, keyword, size) != 0) return;
        tok = build_token(type, ident);
    }

    bool is_alpha(u8 ch) {
        if (ch == '_') return true;
        if ('A' <= ch && ch <= 'Z') return true;
        if ('a' <= ch && ch <= 'z') return true;
        return false;
    }

    bool is_digit(u8 ch) { return '0' <= ch && ch <= '9'; }

    bool at_end(u8* char_ptr) {
        return (char_ptr - source) == size;
    }

private:
    u8* source;
    u8* current;
    u64 line;
    u64 size;
};

enum class precedence {
    NONE,
    ASSIGNMENT,  // =
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    TERM,        // + -
    FACTOR,      // * /
    UNARY,       // ! -
    CALL,        // . ()
    PRIMARY
};


class pratt_parser {
public:
    pratt_parser(const std::string& name) :
        chk(name),
        index{},
        panic{}, parse_error{} {}

    bool parse() { // should probably return vm_result.
        current = &tokens.at(0);
        get_next_token();

        expression();

        consume(token_type::END_OF_FILE, "Expected end of file");
        if (panic) return !parse_error;

        chk.write_instruction(opcode::RETURN, current->line);
        return true;
    }

    dynamic_array<token>& get_tokens() { return tokens; }
    chunk& get_chunk() { return chk; }
private:

    void error_at_token(const token& t, const std::string& msg) {
        if (panic) return;
        panic = true;
        parse_error = true;
        fprintf(stdout, "Error at line %ld: ", t.line);
        if (t.type == token_type::END_OF_FILE) {
            fprintf(stdout, "at end of file.\n");
        } else {
            fprintf(stdout, "%s, got %.*s\n", msg.data(), (int)t.length, t.start);
        }
    }

    void check_current_token(const token_type expected, const std::string& msg) {
        if (current->type != expected) {
            error_at_token(*current, msg);
        }
    }

    void get_next_token() {
        prev = current;
        current = &tokens.at(index);
    }

    void consume(token_type type, const char* msg) {
        get_next_token();
        if (current->type != type) error_at_token(*current, msg);
    }

    void expression() {
    }

    void number() {
        value val;
        val.data = std::stod(std::string(prev->start, prev->length));
        u32 index = chk.load_constant(val);
        chk.write_instruction(opcode::LOAD_CONST, prev->line, index);
    }

    void grouping() {
        // assume { is in previous.
        expression();
        consume(token_type::RIGHT_PAREN, "Expected } after expression");
    }

    void unary() {
        token_type op_type = prev->type;
        expression();

        switch(op_type) {
            case token_type::MINUS: {
                chk.write_instruction(opcode::NEGATE, prev->line);
                break;
            }
            default: return;
        }
    }

    token* prev;
    token* current;
    u64 index;
    dynamic_array<token> tokens;
    chunk chk;
    bool parse_error;
    bool panic;
};

vm_result interpret(const std::filesystem::path& file) {
    bool result;
    std::string source = read_file(file);
    // std::cout << "------- SOURCE -------\n" << source
    //           << "----------------------\n";
    scanner scan(source.data(), source.size());
    pratt_parser parser(file.string());
    result = scan.tokenize(parser.get_tokens());
    if (!result) return vm_result::COMPILE_ERROR;

    result = parser.parse();
    if (!result) return vm_result::COMPILE_ERROR;
    virtual_machine vm(parser.get_chunk());

    return vm.run_chunk();
}

void manage_result(vm_result result) {
    u8 code = -1;
    switch (result) {
        case sting::vm_result::OK: {
            std::cerr << "Success.\n";
            code = 0;
            break;
        }
        case sting::vm_result::COMPILE_ERROR: {
            std::cerr << "Compiler error.\n";
            break;
        }
        case sting::vm_result::RUNTIME_ERROR: {
            std::cerr << "Runtime error.\n";
            break;
        }
        default:
            std::cerr << "Unknown interpreter result.\n";
    }
    exit(code);
}

}

i32 main() {
    const std::filesystem::path file("main.sting");
    sting::vm_result result = sting::interpret(file);
    sting::manage_result(result);
    return 0;
}
