/*

1. disassembler

*/

// TODO: use precompiled headers
#include <cstdint>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <ostream>

using u64 = uint64_t;
using i32 = int32_t;
using f32 = float;

namespace sting {

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
    dynamic_array() : dynamic_array(256) {}

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
        std::cerr << "copying dynamic array\n";
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
    os << "Capacity: " << other.capacity() << "\tSize: " << other.size() << "\n";
    for (u64 i{0}; i < other.size(); i++) {
        os << other.at(i);
        if (i != other.size() - 1) {
            os << "\n";
            // os << ", ";
        }
    }
    return os;
}

enum class opcode {
    RETURN
};

std::string opcode_to_string(opcode op) {
    switch (op) {
        case opcode::RETURN:
            return "RETURN";
        default:
            return "UNKNOWN";
    }
}

std::ostream& operator<<(std::ostream& os, opcode op) {
    os << opcode_to_string(op);
    return os;
}

}

int main() {
    sting::dynamic_array<sting::opcode> a;
    std::cerr << a << "\n";

    return 0;
}
