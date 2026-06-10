#pragma once
#ifndef REPO_MODELS_HPP
#define REPO_MODELS_HPP

#include <string>
#include <sqlite3.h>
#include <nlohmann/json.hpp>

namespace repo {
    struct reading_type {
        int id;
        std::string name;
        std::string created_at;

        static reading_type db_mapper(sqlite3_stmt* s);
        nlohmann::json to_json();
    };

    struct condition_type {
        int id;
        std::string name;
        std::string created_at;

        static condition_type db_mapper(sqlite3_stmt* s);
        nlohmann::json to_json();
    };
    struct actuator_type {
        int id;
        std::string name;
        std::string created_at;

        static actuator_type db_mapper(sqlite3_stmt* s);
        nlohmann::json to_json();
    };

    struct reading {
        int id;
        int id_reading_type;
        double value;
        std::string created_at;

        static reading db_mapper(sqlite3_stmt* s);
        nlohmann::json to_json();
    };

    struct rule {
        int id;
        int id_reading_type;
        int id_condition_type;
        int id_actuator_type;
        double condition_value;
        std::string created_at;

        static rule db_mapper(sqlite3_stmt* s);
        nlohmann::json to_json();
    };

    struct actuator_log {
        int  id;
        int  id_actuator_type;
        int  id_rule;
        int  id_reading;
        std::string command;
        std::string created_at;

        static actuator_log db_mapper(sqlite3_stmt* s);
        nlohmann::json to_json();
    };
}

#endif
