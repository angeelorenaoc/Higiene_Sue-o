#pragma once
#ifndef RULES_HPP
#define RULES_HPP

#include <functional>
#include <string_view>

#include "../logger/shorthands.hpp"

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

    bool compare(std::string_view cond, auto value, auto base){
        if      (cond == "over")          return value >  base;
        else if (cond == "under")         return value <  base;
        else if (cond == "equal")         return value == base;
        else if (cond == "different")     return value != base;
        else if (cond == "over_or_equal") return value >= base;
        else if (cond == "under_or_equal")return value <= base;

        RULE_ERROR("Condition '{}' not known, defaulting to false", cond);
        return false;
    }
}

#endif
