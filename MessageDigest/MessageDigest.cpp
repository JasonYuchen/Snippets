#include <iostream>
#include "MessageDigest.h"

int main()
{
    std::cout << MessageDigest::MD5("test MD5 algorithm").toString() << std::endl;
    return 0;
}

