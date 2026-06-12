#pragma once
#include <bits/chrono.h>
#ifndef DATETIME_HPP
#define DATETIME_HPP

#include <cstdint>
#include <optional>
#include <compare>
#include <string_view>
#include <chrono>
#include <spdlog/fmt/fmt.h>

namespace dt {
    struct date {
        uint16_t year;
        uint8_t month;
        uint8_t day;

        static std::optional<date> from(std::string_view date);
        std::string to_string() const ;

        std::strong_ordering operator<=>(const date&) const = default;
        std::chrono::year_month_day to_chrono() const;
        std::chrono::days operator-(const date& other) const;
    };

    struct time {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;

        static std::optional<time> from(std::string_view time);
        std::string to_string() const;

        std::strong_ordering operator<=>(const time&) const = default;
        std::chrono::seconds to_chrono() const;
        std::chrono::seconds operator-(const time& other) const;
    };

    struct datetime { // 2026-06-17 01:07:56
        date date_value;
        time time_value;

        static std::optional<datetime> from(std::string_view datetime);
        std::string to_string() const;

        std::strong_ordering operator<=>(const datetime&) const = default;
        std::chrono::sys_seconds to_chrono() const;
        std::chrono::seconds operator-(const datetime& other) const;
    };
}

template <>
struct fmt::formatter<dt::date> : fmt::formatter<std::string> {
    auto format(const dt::date& d, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(d.to_string(), ctx);
    }
};

template <>
struct fmt::formatter<dt::time> : fmt::formatter<std::string> {
    auto format(const dt::time& t, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(t.to_string(), ctx);
    }
};

template <>
struct fmt::formatter<dt::datetime> : fmt::formatter<std::string> {
    auto format(const dt::datetime& dt, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(dt.to_string(), ctx);
    }
};

#endif
