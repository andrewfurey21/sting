#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <cassert>
#include <cmath>
#include <cstdint>
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

inline void panic_if(bool expr, const std::string& msg, const i32 code) {
    if (!expr) return;
    std::cerr << "---------------- ERROR ----------------\n" << "Code: "
        << code << "\n" << msg << "\n"
        << "---------------------------------------\n";
    // TODO: use std::terminate
    exit(code);
}

inline void panic(const std::string& msg, const i32 code = -1) {
    panic_if(true, msg, code);
}

inline std::string read_file(const std::filesystem::path& path) {
    std::ifstream f(path);

    const u64 size = std::filesystem::file_size(path);
    u8* data = (u8*)malloc(size * sizeof(u8*));

    f.read(data, size);
    std::string str(data, size);
    free(data);
    return str;
}

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

} // namespace sting

#endif
