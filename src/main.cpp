#include "../include/sting.hpp"

i32 main() {
    // const std::filesystem::path file("main.sting");
    // sting::vm_result result = sting::interpret(file, true);
    // sting::manage_result(result);

    sting::hashmap<int, int> a(5);
    for (int i = 0; i < 100; i++) {
        a.insert(i, i+1);
    }

    for (int i = 0; i < 100; i++) {
        std::cerr << a.at(i) << ", ";
    }
    std::cerr << "\n";

    sting::hashmap<int, int> b = a;
    for (int i = 0; i < 100; i++) {
        if (a.contains(i))
            std::cerr << b.at(i) << ", ";
    }

    std::cerr << "\n" << std::flush;

    for (int i = 0; i < 50; i++) {
        if (b.contains(i))
            b.remove(i);
    }

    sting::hashmap<int, int> c = sting::steal(b);
    // assert(b.size() == 0);
    // std::cerr << c.size() << "\n";
    // assert(c.size() == 50);

    for (int i = 0; i < 100; i+=2) {
        if (c.contains(i))
            std::cerr << c.at(i) << ", ";
    }
    std::cerr << "\n";


    for (int i = 0; i < 256; i += 3) {
        b.insert(i, i / 3);
        std::cerr << i << ": " << b.at(i) << ", ";
    }
    std::cerr << "\n";

    b = sting::steal(c);
    for (int i = 0; i < 100; i+=2) {
        if (b.contains(i))
            std::cerr << b.at(i) << ", ";
    }
    std::cerr << "\n";
    assert(c.size() == 0);
}
