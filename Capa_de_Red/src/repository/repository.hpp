#pragma once
#include "spdlog/fmt/bundled/format.h"
#include <type_traits>
#ifndef REPO_REPOSITORY_HPP
#define REPO_REPOSITORY_HPP

#include "models.hpp"
#include "../db/sqlite_db.hpp"

namespace repo {

    class repository {
    public:
        explicit repository(db::sqlite& database) : database(database) {}

        db::sqlite::expected_t<int64_t>             insert_reading(std::string_view type_name, double value);
        db::sqlite::expected_t<>                    insert_rule(int reading_type_id, int condition_type_id, int actuator_type_id, double condition_value);
        db::sqlite::expected_t<>                    insert_actuator_log(std::string_view actuator_name, int rule_id, int reading_id, std::string_view command);

        template<class mapper_t>
        auto get_by_id(int id, std::string tablename, mapper_t&& mapper) -> db::sqlite::expected_t<std::invoke_result_t<mapper_t, sqlite3_stmt*>> {
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

        db::sqlite::expected_t<rule>                get_latest_rule(std::string_view type_name);
        db::sqlite::expected_t<std::vector<rule>>   get_rules_for_reading(std::string_view type_name);
        db::sqlite::expected_t<reading_type>      get_reading_type(int condition_type_id);
        db::sqlite::expected_t<condition_type>      get_condition_type(int condition_type_id);
        db::sqlite::expected_t<actuator_type>      get_actuator_type(int condition_type_id);

        db::sqlite::expected_t<>                    soft_delete(std::string_view table, int id);

    private:
        db::sqlite& database;

    private:
        db::sqlite::expected_t<int> get_type_id(std::string_view table, std::string_view name);
    };

} // namespace db

#endif
