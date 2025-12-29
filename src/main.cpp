#include "../include/sting.hpp"

i32 main() {
    // const std::filesystem::path file("main.sting");
    // sting::vm_result result = sting::interpret(file, true);
    // sting::manage_result(result);

    sting::hashmap<int, int> a(5);
    int num = 100;
    for (int i = 0; i < num; i++) {
        a.insert(i, i + 1);
    }
    assert(a.size() == 100);

    std::cerr << "Should say 1 -> 100\n";
    for (int i = 0; i < num; i++) {
        std::cerr << a.at(i) << ", ";
    }
    std::cerr << "\n";


    sting::hashmap<int, int> b = a;
    for (int i = 0; i < 50; i++) {
        if (b.contains(i))
            b.remove(i);
    }

    std::cerr << "Should say 50 -> 100\n";
    for (int i = 0; i < 100; i++) {
        if (b.contains(i))
            std::cerr << b.at(i) << ", ";
    }

    std::cerr << "\n" << std::flush;

    std::cerr << "Should be a steal: ";
    sting::hashmap<int, int> c = sting::steal(b);
    assert(b.size() == 0);
    assert(c.size() == 50);

    std::cerr << "Should say even 50 -> 100\n";
    std::cerr << "Size: " << c.size() << "\t Capacity: " << c.capacity() << "\n";
    for (int i = 0; i < 100; i++) {
        if (c.contains(i))
            std::cerr << c.at(i) << ", ";
    }
    std::cerr << "\n";

    std::cerr << "Should say 0-256, i: i/3\n";
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
