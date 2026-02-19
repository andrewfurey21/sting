#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "utilities.hpp"
#include "dynarray.hpp"

namespace sting {

class object {
public:
    // we can copy derived from base pointer
    virtual object *clone() const = 0;
    virtual u8 *cstr() const = 0;
    virtual ~object() = default;
};

// make a memory arena?
extern dynarray<object*> object_list;

} // namespace sting

#endif
