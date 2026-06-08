#pragma once
#ifndef RULES_HPP
#define RULES_HPP

#include <functional>

namespace rules {

    template<typename ...type_t>
    using predicate_t = std::move_only_function<bool(type_t...)>;

    template<typename ...type_t>
    struct rule {
        predicate_t<type_t...> operation;

        constexpr bool operator()(type_t... args) const {
            return operation(args...);
        }
    };

    template<typename ...type_t>
    constexpr rule<type_t...> from(predicate_t<type_t...> p){
        return rule{.operation = p};
    }
}

#endif
