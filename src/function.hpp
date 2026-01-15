#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "object.hpp"
#include "string.hpp"
#include "chunk.hpp"

namespace sting {

class function : public object {
public:
    function();
    function(const string& name, u64 arity);
private:
    string name;
    u64 arity;
    chunk chk;
};

} // namespace sting

#endif
