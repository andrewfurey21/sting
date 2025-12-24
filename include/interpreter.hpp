#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "vmachine.hpp"
#include "parser.hpp"

namespace sting {

vm_result interpret(const std::filesystem::path& file, bool debug = true);
void manage_result(vm_result result);

}

#endif
