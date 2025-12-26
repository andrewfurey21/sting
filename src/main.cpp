#include "../include/sting.hpp"

i32 main() {
    // const std::filesystem::path file("main.sting");
    // sting::vm_result result = sting::interpret(file, true);
    // sting::manage_result(result);

    sting::string s = "hello world\n";

    sting::string x = s;
    sting::string y = sting::steal(x);
    x = y;
    y = sting::steal(x);

    std::cout << y << "\n";

    //
    // sting::string a = "abc";
    // sting::string b = "def";
    // a += b;
    // b = a + b;
    // std::cout << a.size() << ": " << a << "\n";
    // std::cout << b.size() << ": " << b << "\n";
    return 0;
}
