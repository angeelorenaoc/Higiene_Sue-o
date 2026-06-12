#include "spdlog/common.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "../config/env.hpp"

namespace logger {
    void setup() {
        //spdlog::set_pattern("[%Y-%m-%d %T] [%^%l%$] [%s:%#] %v"); // [pid:%P | tid:%t]

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(config::env.enum_or("SPD_CONSOLE_LOG_LEVEL", spdlog::level::trace));
        console_sink->set_pattern("[%d/%m/%Y %T] [%^%l%$] [%s:%#] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("./sleep_monitor.log", true);
        file_sink->set_level(config::env.enum_or("SPD_FILE_LOG_LEVEL", spdlog::level::trace));
        file_sink->set_pattern("[%d/%m/%Y %T] [%l] [%s:%#] [%!] [pid:%P | tid:%t] %v");

        auto logger = std::make_shared<spdlog::logger>("main", spdlog::sinks_init_list{console_sink, file_sink});
        logger->set_level(spdlog::level::trace); // logger itself must allow all levels through

        spdlog::set_default_logger(logger);
        spdlog::flush_every(std::chrono::seconds(200));
    }

    void shutdown(){
        spdlog::shutdown();
    }
}
