#include "Helper.h"
#include <cassert>
#include <typeinfo>
#include <iostream>

enum Test {
    ONE, TWO
};

int main() 
{
    auto utype = underlyingType_FromEnum(Test::ONE);
    assert(typeid(utype) == typeid(int));

    Test one = Test::ONE;
    CopyOnWrite<Test> cow(one);
    cow.read([](const Test &val) { assert(val == Test::ONE); });
    cow.modify([](Test &val) { val = Test::TWO; });
    cow.read([](const Test &val) { assert(val == Test::TWO); });

    return 0;
}