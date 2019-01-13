#pragma once
#include <type_traits>
template<typename E>
constexpr auto underlyingType_FromEnum(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}