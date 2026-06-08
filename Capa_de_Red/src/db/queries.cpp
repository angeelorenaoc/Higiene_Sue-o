#include "queries.hpp"

#include "../logger/shorthands.hpp"
#include "models.hpp"

namespace db {
    sqlite::result_t<> insert_reading(sqlite& db, const std::string& type, double value) {
        DB_DEBUG("Inserting reading: {} = {}", type, value);

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, R"(
            INSERT INTO readings (reading_type, value) VALUES (?, ?);
        )", -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        sqlite3_bind_text(g.get(), 1, type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(g.get(), 2, value);

        if (sqlite3_step(g.get()) != SQLITE_DONE) {
            DB_ERROR("insert_reading failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        DB_DEBUG("Reading inserted");
        return {};
    }
    sqlite::result_t<std::vector<reading>> fetch_active_readings(sqlite& db) {
        DB_DEBUG("Fetching active readings");

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, R"(
            SELECT id, reading_type, value, created_at
            FROM readings
            WHERE deleted_at IS NULL;
        )", -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        std::vector<reading> rows;
        while (sqlite3_step(g.get()) == SQLITE_ROW) {
            rows.push_back({
                sqlite3_column_int(g.get(), 0),
                reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 1)),
                sqlite3_column_double(g.get(), 2),
                reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 3))
            });
        }

        DB_INFO("Fetched {} active readings", rows.size());
        return rows;
    }

    sqlite::result_t<> insert_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type, double value) {
        DB_DEBUG("Inserting config: {} {} = {}", reading_type, threshold_type, value);

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, R"(
            INSERT INTO config (reading_type, threshold_type, value) VALUES (?, ?, ?);
        )", -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        sqlite3_bind_text(g.get(), 1, reading_type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(g.get(), 2, threshold_type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(g.get(), 3, value);

        if (sqlite3_step(g.get()) != SQLITE_DONE) {
            DB_ERROR("insert_config failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        DB_DEBUG("Config inserted");
        return {};
    }
    sqlite::result_t<config> get_latest_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type) {
        DB_DEBUG("Fetching config: {} {}", reading_type, threshold_type);

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, R"(
            SELECT id, reading_type, threshold_type, value, created_at
            FROM config
            WHERE reading_type = ? AND threshold_type = ?
            ORDER BY created_at DESC
            LIMIT 1;
        )", -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        sqlite3_bind_text(g.get(), 1, reading_type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(g.get(), 2, threshold_type.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(g.get()) != SQLITE_ROW) {
            DB_WARN("No config found for {} {}", reading_type, threshold_type);
            return std::unexpected(Error::NOT_PRESENT);
        }

        config c {
            sqlite3_column_int(g.get(), 0),
            reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 2)),
            sqlite3_column_double(g.get(), 3),
            reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 4))
        };

        DB_DEBUG("Config found: {} {} = {}", c.reading_type, c.threshold_type, c.value);
        return c;
    }

    sqlite::result_t<> insert_actuator_log(sqlite& db, const std::string& actuator, const std::string& action, int config_id, int rule_id) {
        DB_DEBUG("Inserting actuator log: {} {}", actuator, action);

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, R"(
            INSERT INTO actuator_log (actuator, action, config_id, rule_id) VALUES (?, ?, ?, ?);
        )", -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        sqlite3_bind_text(g.get(), 1, actuator.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(g.get(), 2, action.c_str(),   -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(g.get(),  3, config_id);
        sqlite3_bind_int(g.get(),  4, rule_id);

        if (sqlite3_step(g.get()) != SQLITE_DONE) {
            DB_ERROR("insert_actuator_log failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        DB_DEBUG("Actuator log inserted");
        return {};
    }

    sqlite::result_t<std::vector<actuator_log>> fetch_active_actuator_logs(sqlite& db) {
        DB_DEBUG("Fetching active actuator logs");

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, R"(
            SELECT id, actuator, action, config_id, rule_id, created_at
            FROM actuator_log
            WHERE deleted_at IS NULL;
        )", -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        std::vector<actuator_log> rows;
        while (sqlite3_step(g.get()) == SQLITE_ROW) {
            rows.push_back({
                sqlite3_column_int(g.get(), 0),
                reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 1)),
                reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 2)),
                sqlite3_column_int(g.get(), 3),
                sqlite3_column_int(g.get(), 4),
                reinterpret_cast<const char*>(sqlite3_column_text(g.get(), 5))
            });
        }

        DB_INFO("Fetched {} active actuator logs", rows.size());
        return rows;
    }

    sqlite::result_t<> soft_delete(sqlite& db, const std::string& table, int id) {
        DB_DEBUG("Soft deleting id {} from {}", id, table);

        const std::string sql = "UPDATE " + table + " SET deleted_at = CURRENT_TIMESTAMP WHERE id = ? AND deleted_at IS NULL;";

        sqlite::stmt_guard g;
        if (sqlite3_prepare_v2(db.db_, sql.c_str(), -1, g.ptr(), nullptr) != SQLITE_OK) {
            DB_ERROR("prepare failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        sqlite3_bind_int(g.get(), 1, id);

        if (sqlite3_step(g.get()) != SQLITE_DONE) {
            DB_ERROR("soft_delete failed: {}", sqlite3_errmsg(db.db_));
            return std::unexpected(Error::FAILED);
        }

        if (sqlite3_changes(db.db_) == 0) {
            DB_WARN("No row affected for id {} in {}", id, table);
            return std::unexpected(Error::NOT_PRESENT);
        }

        DB_DEBUG("Soft deleted id {} from {}", id, table);
        return {};
    }

}
