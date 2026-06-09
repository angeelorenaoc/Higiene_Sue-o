#pragma once

#ifndef SQLITE_DB_HPP
#define SQLITE_DB_HPP

#include <cstdint>
#include <type_traits>
#include <string_view>
#include <string>
#include <expected>
#include <vector>
#include <unordered_set>
#include <sqlite3.h>

#include "errors.hpp"
#include "models.hpp"

#include "../logger/shorthands.hpp"

namespace db {
    class sqlite {
    public:
        template<class ret_t = void>
        using expected_t = std::expected<ret_t, Error>;

        //template<class result_t>
        //using mapping_t = std::move_only_function<result_t(sqlite3_stmt *)>;

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

        /// Initializers

        expected_t<> open(std::string_view path);
        expected_t<> start(std::string_view path, std::string_view migrationsPath);

        /// Database Operations

        template<class callable_t>
        auto transaction(callable_t&& queries) -> std::invoke_result_t<callable_t> {
            if (auto beginRes = raw_exec("BEGIN"); !beginRes){
                return std::unexpected(beginRes.error());
            }

            auto result = std::forward<callable_t>(queries)();
            if (!result) {
                auto _ = raw_exec("ROLLBACK");
                return result;
            }

            if (auto commitRes = raw_exec("COMMIT"); !commitRes) {
                auto _ = raw_exec("ROLLBACK");
                return std::unexpected(commitRes.error());
            }

            return result;
        }

        template<typename... Args>
        expected_t<> exec(std::string_view query, Args&&... args) {
            return prep_exec_no_return(query, std::forward<Args>(args)...);
        }
        template<class mapper_t, typename... Args>
        auto query(mapper_t&& mapper, std::string_view query, Args&&... args) {
            using result_t = std::invoke_result_t<mapper_t, sqlite3_stmt*>;
            return prep_exec<result_t>(query, std::forward<mapper_t>(mapper), std::forward<Args>(args)...);
        }

        /// Column specifiers

        static int column_int(sqlite3_stmt* stmt, int column);
        static int64_t column_int64(sqlite3_stmt* stmt, int column);
        static double column_double(sqlite3_stmt* stmt, int column);
        static std::string column_text(sqlite3_stmt* stmt, int column);

    private:
        sqlite3* handle = nullptr;

    private:

        /// Database internal operations

        expected_t<> raw_exec(std::string_view query);
        template<class result_t, class mapper_t>
        expected_t<std::vector<result_t>> prep_exec(std::string_view query, mapper_t&& mapper, auto&&... args){
            DB_DEBUG("Preparing exec");
            sqlite::stmt_guard guard;

            if (auto res = prepare(query); !res){
                return std::unexpected(res.error());
            } else {
                guard.stmt = res.value();
            }

            if(auto res = bind(guard.get(), args...); !res){
                return std::unexpected(res.error());
            }

            std::vector<result_t> readings = {};

            while (true){
                auto res = step(guard.get());
                if (!res) {
                    DB_ERROR("Query failed: {}", sqlite3_errmsg(handle));
                    return std::unexpected(Error::FAILED);
                }

                if (auto value = res.value(); value == DONE){
                    break;
                }
                else if (value == ROW){
                    if constexpr (!std::is_void_v<result_t>) {
                        readings.push_back(mapper(guard.get()));
                    }
                }
            }

            DB_DEBUG("Query succesful");
            return readings;
        }
        expected_t<> prep_exec_no_return(std::string_view query, auto&&... args){
            DB_DEBUG("Preparing exec no return");
            sqlite::stmt_guard guard;

            if (auto res = prepare(query); !res){
                return std::unexpected(res.error());
            } else {
                guard.stmt = res.value();
            }

            if(auto res = bind(guard.get(), args...); !res){
                return std::unexpected(res.error());
            }

            while (true){
                auto res = step(guard.get());
                if (!res) {
                    DB_ERROR("Query failed: {}", sqlite3_errmsg(handle));
                    return std::unexpected(Error::FAILED);
                }
                else if (res.value() == DONE) {
                    break;
                }
            }

            DB_DEBUG("Query succesful");
            return {};
        }

        expected_t<sqlite3_stmt*> prepare(std::string_view query);

        template<typename ...type_t>
        expected_t<> bind(sqlite3_stmt* stmt, type_t&&... values) {
            int idx = 1;
            expected_t<> r{};

            ((r = bind_at(stmt, idx++, std::forward<type_t>(values))) && ...);

            return r;
        }

        expected_t<> bind_at(sqlite3_stmt* stmt, int idx, int value);
        expected_t<> bind_at(sqlite3_stmt* stmt, int idx, int64_t value);
        expected_t<> bind_at(sqlite3_stmt* stmt, int idx, double value);
        expected_t<> bind_at(sqlite3_stmt* stmt, int idx, float value);
        expected_t<> bind_at(sqlite3_stmt* stmt, int idx, std::string_view value);
        expected_t<> bind_at(sqlite3_stmt* stmt, int idx, std::nullptr_t);

        expected_t<Result> step(sqlite3_stmt* stmt);

        /// Migrations

        expected_t<> create_migration_table();
        std::unordered_set<int> get_applied_versions();

        expected_t<std::string> load_file(const std::filesystem::path& path);
        expected_t<> apply_migration(const migration& migration);
        std::vector<migration> find_migrations(const std::filesystem::path& path);
        expected_t<> run_migrations(const std::filesystem::path& path);

        /// Integrity verification

        expected_t<std::string> compute_checksum(const std::filesystem::path& path);
        expected_t<> verify_migrations(const std::filesystem::path& path);

        /// Others
        friend expected_t<> create_schema(sqlite& db);

        friend expected_t<> insert_reading(sqlite& db, const std::string& type, double value);
        friend expected_t<> insert_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type, double value);
        friend expected_t<config> get_latest_config(sqlite& db, const std::string& reading_type, const std::string& threshold_type);
        friend expected_t<std::vector<reading>> fetch_active_readings(sqlite& db);
        friend expected_t<> insert_actuator_log(sqlite& db, const std::string& actuator, const std::string& action, int config_id, int rule_id);
        friend expected_t<std::vector<actuator_log>> fetch_active_actuator_logs(sqlite& db);
        friend expected_t<> soft_delete(sqlite& db, const std::string& table, int id);

    };
}


#endif
