#ifndef DYNARRAY_HPP
#define DYNARRAY_HPP

#include "utilities.hpp"

namespace sting {

template <typename T, typename U = T>
T exchange(T& other, U&& newval) {
    T old = other;
    other = newval;
    return old;
}

template <typename T>
struct remove_reference {
    using type = T;
};

template <typename T>
struct remove_reference<T&> {
    using type = T;
};

template <typename T>
struct remove_reference<T&&> {
    using type = T;
};

// identical to std::move, but steal sounds better
// question: why do we need the remove_reference indirection?
// is decltype auto because we don't know the return type
// until we get to return?
template <typename T>
decltype(auto) steal(T&& value) {
    return static_cast<typename remove_reference<T>::type&&>(value);
}

const u64 DEFAULT_SIZE = 1 << 8;
// currently just a stack, can only push/pop.
// look into semistable::vector
template <typename T>
class dynarray {
public:

    dynarray() : dynarray(DEFAULT_SIZE) {}

    dynarray(u64 capacity) :
        _capacity(capacity),
        _size(0)
    {
        _data = allocate_capacity();
    }

    dynarray(const std::initializer_list<T>& list) :
        dynarray(list.size())
    {
        for (auto&& item : list)
            this->push_back(item);
    }

    dynarray(const dynarray& other) {
        this->_capacity = other._capacity;
        this->_size = other._size;
        _data = copy_array(other);
    }

    dynarray(dynarray&& other) {
        steal_array(sting::steal(other));
    }

    dynarray& operator=(const dynarray& other) {
        if (this != &other) {
            free_array(_data);
            _size = other._size;
            _capacity = other._capacity;
            _data = copy_array(other);
        }
        return *this;
    }

    dynarray& operator=(dynarray&& other) {
        if (this != &other) {
            steal_array(sting::steal(other));
        }
        return *this;
    }

    ~dynarray() { free_array(_data); }

    u64 size() const { return _size; }
    T* data() const { return _data; }
    u64 capacity() const { return _capacity; }

    T& at(u64 index) const {
        panic_if(index >= _size, "Index out of bounds.", -1);
        return _data[index];
    }

    void push_back(const T& x) {
        if (_capacity <= _size + 1) {
            _capacity *= 2;
            resize_array();
        }
        _size++;
        this->at(_size - 1) = x;
    }

    T pop_back() {
        T ret = this->at(_size - 1);
        _size--;
        _data[_size].~T();
        return ret;
    }

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const dynarray<U>& other);

private:

    T* allocate_capacity() {
        return static_cast<T*>(malloc(sizeof(T) * this->_capacity));
    }

    T* copy_array(const dynarray<T>& other) {
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

    void steal_array(dynarray&& other) {
        this->_capacity = exchange(other._capacity, 1);
        this->_size = exchange(other._size, 0);
        this->_data = exchange(other._data, nullptr);
    }

    u64 _capacity;
    u64 _size;
    T* _data;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const dynarray<T>& other) {
    for (u64 i{0}; i < other.size(); i++) {
        os << std::setw(4) << std::setfill('0') << i << ' ';
        os << other.at(i);
        if (i != other.size() - 1) {
            os << "\n";
        }
    }
    return os;
}

} // namespace sting

#endif
