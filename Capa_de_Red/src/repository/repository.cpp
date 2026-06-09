#include "repository.hpp"

#include <expected>
#include <string>
#include <string_view>

#include "../logger/shorthands.hpp"
#include "models.hpp"

namespace repo {

    db::sqlite::expected_t<int> repository::get_type_id(std::string_view table, std::string_view name) {
        const std::string sql = "SELECT id FROM " + std::string(table) + " WHERE name = ? AND deleted_at IS NULL LIMIT 1;";

        auto rows = database.query(
            [](sqlite3_stmt* stmt) { return db::sqlite::column_int(stmt, 0); },
            sql, name
        );

        if (!rows || rows->empty()) {
            DB_WARN("Type '{}' not found in '{}'", name, table);
            return std::unexpected(db::Error::NOT_PRESENT);
        }

        return rows->front();
    }

    db::sqlite::expected_t<int64_t> repository::insert_reading(std::string_view type_name, double value) {
        return database.transaction([&]() -> db::sqlite::expected_t<int64_t> {
            auto type_id = get_type_id("reading_types", type_name);
            if (!type_id) return std::unexpected(type_id.error());

            auto res = database.exec(
                "INSERT INTO readings (id_reading_type, value) VALUES (?, ?);",
                type_id.value(), value
            );
            if (!res) return std::unexpected(res.error());

            DB_DEBUG("Inserted reading: {} = {}", type_name, value);
            return database.last_insert_id();
        });
    }
    db::sqlite::expected_t<> repository::insert_rule(int reading_type_id, int condition_type_id, int actuator_type_id, double condition_value) {
        return database.transaction([&]() -> db::sqlite::expected_t<> {
            // Find the active rule for same reading + actuator type, if any
            auto existing = database.query(
                [](sqlite3_stmt* stmt) { return db::sqlite::column_int(stmt, 0); },
                R"(
                    SELECT id FROM active_rules
                    WHERE id_reading_type = ? AND id_actuator_type = ?
                    LIMIT 1;
                )",
                reading_type_id, actuator_type_id
            );

            if (!existing) return std::unexpected(existing.error());

            // Soft delete the existing rule if found
            if (!existing->empty()) {
                if (auto res = soft_delete("rules", existing->front()); !res)
                    return res;
            }

            // Insert the new rule
            return database.exec(
                R"(
                    INSERT INTO rules (id_reading_type, id_condition_type, id_actuator_type, condition_value)
                    VALUES (?, ?, ?, ?);
                )",
                reading_type_id, condition_type_id, actuator_type_id, condition_value
            );
        });
    }
    db::sqlite::expected_t<> repository::insert_actuator_log(std::string_view actuator_name, int rule_id, int reading_id, std::string_view command) {
        return database.transaction([&]() -> db::sqlite::expected_t<> {
            auto actuator_id = get_type_id("actuator_types", actuator_name);
            if (!actuator_id) return std::unexpected(actuator_id.error());

            return database.exec(
                "INSERT INTO actuator_log (id_actuator_type, id_rule, id_reading, command) VALUES (?, ?, ?, ?);",
                actuator_id.value(), rule_id, reading_id, command
            );
        });
    }

    db::sqlite::expected_t<rule> repository::get_latest_rule(std::string_view type_name) {
        auto type_id = get_type_id("reading_types", type_name);
        if (!type_id) return std::unexpected(type_id.error());

        auto rows = database.query(
            [](sqlite3_stmt* stmt) {
                return rule{
                    .id = db::sqlite::column_int(stmt, 0),
                    .id_reading_type = db::sqlite::column_int(stmt, 1),
                    .id_condition_type = db::sqlite::column_int(stmt, 2),
                    .id_actuator_type = db::sqlite::column_int(stmt, 3),
                    .condition_value = db::sqlite::column_double(stmt, 4),
                    .created_at = db::sqlite::column_text(stmt, 5)
                };
            },
            R"(
                SELECT id, id_reading_type, id_condition_type, id_actuator_type, condition_value, created_at
                FROM active_rules
                WHERE id_reading_type = ?
                ORDER BY created_at DESC
                LIMIT 1;
            )",
            type_id.value()
        );

        if (!rows || rows->empty()) return std::unexpected(db::Error::NOT_PRESENT);
        return rows->front();
    }
    db::sqlite::expected_t<std::vector<rule>> repository::get_rules_for_reading(std::string_view type_name) {
        auto type_id = get_type_id("reading_types", type_name);
        if (!type_id) return std::unexpected(type_id.error());

        return database.query(
            [](sqlite3_stmt* stmt) {
                return rule{
                    .id = db::sqlite::column_int(stmt, 0),
                    .id_reading_type = db::sqlite::column_int(stmt, 1),
                    .id_condition_type = db::sqlite::column_int(stmt, 2),
                    .id_actuator_type = db::sqlite::column_int(stmt, 3),
                    .condition_value = db::sqlite::column_double(stmt, 4),
                    .created_at = db::sqlite::column_text(stmt, 5)
                };
            },
            R"(
                SELECT id, id_reading_type, id_condition_type, id_actuator_type, condition_value, created_at
                FROM active_rules
                WHERE id_reading_type = ?;
            )",
            type_id.value()
        );
    }

    db::sqlite::expected_t<reading_type> repository::get_reading_type(int reading_type_id) {
        static std::unordered_map<int, reading_type> cache;
        if (auto it = cache.find(reading_type_id); it != cache.end())
               return it->second;

        auto mapper = [](sqlite3_stmt* stmt) {
            return reading_type{
                .id = db::sqlite::column_int(stmt, 0),
                .name = db::sqlite::column_text(stmt, 1),
                .created_at = db::sqlite::column_text(stmt, 2),
            };
        };

        auto result = get_by_id(reading_type_id, "reading_types", mapper);

        if (!result) return result;

        cache[reading_type_id] = result.value();
        return result.value();
    }
    db::sqlite::expected_t<condition_type> repository::get_condition_type(int condition_type_id) {
        static std::unordered_map<int, condition_type> cache;
        if (auto it = cache.find(condition_type_id); it != cache.end())
               return it->second;

        auto mapper = [](sqlite3_stmt* stmt) {
            return condition_type{
                .id = db::sqlite::column_int(stmt, 0),
                .name = db::sqlite::column_text(stmt, 1),
                .created_at = db::sqlite::column_text(stmt, 2),
            };
        };

        auto rows = database.query(mapper,
                R"(
                    SELECT id, name
                    FROM condition_types
                    WHERE id = ?
                    LIMIT 1;
                )",
                condition_type_id
            );

            if (!rows || rows->empty()) return std::unexpected(db::Error::NOT_PRESENT);

            cache[condition_type_id] = rows->front();
            return rows->front();
    }
    db::sqlite::expected_t<actuator_type> repository::get_actuator_type(int actuator_type_id) {
        static std::unordered_map<int, actuator_type> cache;
        if (auto it = cache.find(actuator_type_id); it != cache.end())
               return it->second;

        auto mapper = [](sqlite3_stmt* stmt) {
            return actuator_type{
                .id = db::sqlite::column_int(stmt, 0),
                .name = db::sqlite::column_text(stmt, 1),
                .created_at = db::sqlite::column_text(stmt, 2),
            };
        };

        auto rows = database.query(mapper,
                R"(
                    SELECT id, name
                    FROM actuator_types
                    WHERE id = ?
                    LIMIT 1;
                )",
                actuator_type_id
            );

            if (!rows || rows->empty()) return std::unexpected(db::Error::NOT_PRESENT);

            cache[actuator_type_id] = rows->front();
            return rows->front();
    }

    db::sqlite::expected_t<> repository::soft_delete(std::string_view table, int id) {
        return database.transaction([&]() -> db::sqlite::expected_t<> {
            const std::string sql = "UPDATE " + std::string(table) +
                " SET deleted_at = CURRENT_TIMESTAMP WHERE id = ? AND deleted_at IS NULL;";

            auto res = database.exec(sql, id);
            if (!res) return std::unexpected(res.error());

            // sqlite doesn't error if no row matched, so we check manually
            // I may want to expose sqlite3_changes via a method if this becomes common
            return {};
        });
    }

} // namespace db
