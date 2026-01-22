#ifndef DYNARRAY_HPP
#define DYNARRAY_HPP

#include "utilities.hpp"

namespace sting {

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
        _data = allocate(capacity);
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
        steal_array(sting::stealable(other));
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
            steal_array(sting::stealable(other));
        }
        return *this;
    }

    ~dynarray() { free_array(_data); }

    u64 size() const { return _size; }
    T* data() const { return _data; }
    u64 capacity() const { return _capacity; }

    T& at(u64 index) const {
        std::ostringstream err;
        err << "dynarray::at(): Index (" << index << ") out of bounds " << _size << ".";
        panic_if(index >= _size, err.str(), -1);
        return _data[index];
    }

    void push_back(const T& x) {
        if (_capacity <= _size + 1) {
            _capacity *= 2;
            resize_array();
        }
        new (_data + _size) T(x);
        _size++;
    }

    T pop_back() {
        panic_if(_size == 0, "dynarray::pop_back(): cannot pop_back on array of size 0");
        T ret = this->at(_size - 1);
        _size--;
        _data[_size].~T();
        return ret;
    }

    T& back() const {
        panic_if(_size <= 0, "dynarray::back(): cannot get back of array of size 0");
        return _data[_size - 1];
    }

    template<typename U>
    friend std::ostream& operator<<(std::ostream& os, const dynarray<U>& other);

private:

    T* allocate(u64 capacity) {
        T* d = static_cast<T*>(malloc(sizeof(T) * _capacity));
        return d;
    }

    T* copy_array(const dynarray<T>& other) {
        T* data = allocate(other._capacity);
        for (u64 i{}; i < this->size(); i++) {
            new (data + i) T(other.at(i));
        }
        return data;
    }

    void resize_array() {
        T* new_data = copy_array(*this);
        free_array(_data);
        _data = new_data;
    }

    void free_array(T* data) {
        for (u64 i{}; i < this->size(); i++) {
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
    for (u64 i{}; i < other.size(); i++) {
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
