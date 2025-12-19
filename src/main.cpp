// TODO: use precompiled headers
#include <cmath>
#include <cstdint>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <filesystem>

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

    u64 load_constant(const value& val) {
        u64 index = constant_pool.size();
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


enum class vm_result {
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
    scanner(u8* source, u64 size) : source(source), current(source), line(1), size(size) {}

    token next_token() {
        skip_whitespace();
        u8* start = current;
        u8 c = get_char();
        if (at_end(start)) return build_token(token_type::END_OF_FILE, start);

        auto build_token_start = [=](token_type type) -> token {
            return this->build_token(type, start);
        };

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
        current+=2;
        return t;
    }

    bool at_end(u8* char_ptr) {
        return (char_ptr - source) == size;
    }

private:
    u8* source;
    u8* current;
    u64 line;
    u64 size;
};

}

int main() {
    u64 current_line = 0;
    std::string source = sting::read_file("main.sting");
    std::cout << "----------- Source -------------\n" << source
              << "--------------------------------\n";
    sting::scanner scanner(source.data(), source.size());
    for (;;) {
        sting::token t = scanner.next_token();
        if (t.type == sting::token_type::ERROR) {
            printf("%.*s\n", (int)t.length, t.start);
            break;
        } else if (t.type == sting::token_type::END_OF_FILE) {
            break;
        } else if (t.line != current_line) {
            std::cerr << t.line;
            current_line = t.line;
        } else {
            std::cerr << "|";
        }

        printf("%.*s\n", (int)t.length, t.start);
    }

    return 0;
}
