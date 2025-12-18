/*

1. disassembler

*/

// TODO: use precompiled headers
#include <cmath>
#include <cstdint>
#include <cassert>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <ostream>

using u64 = uint64_t;
using u32 = uint32_t;
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
    // os << "Capacity: " << other.capacity() << "\tSize: " << other.size() << "\n";
    for (u64 i{0}; i < other.size(); i++) {
        os << std::setw(4) << std::setfill('0') << i << ' ';
        os << other.at(i);
        if (i != other.size() - 1) {
            os << "\n";
            // os << ", ";
        }
    }
    return os;
}

enum class opcode {
    RETURN,
    CONST
};


// TODO: inherit dynamic_array
// template <typename T, const u64 Size = DEFAULT_SIZE>
// class static_array {
// public:
//     static_array() : array(Size) {}
//     void push_back(const T& t) {
//         array.push_back(t);
//         panic_if(array.size() > Size);
//     }
// private:
//     dynamic_array<T> array;
// };

// should be std::expected
std::string opcode_to_string(opcode op) {
    switch (op) {
        case opcode::RETURN:
            return "RETURN";
        case opcode::CONST:
            return "CONST";
        default:
            return "UNKNOWN";
    }
}

// NOTE: could possibly be a base class whose derivatives point to memory of some size
struct value {
    f32 data;
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

    u64 add_constant(const value& val) {
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
            case opcode::CONST:
                os << "Value(" << chk.constant_pool.at(instr.a).data << ")";
            default:
                os << "\tline: " << chk.lines.at(i) << "\n";
        }
    }
    return os;
}

}

int main() {

    sting::chunk hello("hello world");

    sting::value a { .data = 0.5f };
    u64 index = hello.add_constant(a);
    hello.write_instruction(sting::opcode::CONST, 0, index);

    sting::value b { .data = 1.5f };
    index = hello.add_constant(a);
    hello.write_instruction(sting::opcode::CONST, 256, index);

    sting::value c { .data = 3.5f };
    index = hello.add_constant(a);
    hello.write_instruction(sting::opcode::CONST, 2, index);

    std::cerr << hello << "\n";
    return 0;
}
