#pragma once
#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP

#include <type_traits>

namespace util::require {
    template<typename _Tp>
    concept Numeric = std::is_integral_v<_Tp> || std::is_floating_point_v<_Tp>;

}

#endif
