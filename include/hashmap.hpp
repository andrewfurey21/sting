#ifndef HASHMAP_HPP
#define HASHMAP_HPP

#include "utilities.hpp"

namespace sting {

/*
 *  Open addressing (closed hashing), linear probing hash map
 *
 *  TODO: work needs to be done to remove all the copies.
 */

const u64 DEFAULT_CAPACITY = 256;
const u64 DEFAULT_FNV_PRIME = 0x00000100000001B3;
const u64 DEFAULT_FNV_OFFSET = 0xCBF29CE484222325;
const f64 DEFAULT_GROWTH_FACTOR = 2.0;
const f64 DEFAULT_MAX_LOAD = 0.4;

template <typename Key, typename Value>
class hashmap {
public:
    hashmap() : hashmap(DEFAULT_CAPACITY) {};

    hashmap(u64 capacity) :
        _data(nullptr),
        _capacity(capacity),
        _size(0),
        _fnv_prime(DEFAULT_FNV_PRIME),
        _fnv_offset(DEFAULT_FNV_OFFSET),
        _growth_factor(DEFAULT_GROWTH_FACTOR),
        _max_load(DEFAULT_MAX_LOAD)
    {
        _data = _allocate(_capacity);
    }

    hashmap(const hashmap& other) : hashmap(other._capacity) {
        _size = other._size;
        for (u64 i{}; i < _capacity; ++i) {
            _data[i].state = other._data[i].state;

            if (_data[i].state == _slot::OCCUPIED)
                new (_data + i) _slot(other._data[i]);
        }
    }

    hashmap(hashmap&& other) {
        std::cerr << "steal!\n";
         _data = exchange(other._data, _allocate(1));
        _capacity = exchange(other._capacity, 1);
        _size = exchange(other._size, 0);
        _fnv_prime = other._fnv_prime;
        _fnv_offset = other._fnv_offset;
        _growth_factor = other._growth_factor;
        _max_load = other._max_load;
    }

    hashmap& operator=(const hashmap& other) {
        if (this != &other) {
            _capacity = other._capacity;
            _size = other._size;
            _fnv_prime = other._fnv_prime;
            _fnv_offset = other._fnv_offset;
            _growth_factor = other._growth_factor;
            _max_load = other._max_load;

            _free_data();
            _data = other->_copy_data(_capacity);
        }
        return *this;
    }

    hashmap& operator=(hashmap&& other) {
        if (this != &other) {
            _free_data();
            _data = exchange(other._data, _allocate(1));
            _capacity = exchange(other._capacity, 1);
            _size = exchange(other._size, 0);
            _fnv_prime = other._fnv_prime;
            _fnv_offset = other._fnv_offset;
            _growth_factor = other._growth_factor;
            _max_load = other._max_load;
        }
        return *this;
    }

    ~hashmap() {
        _free_data();
    }

    bool contains(const Key& key) {
        if (_size == 0) return false;
        u64 original = _hash_key(key, _capacity);
        u64 index = original;
        while (_data[index].k != key) {
            if (_data[index].state == _slot::EMPTY)
                return false;

            ++index;
            index = _cycle_index(index, _capacity);
            if (index == original)
                return false;
        }

        if (_data[index].state == _slot::DELETED)
            return false;

        return true;
    }

    // would be nice to have a move version
    void insert(const Key& key, const Value& value) {
        panic_if(1.0 < _max_load || _max_load < 0.0,
                 "sting::hashmap::contains(): _max_load must be within 0 and 1");

        if (this->contains(key)) {
            this->at(key) = value;
            return;
        }

        _grow_capacity();
        u64 index = _hash_key(key, _capacity);
        while (_data[index].state == _slot::OCCUPIED) {
            ++index;
            index = _cycle_index(index, _capacity);
            // there will always be empty space, given that _max_load < 1.0
            // no need to check for original index.
        }

        new (_data + index) _slot(key, value);
        _size++;
    }

    // panic if not contains
    Value& at(const Key& key) {
        panic_if(_size == 0ul, "sting::hashmap::at(): at on empty hashmap");
        u64 index = _hash_key(key, _capacity);
        while (_data[index].k != key) {
            panic_if(_data[index].state == _slot::EMPTY,
                     "sting::hashmap::at(): non existent key-value pair");
            ++index;
            index = _cycle_index(index, _capacity);
        }

        panic_if(_data[index].state == _slot::DELETED, "sting::hashmap::at(): key-value pair was deleted\n");
        return _data[index].v;
    }

    void remove(const Key& key) {
        panic_if(_size == 0ul, "sting::hashmap::remove(): remove on empty hashmap");

        u64 index = _hash_key(key, _capacity);
        while (_data[index].k != key) {
            panic_if(_data[index].state == _slot::EMPTY,
                     "sting::hashmap::remove(): non existent key-value pair");
            ++index;
            index = _cycle_index(index, _capacity);
        }
        _data[index].k.~Key();
        _data[index].v.~Value();
        _data[index].state = _slot::DELETED;
        _size--;
    }

    u64 capacity() { return _capacity; }
    u64 size() { return _size; }

private:
    struct _slot {
        _slot() = default;
        _slot(const Key& k, const Value& v) : state(OCCUPIED), k(k), v(v) {}
        _slot(const _slot& other) = default;

        enum : u8 {
            EMPTY,
            OCCUPIED,
            DELETED,
        } state;

        Key k;
        Value v;
    };

    static _slot* _allocate(u64 capacity) {
        return static_cast<_slot*>(calloc(capacity, sizeof(_slot)));
    }

    // produces some offset into _data, using FNV-1a
    u64 _hash_key(const Key& key, const u64 capacity) const {
        u64 hash = _fnv_offset;
        const u8* key_addr = reinterpret_cast<const u8*>(&key);
        for (u64 i{}; i < sizeof(Key); ++i) {
            hash ^= key_addr[i];
            hash *= _fnv_prime;
        }
        return hash % capacity;
    }

    void _grow_capacity() {
        if ((_size + 1.0) / ((f64)_capacity + 1e-8) < _max_load)
            return;

        u64 new_capacity = ceilf64(_capacity * _growth_factor);
        _slot* data = _copy_data(new_capacity);
        _free_data();
        _data = data;
        _capacity = new_capacity;
    }

    _slot* _copy_data(u64 new_capacity) {
        _slot* new_data = _allocate(new_capacity);
        for (u64 i{}; i < _capacity; ++i) {
            if (_data[i].state != _slot::OCCUPIED)
                continue;

            u64 index = _hash_key(_data[i].k, new_capacity);
            while (new_data[index].state == _slot::OCCUPIED) {
                ++index;
                index = _cycle_index(index, new_capacity);
            }
            new (new_data + index) _slot(_data[i].k, _data[i].v);
        }
        return new_data;
    }

    void _free_data() {
        if (_data == nullptr)
            return;

        if (_size == 0) {
            free(_data);
            _data = nullptr;
            return;
        }

        for (u64 i{}; i < _capacity; ++i) {
            if (_data[i].state == _slot::OCCUPIED) {
                _data[i].k.~Key();
                _data[i].v.~Value();
            }
        }
        free(_data);
        _data = nullptr;
    }

    static u64 _cycle_index(u64 index, u64 capacity) {
        if (index >= capacity) return 0;
        return index;
    }

    _slot* _data;
    u64 _capacity;
    u64 _size;
    u64 _fnv_prime;
    u64 _fnv_offset;
    f64 _growth_factor;
    f64 _max_load;
};

} // namespace sting

#endif
