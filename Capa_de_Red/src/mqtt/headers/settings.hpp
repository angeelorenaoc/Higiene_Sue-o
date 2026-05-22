#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "../../config/env_vars.hpp"

/// To change
namespace mqtt::setting {
    constexpr const auto HOST = config::mqtt::HOST;
    constexpr const auto PORT = config::mqtt::PORT;
    constexpr const auto QOS = 1;
    constexpr const auto KEEP_ALIVE = 60;
    constexpr const auto TIMEOUT = -1; // Default: -1 (1000ms)
    constexpr const auto USE_TLS = config::mqtt::USE_TLS;
}

#endif
