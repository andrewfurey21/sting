#include "../include/sting.hpp"

i32 main() {
    const std::filesystem::path file("main.sting");
    sting::vm_result result = sting::interpret(file, true);
    sting::manage_result(result);

    for (u64 i{}; i < sting::object_list.size(); i++) {
        delete sting::object_list.at(i);
    }
}
