#include "datetime.hpp"

#include <bits/chrono.h>
#include <cstdint>
#include <chrono>
#include <optional>
#include <string_view>
#include <chrono>

#include "../utils/parse.hpp"

namespace dt {

    std::optional<date> date::from(std::string_view text) {
        if (text.size() != 10) {
            return std::nullopt;
        }

        if (text[4] != '-' || text[7] != '-') {
            return std::nullopt;
        }

        auto y = util::parse::to<uint16_t>(std::string_view{text}.substr(0, 4));
        auto m = util::parse::to<uint8_t >(std::string_view{text}.substr(5, 2));
        auto d = util::parse::to<uint8_t >(std::string_view{text}.substr(8, 2));

        if (!y || !m || !d) {
            return std::nullopt;
        }

        std::chrono::year_month_day ymd{
            std::chrono::year{*y},
            std::chrono::month{*m},
            std::chrono::day{*d}
        };

        if (!ymd.ok()) {
            return std::nullopt;
        }

        return date{
            .year  = *y,
            .month = *m,
            .day   = *d
        };
    }
    std::string date::to_string() const {
        return fmt::format("{:04}-{:02}-{:02}", year, month, day);
    }
    std::chrono::year_month_day date::to_chrono() const {
        return std::chrono::year_month_day{
            std::chrono::year{year},
            std::chrono::month{month},
            std::chrono::day{day}
        };
    }
    std::chrono::days date::operator-(const date& other) const {
        return std::chrono::sys_days{to_chrono()} - std::chrono::sys_days{other.to_chrono()};
    }

    std::optional<time> time::from(std::string_view text) {
        if (text.size() < 7 || text.size() > 8) {
            return std::nullopt;
        }

        auto sections = util::parse::split(text, ':');

        if (sections.size() != 3) {
            return std::nullopt;
        }

        auto h = util::parse::to<uint8_t>(sections[0]);
        auto m = util::parse::to<uint8_t>(sections[1]);
        auto s = util::parse::to<uint8_t>(sections[2]);

        if (!h || !m || !s) {
            return std::nullopt;
        }

        if (*h > 23 || *m > 59 || *s > 59) {
            return std::nullopt;
        }

        return time{
            .hour   = *h,
            .minute = *m,
            .second = *s
        };
    }
    std::string time::to_string() const {
        return std::format("{:02}:{:02}:{:02}", hour, minute, second);
    }
    std::chrono::seconds time::to_chrono() const {
        return std::chrono::hours{hour} + std::chrono::minutes{minute} + std::chrono::seconds{second};
    }
    std::chrono::seconds time::operator-(const time& other) const {
        return to_chrono() - other.to_chrono();
    }

    std::optional<datetime> datetime::from(std::string_view text) {
        if (text.size() != 19) {
            return std::nullopt;
        }

        if (text[10] != ' ') {
            return std::nullopt;
        }

        auto d = date::from(text.substr(0, 10));
        auto t = time::from(text.substr(11, 8));

        if (!d || !t) {
            return std::nullopt;
        }

        return datetime{
            .date_value = *d,
            .time_value = *t
        };
    }
    std::string datetime::to_string() const {
        return std::format("{} {}", date_value.to_string(), time_value.to_string());
    }
    std::chrono::sys_seconds datetime::to_chrono() const {
        // chrono::year_month_day -> sys_days (days since epoch)
        std::chrono::sys_days day_point{date_value.to_chrono()};
        return day_point + time_value.to_chrono();
    }
    std::chrono::seconds datetime::operator-(const datetime& other) const {
        return to_chrono() - other.to_chrono();
    }
}
