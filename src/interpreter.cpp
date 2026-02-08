#include "interpreter.hpp"
#include "parser.hpp"

namespace sting {

dynarray<object*> object_list;

vm_result interpret(const std::filesystem::path& file, bool debug) {
    bool result;
    std::string source = read_file(file);
    if (debug) {
        std::cout << "------- SOURCE -------\n" << source
                  << "----------------------\n";
    }
    scanner scan(source.data(), source.size());
    parser p(file.string());
    result = scan.tokenize(p.get_tokens());
    if (!result) return vm_result::COMPILE_ERROR;

    const dynarray<token>& ts = p.get_tokens();

    result = p.parse();
    if (!result) return vm_result::COMPILE_ERROR;
    vmachine vm(p.get_script());

    if (debug) {
        std::cout << vm.script() << "\n";
        // NOTE: for closures, this needs to be dfs
        for (u64 i = 0; i < vm.script().constant_pool.size(); i++) {
            const value& v = vm.script().constant_pool.at(i);
            if (v.type == vtype::FUNCTION) {
                function& f = *static_cast<function*>(v.obj());
                std::cout << f.get_chunk() << "\n";
            }
        }

    }

    return vm.run_chunk();
}

void manage_result(vm_result result) {
    u8 code = -1;
    switch (result) {
        case sting::vm_result::OK: {
            code = 0;
            break;
        }
        case sting::vm_result::COMPILE_ERROR: {
            std::cerr << "Compiler error.\n";
            break;
        }
        case sting::vm_result::RUNTIME_ERROR: {
            std::cerr << "Runtime error.\n";
            break;
        }
        default:
            std::cerr << "Unknown interpreter result.\n";
    }

    for (u64 i{}; i < object_list.size(); i++) {
        delete object_list.at(i);
        object_list.at(i) = nullptr;
    }
    exit(code);
}

} // namespace sting
