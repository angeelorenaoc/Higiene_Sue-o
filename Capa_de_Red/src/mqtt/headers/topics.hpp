#pragma once
#ifndef TOPICS_HPP
#define TOPICS_HPP

#include <string>
#include <fmt/format.h>

namespace mqtt {
    /*
    template<uint64_t size>
    struct topic {
        static constexpr topic BASE = "gna";

        static constexpr topic PUB = BASE/"esp32/control";
        static constexpr topic SUB = "test/topic";

        static constexpr topic HMM = PUB/SUB;

        public:
        std::array<char, size> buf{};

        constexpr topic(){}
        constexpr topic(const char(&s)[size]) {
            for (uint64_t i = 0; i < size; i++) buf[i] = s[i];
        }

        operator const char*() const { return buf.data(); }
        operator std::string() const { return buf; }

        template<uint64_t size_other>
        constexpr auto operator/(const topic<size_other>& other) const {
            topic<size + size_other - 1> result{};
            uint64_t i = 0;
            for (; i < size-1; i++) result.buf[i] = buf[i];
            result.buf[i++] = '/';
            for (uint64_t j = 0; j < size_other-1; j++) result.buf[i++] = other.buf[j];
            return result;
        }
        template<uint64_t size_other>
        constexpr auto operator/(const char(&other)[size_other]) const {
            topic<size + size_other - 1> result{};
            uint64_t i = 0;
            for (; i < size-1; i++) result.buf[i] = buf[i];
            result.buf[i++] = '/';
            for (uint64_t j = 0; j < size_other-1; j++) result.buf[i++] = other[j];
            return result;
        }
    };
    */

    struct topic {
        public:
        std::string buf = "";

        topic(){}
        topic(std::string s) : buf(s) {}
        topic(const char * s) : buf(s) {}

        operator const char*() const { return buf.data(); }
        operator std::string() const { return buf; }

        auto operator/(const topic& other) const {
            return buf + "/" + other.buf;
        }

        static const topic BASE;
        static const topic PUB;
        static const topic SUB;
        static const topic HMM;
    };

    inline const topic topic::BASE = "gna";

    inline const topic topic::PUB = BASE/"esp32/control";
    inline const topic topic::SUB = "test/topic";

    inline const topic topic::HMM = PUB/SUB;
}

template <>
struct fmt::formatter<mqtt::topic> : fmt::formatter<std::string> {
    auto format(const mqtt::topic& t, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(t.buf, ctx);
    }
};

#endif
