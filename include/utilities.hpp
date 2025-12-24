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

} // namespace sting

#endif
