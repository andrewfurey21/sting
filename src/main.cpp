#include "../include/sting.hpp"

i32 main() {
    const std::filesystem::path file("main.sting");
    sting::vm_result result = sting::interpret(file, false);
    sting::manage_result(result); // uses exit
}
