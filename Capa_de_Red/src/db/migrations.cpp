#include "migrations.hpp"

#include "../logger/shorthands.hpp"

namespace db {
    sqlite::result_t<> create_schema(sqlite& db) {
        DB_DEBUG("Creating schema");

        if (auto r = db.exec(R"(
            CREATE TABLE IF NOT EXISTS readings (
                id           INTEGER PRIMARY KEY AUTOINCREMENT,
                reading_type TEXT NOT NULL CHECK (
                    reading_type IN ('temperature','humidity','light','sound','motion')
                ),
                value        REAL NOT NULL,
                created_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at   DATETIME,
                deleted_at   DATETIME
            );
        )"); !r) return r;

        if (auto r = db.exec(R"(
            CREATE TABLE IF NOT EXISTS config (
                id             INTEGER PRIMARY KEY AUTOINCREMENT,
                reading_type   TEXT NOT NULL CHECK (
                    reading_type IN ('temperature','humidity','light','sound','motion')
                ),
                threshold_type TEXT NOT NULL CHECK (
                    threshold_type IN ('min','max','equal')
                ),
                value          REAL NOT NULL,
                created_at     DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )"); !r) return r;

        if (auto r = db.exec(R"(
            CREATE TABLE IF NOT EXISTS rules (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME,
                deleted_at DATETIME
            );
        )"); !r) return r;

        if (auto r = db.exec(R"(
            CREATE TABLE IF NOT EXISTS actuator_log (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                actuator   TEXT NOT NULL,
                action     TEXT NOT NULL,
                config_id  INTEGER,
                rule_id    INTEGER,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                updated_at DATETIME,
                deleted_at DATETIME,
                FOREIGN KEY(config_id) REFERENCES config(id),
                FOREIGN KEY(rule_id)   REFERENCES rules(id)
            );
        )"); !r) return r;

        if (auto r = db.exec("CREATE INDEX IF NOT EXISTS idx_readings_type ON readings(reading_type);"); !r) return r;
        if (auto r = db.exec("CREATE INDEX IF NOT EXISTS idx_readings_deleted ON readings(deleted_at);"); !r) return r;

        DB_INFO("Schema ready");
        return {};
    }

}
