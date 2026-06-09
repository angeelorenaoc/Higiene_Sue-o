#pragma once
#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <filesystem>

namespace db {
    struct migration {
        int version;
        std::filesystem::path path;
    };

    struct reading {
        int id;
        std::string reading_type;
        double value;
        std::string created_at;
    };

    struct config {
        int id;
        std::string reading_type;
        std::string threshold_type;
        double value;
        std::string created_at;
    };

    struct actuator_log {
        int id;
        std::string actuator;
        std::string action;
        int config_id;
        int rule_id;
        std::string created_at;
    };
}

#endif
