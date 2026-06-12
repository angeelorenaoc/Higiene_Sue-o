#include "parse.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <ranges>

namespace util::parse {
    std::string errorToString(Error e){
        switch (e) {
            case Error::INVALID_VALUE: return "invalid value";
            case Error::OUT_OF_RANGE: return "number too big";
            case Error::PARTIAL: return "partial parse";
            default: return "undefined message";
        }
    }


    std::vector<std::string> split(std::string_view text, char separator){
        std::vector<std::string> parts;
        for (auto &&part : text | std::ranges::views::split(separator)) {
            parts.emplace_back(part.begin(), part.end());
        }

        return parts;
    }

}
