#pragma once
#include "spdlog/fmt/bundled/format.h"
#include <type_traits>
#ifndef REPO_REPOSITORY_HPP
#define REPO_REPOSITORY_HPP

#include "models.hpp"
#include "../db/sqlite_db.hpp"

namespace repo {
    template<class ret_t = void>
    using expected_t = db::sqlite::expected_t<ret_t>;

    class repository {
    public:
        explicit repository(db::sqlite& database) : database(database) {}

        expected_t<int64_t>             insert_reading(std::string_view type_name, double value);
        expected_t<>                    insert_rule(int reading_type_id, int condition_type_id, int actuator_type_id, double condition_value);
        expected_t<>                    insert_actuator_log(std::string_view actuator_name, int rule_id, int reading_id, std::string_view command);

        template<class mapper_t>
        auto get_by_id(int id, std::string tablename, mapper_t&& mapper) -> expected_t<std::invoke_result_t<mapper_t, sqlite3_stmt*>> {
            std::string sql = fmt::format(
                R"(
                    SELECT *
                    FROM {}
                    WHERE id = ?
                    LIMIT 1;
                )",
                tablename
            );
            auto rows = database.query(mapper, sql, id);

            if (!rows || rows->empty()) return std::unexpected(db::Error::NOT_PRESENT);

            return rows->front();
        }

        template<class mapper_t>
        auto get_all(std::string tablename, mapper_t&& mapper) -> expected_t<std::vector<std::invoke_result_t<mapper_t, sqlite3_stmt*>>> {
            std::string sql = fmt::format(
                R"(
                    SELECT *
                    FROM {}
                    WHERE deleted_at IS NULL;
                )",
                tablename
            );
            auto rows = database.query(mapper, sql);

            if (!rows || rows->empty()) return std::unexpected(db::Error::NOT_PRESENT);

            return rows;
        }
        template<class mapper_t>
        auto get_all_and_deleted(std::string tablename, mapper_t&& mapper) -> expected_t<std::vector<std::invoke_result_t<mapper_t, sqlite3_stmt*>>> {
            std::string sql = fmt::format(
                R"(
                    SELECT *
                    FROM {}
                )",
                tablename
            );
            auto rows = database.query(mapper, sql);

            if (!rows || rows->empty()) return std::unexpected(db::Error::NOT_PRESENT);

            return rows;
        }

        // hpp
        expected_t<std::vector<reading>> get_readings(std::optional<int> type_id, std::optional<std::string> from, std::optional<std::string> to);
        expected_t<std::vector<rule>> get_all_rules();
        expected_t<> delete_rule(int id);

        expected_t<rule>                get_latest_rule(std::string_view type_name);
        expected_t<std::vector<rule>>   get_rules_for_reading(std::string_view type_name);
        expected_t<reading_type>      get_reading_type(int condition_type_id);
        expected_t<condition_type>      get_condition_type(int condition_type_id);
        expected_t<actuator_type>      get_actuator_type(int condition_type_id);

        expected_t<>                    soft_delete(std::string_view table, int id);

    private:
        db::sqlite& database;

    private:
        expected_t<int> get_type_id(std::string_view table, std::string_view name);
    };

} // namespace db

#endif
