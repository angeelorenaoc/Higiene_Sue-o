#pragma once
#include <string_view>
#ifndef TOPICS_HPP
#define TOPICS_HPP

#include <string>
#include <vector>
#include <spdlog/fmt/fmt.h>

#include "../../utils/parse.hpp"

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
    private:
        std::vector<std::string> sections = {};
        mutable std::string cache_;

    public:
        topic() = default;
        topic(const topic&) = default;
        topic(topic&&) = default;

        topic(std::string s) : sections{parse(s)} {}
        topic(const char* s) : sections{parse(s)} {}

        operator std::string() const {
            std::string ret{};
            for (std::string_view section : sections){
                ret += section;
                ret += "/";
            }

            if (!ret.empty()){
                ret.pop_back();
            }

            return ret;
        }
        operator const char*() const {
            cache_ = std::string(*this);
            return cache_.c_str();
        }

        std::string last() const noexcept {
            return sections.back();
        }

        topic& append(const char* section) {
            this->sections.emplace_back(section);
            return *this;
        }
        topic& append(std::string_view section) {
            return append(section.begin());
        }
        topic& append(const topic& t) {
            for (std::string_view section : t.sections){
                this->append(section);
            }

            return *this;
        }

        topic append(const char* section) const {
            topic ret = *this;
            return ret.append(section);
        }
        topic append(std::string_view section) const {
            topic ret = *this;
            return ret.append(section);
        }
        topic append(const topic& t) const {
            topic ret = *this;
            for (std::string_view section : t.sections){
                ret.append(section);
            }

            return ret;
        }

        topic operator/(const char* other) const {
            return append(other);
        }
        topic operator/(std::string_view other) const {
            return append(other);
        }
        topic operator/(const topic& other) const {
            return append(other);
        }
    private:
        static std::vector<std::string> parse(std::string t){
            return util::parse::split(t, '/');
        }
    public:

        static const topic ANY;
        static const topic ALL;

        static const topic BASE;
        static const topic ESP;

        static const topic TEST;
        static const topic CONTROL;
        static const topic INFO;

        static const topic PUB;
        static const topic SUB;
        static const topic HMM;
    };


    inline const topic topic::ANY = "+";
    inline const topic topic::ALL = "#";

    inline const topic topic::TEST = "test";
    inline const topic topic::CONTROL = "control";
    inline const topic topic::INFO = "info";

    inline const topic topic::PUB = TEST/CONTROL/"aaaa";
    inline const topic topic::SUB = TEST/CONTROL/ALL;

    inline const topic topic::HMM = PUB/SUB;
}

template <>
struct fmt::formatter<mqtt::topic> : fmt::formatter<std::string> {
    auto format(const mqtt::topic& t, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(std::string(t), ctx);
    }
};

#endif
