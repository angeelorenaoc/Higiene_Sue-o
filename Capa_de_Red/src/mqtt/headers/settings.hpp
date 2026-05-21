#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

namespace mqtt::setting {
    constexpr auto HOST = "localhost";
    constexpr auto PORT = 1883;
    constexpr auto QOS = 1;
    constexpr auto KEEP_ALIVE = 60;
    constexpr auto TIMEOUT = -1; // Default: -1 (1000ms)
}

#endif
