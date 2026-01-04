#include "../include/interpreter.hpp"

namespace sting {

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

    if (debug) {
        // int line = 0;
        // for (u64 i{}; i < ts.size(); i++) {
        //     if (ts.at(i).type == token_type::END_OF_FILE) {
        //         break;
        //     } else if (line != ts.at(i).line) {
        //         line = ts.at(i).line;
        //         std::cout << "\n" << line << ": ";
        //     }
        //     printf("token(%.*s), ", (int)ts.at(i).length, ts.at(i).start);
        // }
        // std::cout << "\n";
    }

    result = p.parse();
    if (!result) return vm_result::COMPILE_ERROR;
    vmachine vm(p.get_chunk());

    if (debug) {
        std::cout << vm.chk << "\n";
    }

    return vm.run_chunk();
}

void manage_result(vm_result result) {
    u8 code = -1;
    switch (result) {
        case sting::vm_result::OK: {
            // std::cerr << "Success.\n";
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

    // hack, when redoing need to think more carefully about gc
    for (u64 i{}; i < object_list.size(); i++) {
        delete object_list.at(i);
        object_list.at(i) = nullptr;
    }
    exit(code);
}

} // namespace sting
