#include "rules.hpp"

#include "../logger/shorthands.hpp"

namespace rules {

    bool compare(std::string_view cond, double value, double base){
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
