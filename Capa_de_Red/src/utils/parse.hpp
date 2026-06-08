#pragma once
#include <vector>
#ifndef PARSE_HPP
#define PARSE_HPP

#include <expected>
#include <string>
#include <charconv>
#include <spdlog/fmt/fmt.h>
#include "constrains.hpp"

namespace util::parse {
    enum Error {
        INVALID_VALUE,
        OUT_OF_RANGE,
        PARTIAL,
    };

    std::string errorToString(Error e);

    template<require::Numeric type_t>
    std::expected<type_t, Error> to(std::string_view s){
        type_t value;
        auto [ptr, errorCode] = std::from_chars(
            s.data(),
            s.data() + s.size(),
            value
        );

        if (errorCode != std::errc()){
            if (errorCode == std::errc::invalid_argument){
                return std::unexpected(Error::INVALID_VALUE);
            }
            else if (errorCode == std::errc::result_out_of_range){
                return std::unexpected(Error::OUT_OF_RANGE);
            }
        } else {
            if (ptr != s.data() + s.size()){
                return std::unexpected(Error::PARTIAL);
            }
        }

        return value;
    }

    std::vector<std::string> split(std::string text, char separator);
}

template <>
struct fmt::formatter<util::parse::Error> : fmt::formatter<std::string> {
    auto format(const util::parse::Error& e, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(util::parse::errorToString(e), ctx);
    }
};


#endif
