#pragma once
#ifndef HTTP_SETTINGS_HPP
#define HTTP_SETTINGS_HPP

#include "../../config/env.hpp"

namespace http {
    const auto HOST = config::env.get_or("HTTP_HOST", "localhost");
    const auto PORT = config::env.int_or("HTTP_PORT", 8000);
    const auto CERTS_PATH = config::env.get("MQTT_CERTS_PATH");
}

#endif
