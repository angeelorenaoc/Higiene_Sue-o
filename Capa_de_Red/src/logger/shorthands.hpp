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

#define RULE_TRACE(...) TLOG("RULE", TRACE, __VA_ARGS__)
#define RULE_DEBUG(...) TLOG("RULE", DEBUG, __VA_ARGS__)
#define RULE_INFO(...) TLOG("RULE", INFO, __VA_ARGS__)
#define RULE_WARN(...) TLOG("RULE", WARN, __VA_ARGS__)
#define RULE_ERROR(...) TLOG("RULE", ERROR, __VA_ARGS__)
#define RULE_CRITICAL(...) TLOG("RULE", CRITICAL, __VA_ARGS__)

#define DB_TRACE(...) TLOG("DB", TRACE, __VA_ARGS__)
#define DB_DEBUG(...) TLOG("DB", DEBUG, __VA_ARGS__)
#define DB_INFO(...) TLOG("DB", INFO, __VA_ARGS__)
#define DB_WARN(...) TLOG("DB", WARN, __VA_ARGS__)
#define DB_ERROR(...) TLOG("DB", ERROR, __VA_ARGS__)
#define DB_CRITICAL(...) TLOG("DB", CRITICAL, __VA_ARGS__)

#define HTTP_TRACE(...) TLOG("HTTP", TRACE, __VA_ARGS__)
#define HTTP_DEBUG(...) TLOG("HTTP", DEBUG, __VA_ARGS__)
#define HTTP_INFO(...) TLOG("HTTP", INFO, __VA_ARGS__)
#define HTTP_WARN(...) TLOG("HTTP", WARN, __VA_ARGS__)
#define HTTP_ERROR(...) TLOG("HTTP", ERROR, __VA_ARGS__)
#define HTTP_CRITICAL(...) TLOG("HTTP", CRITICAL, __VA_ARGS__)

#endif
