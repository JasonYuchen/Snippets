#include "Helper.h"
#include <cassert>
#include <typeinfo>

enum Test {
    ONE, TWO
};

int main() 
{
    auto utype = underlyingType_FromEnum(Test::ONE);
    assert(typeid(utype) == typeid(int));
    return 0;
}