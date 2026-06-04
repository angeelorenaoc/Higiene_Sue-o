#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "../../config/env_vars.hpp"

/// To change
namespace mqtt::setting {
    const auto HOST = config::env.get_or("MQTT_HOST", "localhost");
    const auto PORT = config::env.int_or("MQTT_PORT", 1883);
    const auto QOS = config::env.int_or("MQTT_QOS", 1);
    const auto KEEP_ALIVE = config::env.int_or("MQTT_KEEP_ALIVE", 60);
    const auto TIMEOUT = config::env.int_or("MQTT_TIMEOUT", -1); // Default: -1 (1000ms)
    const auto USE_TLS = config::env.bool_or("MQTT_USE_TLS", false);
    const auto USE_TLS_INSECURE = config::env.bool_or("MQTT_USE_TLS_INSECURE", true);
}

#endif
