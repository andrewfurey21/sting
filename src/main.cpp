#include "../include/sting.hpp"

i32 main() {
    // const std::filesystem::path file("main.sting");
    // sting::vm_result result = sting::interpret(file, true);
    // sting::manage_result(result);

    sting::hashmap<int, sting::string> a(5);
    a.insert(22, "Andrew");
    a.insert(21, "Liam");
    a.insert(18, "Shane");
    a.insert(18, "Matthew");
    a.insert(15, "Tristan");

    for (int i = 0; i < 23; i++) {
        if (a.contains(i)) {
            std::cout << a.at(i) << " is " << i << " years old\n";
        }
    }
}
