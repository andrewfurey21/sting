#ifndef HASH_HPP
#define HASH_HPP

#include "utilities.hpp"

namespace sting {

const u64 DEFAULT_FNV_PRIME = 0x00000100000001B3;
const u64 DEFAULT_FNV_OFFSET = 0xCBF29CE484222325;

// u64 hash = _fnv_offset;
// for (u64 i{}; i < sizeof(key); ++i) {
//     hash ^= key_byte[i];
//     hash *= _fnv_prime;
// }

template <typename Key>
u64 fnv_1a_hash(const Key& key);

} // namespace sting

#endif
