#pragma once
#ifndef DB_ERRORS_HPP
#define DB_ERRORS_HPP

#include <string>
#include <spdlog/fmt/fmt.h>

namespace db {
    enum Error {
        FAILED_OPEN_DB,

        NOT_PRESENT,
        FAILED,
        INVALID
    };

    std::string errorToString(Error e);

    enum Result {
        DONE,
        ROW,
    };
}

template <>
struct fmt::formatter<db::Error> : fmt::formatter<std::string> {
    auto format(const db::Error& e, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(db::errorToString(e), ctx);
    }
};

#endif
