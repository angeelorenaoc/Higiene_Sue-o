#pragma once
#ifndef REPO_MODELS_HPP
#define REPO_MODELS_HPP

#include <string>

namespace repo {
    struct reading_type {
        int id;
        std::string name;
        std::string created_at;
    };
    struct condition_type {
        int id;
        std::string name;
        std::string created_at;
    };
    struct actuator_type {
        int id;
        std::string name;
        std::string created_at;
    };

    struct reading {
        int id;
        int id_reading_type;
        double value;
        std::string created_at;
    };

    struct rule {
        int id;
        int id_reading_type;
        int id_condition_type;
        int id_actuator_type;
        double condition_value;
        std::string created_at;
    };

    struct actuator_log {
        int  id;
        int  id_actuator_type;
        int  id_rule;
        int  id_reading;
        std::string command;
        std::string created_at;
    };
}

#endif
