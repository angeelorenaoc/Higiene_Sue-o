#pragma once
#ifndef CONFIG_ENV_VARS_HPP
#define CONFIG_ENV_VARS_HPP

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "../logger/shorthands.hpp"

namespace config {

    class dotenv {
    private:
        std::unordered_map<std::string, std::string> cache_;
    public:
        dotenv(){ load(); }
        // Load a .env file into the environment.
        // path      : path to the .env file (absolute or relative to executable)
        // overwrite : whether to overwrite already-set env vars (default: false)
        // throws    : std::runtime_error if the file cannot be opened
        bool load(const std::string& path = ".env", bool overwrite = false) {
            std::ifstream file(path);
            if (!file.is_open()){
                ENV_ERROR("Could not open file: {}", path);
                return false;
            }

            ENV_DEBUG("Reading vars on file \"{}\"", path);
            std::string line;
            int line_number = 0;
            while (std::getline(file, line)) {
                ++line_number;
                parse_line(line, overwrite);
                ENV_TRACE("Line: {}", line);
            }

            return true;
        }

        // Get a variable previously loaded into the cache.
        // Returns empty string_view if the key was not found.
        std::string get(const std::string& key) const {
            if (auto it = cache_.find(key); it != cache_.end()){
                return it->second;
            }

            ENV_ERROR("Var \"{}\" not found", key);
            return {};
        }
        std::string get_or(const std::string& key, const std::string& fallback) const {
            if (auto it = cache_.find(key); it != cache_.end()){
                return it->second;
            }

            ENV_WARN("Var \"{}\" not found, using fallback", key);
            return fallback;
        }

        // ------------------------------------------------------------------ //
        //  Typed getters                                                       //
        // ------------------------------------------------------------------ //

        int as_int(const std::string& key) const {
            auto val = get(key);
            if (val.empty())
                throw std::runtime_error("dotenv: key not found or empty: " + key);
            try { return std::stoi(val); }
            catch (...) { throw std::runtime_error("dotenv: cannot parse as int: " + key + "=" + val); }
        }
        int int_or(const std::string& key, int fallback) const {
            auto val = get_or(key, {});
            if (val.empty()) return fallback;
            try { return std::stoi(val); }
            catch (...) { return fallback; }
        }

        float as_float(const std::string& key) const {
            auto val = get(key);
            if (val.empty())
                throw std::runtime_error("dotenv: key not found or empty: " + key);
            try { return std::stof(val); }
            catch (...) { throw std::runtime_error("dotenv: cannot parse as float: " + key + "=" + val); }
        }
        float float_or(const std::string& key, float fallback) const {
            auto val = get_or(key, {});
            if (val.empty()) return fallback;
            try { return std::stof(val); }
            catch (...) { return fallback; }
        }

        // Accepts: true/false, 1/0  (case-insensitive)
        bool as_bool(const std::string& key) const {
            auto val = get(key);
            if (val.empty())
                throw std::runtime_error("dotenv: key not found or empty: " + key);
            return parse_bool(key, val);
        }
        bool bool_or(const std::string& key, bool fallback) const {
            auto val = get_or(key, {});
            if (val.empty()) return fallback;
            try { return parse_bool(key, val); }
            catch (...) { return fallback; }
        }

    private:
        // ------------------------------------------------------------------ //
        //  Parsing helpers                                                     //
        // ------------------------------------------------------------------ //

        static std::string trim(std::string_view s) {
            const std::string_view ws = " \t\r\n";
            auto start = s.find_first_not_of(ws);
            if (start == std::string_view::npos) return {};
            auto end = s.find_last_not_of(ws);
            return std::string(s.substr(start, end - start + 1));
        }
        static bool starts_with(std::string_view str, std::string_view s) {
            auto prefix = str.substr(0, s.size());
            return s == prefix;
        }

        static std::string strip_inline_comment(std::string_view s) {
            // Only strip unquoted # characters
            bool in_single = false, in_double = false;
            for (std::size_t i = 0; i < s.size(); ++i) {
                if (s[i] == '\'' && !in_double) in_single = !in_single;
                else if (s[i] == '"' && !in_single) in_double = !in_double;
                else if (s[i] == '#' && !in_single && !in_double)
                    return std::string(s.substr(0, i));
            }
            return std::string(s);
        }

        static std::string strip_quotes(std::string_view s) {
            if (s.size() >= 2 &&
            ((s.front() == '"'  && s.back() == '"') ||
                (s.front() == '\'' && s.back() == '\'')))
                return std::string(s.substr(1, s.size() - 2));
            return std::string(s);
        }

    private:
        static bool parse_bool(const std::string& key, std::string val) {
            // lowercase in-place
            std::transform(val.begin(), val.end(), val.begin(), ::tolower);
            if (val == "true"  || val == "1")  return true;
            if (val == "false" || val == "0") return false;
            throw std::runtime_error("dotenv: cannot parse as bool: " + key + "=" + val);
        }

        void parse_line(const std::string& raw_line, bool overwrite) {
            std::string line = trim(raw_line);

            // Skip blank lines and comments
            if (line.empty() || line[0] == '#'){
                return;
            }

            // Strip optional "export " prefix
            if (starts_with(line, "export ")){
                line = trim(line.substr(7));
            }

            auto eq = line.find('=');
            if (eq == std::string::npos) {
                return; // no '=', skip silently
            }

            auto key   = trim(line.substr(0, eq));
            auto value = trim(strip_inline_comment(trim(line.substr(eq + 1))));
            value = strip_quotes(value);

            if (key.empty()) return;

            ENV_DEBUG("Stored var: {} = {}", key, value);
            cache_[key] = value;
            setenv(key.c_str(), value.c_str(), overwrite ? 1 : 0);
        }

    };

    inline dotenv env;

} // namespace dotenv

#endif
