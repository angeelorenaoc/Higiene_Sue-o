#pragma once
#ifndef LOGGER_SHORTCUTS_HPP
#define LOGGER_SHORTCUTS_HPP

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

#define TLOG(tag, level, formt, ...) SPDLOG_##level("[{}] {}", tag, fmt::format(fmt::runtime(formt) __VA_OPT__(,) __VA_ARGS__))

#define MQTT_TRACE(...) TLOG("MQTT", TRACE, __VA_ARGS__)
#define MQTT_DEBUG(...) TLOG("MQTT", DEBUG, __VA_ARGS__)
#define MQTT_INFO(...) TLOG("MQTT", INFO, __VA_ARGS__)
#define MQTT_WARN(...) TLOG("MQTT", WARN, __VA_ARGS__)
#define MQTT_ERROR(...) TLOG("MQTT", ERROR, __VA_ARGS__)
#define MQTT_CRITICAL(...) TLOG("MQTT", CRITICAL, __VA_ARGS__)

#define ENV_TRACE(...) TLOG("ENV", TRACE, __VA_ARGS__)
#define ENV_DEBUG(...) TLOG("ENV", DEBUG, __VA_ARGS__)
#define ENV_INFO(...) TLOG("ENV", INFO, __VA_ARGS__)
#define ENV_WARN(...) TLOG("ENV", WARN, __VA_ARGS__)
#define ENV_ERROR(...) TLOG("ENV", ERROR, __VA_ARGS__)
#define ENV_CRITICAL(...) TLOG("ENV", CRITICAL, __VA_ARGS__)

#endif
