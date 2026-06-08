#pragma once

#ifndef SQLITE_DB_HPP
#define SQLITE_DB_HPP

#include <sqlite3.h>
#include <string>
#include <expected>
#include <vector>

#include "errors.hpp"
#include "models.hpp"

namespace db {
    class sqlite {
    public:
        template<class ret_t = void>
        using result_t = std::expected<ret_t, Error>;

        struct stmt_guard {
            sqlite3_stmt* stmt = nullptr;
            ~stmt_guard() { if (stmt) sqlite3_finalize(stmt); }
            sqlite3_stmt* get() { return stmt; }
            sqlite3_stmt** ptr() { return &stmt; }
        };

    public:
        sqlite() = default;
        ~sqlite();

        sqlite(const sqlite&) = delete;
        sqlite& operator=(const sqlite&) = delete;

        result_t<> open(const std::string& path);

    private:
        sqlite3* db_ = nullptr;

        result_t<> exec(const std::string& query);
        result_t<sqlite3_stmt*> prepare(const std::string& query);

        friend result_t<> create_schema(sqlite& db);

        friend result_t<> insert_reading(sqlite& db, const std::string& type, double value);
        friend result_t<> insert_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type, double value);
        friend result_t<config> get_latest_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type);
        friend result_t<std::vector<reading>> fetch_active_readings(sqlite& db);
        friend result_t<> insert_actuator_log(sqlite& db, const std::string& actuator, const std::string& action, int config_id, int rule_id);
        friend result_t<std::vector<actuator_log>> fetch_active_actuator_logs(sqlite& db);
        friend result_t<> soft_delete(sqlite& db, const std::string& table, int id);

    };
}


#endif
