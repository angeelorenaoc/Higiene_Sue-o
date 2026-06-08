#pragma once
#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <string>
#include <spdlog/fmt/fmt.h>

namespace db {
    enum Error {
        FAILED_OPEN_DB,

        NOT_PRESENT,
        FAILED,
    };

    std::string errorToString(Error e);
}

template <>
struct fmt::formatter<db::Error> : fmt::formatter<std::string> {
    auto format(const db::Error& e, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(db::errorToString(e), ctx);
    }
};

#endif
