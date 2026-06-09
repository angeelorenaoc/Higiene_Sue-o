#pragma once
#ifndef QUERIES_HPP
#define QUERIES_HPP

#include "sqlite_db.hpp"
#include "models.hpp"
#include <vector>

namespace db {
    sqlite::expected_t<> insert_reading(sqlite& db, const std::string& type, double value);
    sqlite::expected_t<std::vector<reading>> fetch_active_readings(sqlite& db);

    sqlite::expected_t<> insert_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type, double value);
    sqlite::expected_t<config> get_latest_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type);

    sqlite::expected_t<> insert_actuator_log(sqlite& db, const std::string& actuator, const std::string& action, int config_id, int rule_id);

    sqlite::expected_t<std::vector<actuator_log>> fetch_active_actuator_logs(sqlite& db);
    sqlite::expected_t<> soft_delete(sqlite& db, const std::string& table, int id);
}

#endif
