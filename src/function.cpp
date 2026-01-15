#include "function.hpp"

namespace sting {

function::function() : function("unnamed_func", 0) {}

function::function(const string& name, u64 arity) :
    name(name),
    arity(arity)
{}

} // namespace sting
