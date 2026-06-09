#include "sqlite_db.hpp"

#include <expected>
#include <fstream>
#include <functional>
#include <string_view>
#include <spdlog/fmt/fmt.h>
#include <sqlite3.h>


#include "errors.hpp"

#include "../logger/shorthands.hpp"
#include "../utils/parse.hpp"
#include "../utils/crypto.hpp"

namespace db {
    sqlite::~sqlite() {
        if (handle) {
            sqlite3_close(handle);
        }
    }

    /// Initializers

    sqlite::expected_t<> sqlite::open(std::string_view path) {
        if (sqlite3_open(path.data(), &handle) != SQLITE_OK) {
            DB_ERROR("Failed to open database: {}", sqlite3_errmsg(handle));
            return std::unexpected(Error::FAILED_OPEN_DB);
        }

        DB_INFO("Database opened at {}", path);
        if (auto r = raw_exec("PRAGMA foreign_keys = ON;"); !r) return r;
        if (auto r = raw_exec("PRAGMA journal_mode = WAL;"); !r) return r;

        return {};
    }
    sqlite::expected_t<> sqlite::start(std::string_view dbPath, std::string_view migrationsPath) {
        if(auto res = open(dbPath); !res){
            return res;
        }

        if (auto res = run_migrations(migrationsPath); !res){
            return res;
        }

        return {};
    }

    /// Database Operations
    template<>
    sqlite::expected_t<> sqlite::exec(std::string_view query) {
        return raw_exec(query);
    }

    /// Column specifiers

    int sqlite::column_int(sqlite3_stmt* stmt, int column){
        return sqlite3_column_int(stmt, column);
    }
    int64_t sqlite::column_int64(sqlite3_stmt* stmt, int column){
        return sqlite3_column_int64(stmt, column);
    }
    double sqlite::column_double(sqlite3_stmt* stmt, int column){
        return sqlite3_column_double(stmt, column);
    }
    std::string sqlite::column_text(sqlite3_stmt* stmt, int column) {
        auto ptr = sqlite3_column_text(stmt, column);

        return ptr ? reinterpret_cast<const char*>(ptr) : "";
    }

    /// Database internal operations

    sqlite::expected_t<> sqlite::raw_exec(std::string_view query) {
        char* errMsg = nullptr;
        if (sqlite3_exec(handle, query.data(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string err = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            DB_ERROR("exec failed: {}", err);
            return std::unexpected(Error::FAILED);
        }

        return {};
    }

    sqlite::expected_t<sqlite3_stmt*> sqlite::prepare(std::string_view query) {
        sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(handle, query.data(), -1, &stmt, nullptr) != SQLITE_OK) {
                std::string err = sqlite3_errmsg(handle);
                DB_ERROR("prepare failed: {}", err);
                return std::unexpected(Error::FAILED);
            }
            return {stmt};
    }


    sqlite::expected_t<> sqlite::bind_at(sqlite3_stmt* stmt, int idx, int value) {
        if (sqlite3_bind_int(stmt, idx, value) != SQLITE_OK) return std::unexpected(Error::FAILED);
        return {};
    }
    sqlite::expected_t<> sqlite::bind_at(sqlite3_stmt* stmt, int idx, int64_t value) {
        if (sqlite3_bind_int64(stmt, idx, value) != SQLITE_OK) return std::unexpected(Error::FAILED);
        return {};
    }
    sqlite::expected_t<> sqlite::bind_at(sqlite3_stmt* stmt, int idx, double value) {
        if (sqlite3_bind_double(stmt, idx, value) != SQLITE_OK) return std::unexpected(Error::FAILED);
        return {};
    }
    sqlite::expected_t<> sqlite::bind_at(sqlite3_stmt* stmt, int idx, float value) {
        if (sqlite3_bind_double(stmt, idx, value) != SQLITE_OK) return std::unexpected(Error::FAILED);
        return {};
    }
    sqlite::expected_t<> sqlite::bind_at(sqlite3_stmt* stmt, int idx, std::string_view value) {
        if (sqlite3_bind_text(stmt, idx, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT) != SQLITE_OK){
            return std::unexpected(Error::FAILED);
        }
        return {};
    }
    sqlite::expected_t<> sqlite::bind_at(sqlite3_stmt* stmt, int idx, std::nullptr_t) {
        if (sqlite3_bind_null(stmt, idx) != SQLITE_OK) return std::unexpected(Error::FAILED);
        return {};
    }

    sqlite::expected_t<Result> sqlite::step(sqlite3_stmt* stmt) {
        switch (auto rc = sqlite3_step(stmt)) {
            case SQLITE_ROW:
                return Result::ROW;

            case SQLITE_DONE:
                return Result::DONE;

            default:
                return std::unexpected(Error::FAILED);
        }
    }

    /// Migrations

    sqlite::expected_t<> sqlite::create_migration_table() {
        auto sql = R"(
            CREATE TABLE IF NOT EXISTS schema_migrations (
                version INTEGER PRIMARY KEY,
                filename TEXT NOT NULL,
                checksum TEXT NOT NULL,
                applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
            );
        )";

        return transaction([&] { return exec(sql); });
    }
    std::unordered_set<int> sqlite::get_applied_versions() {
        std::unordered_set<int> versions;

        auto result = query(std::bind_back(column_int, 0),
            "SELECT version FROM schema_migrations"
        );

        if (!result){
            return versions;
        }

        for (auto version : result.value()){
            versions.insert(version);
        }

        return versions;
    }
    sqlite::expected_t<std::string> sqlite::load_file(const std::filesystem::path& path) {
        std::ifstream file(path);

        if (!file) {
            DB_ERROR("Failed to open migration: {}", path.string());
            return std::unexpected(Error::FAILED_OPEN_DB);
        }

        return std::string{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };
    }

    sqlite::expected_t<> sqlite::apply_migration(const migration& migration) {
        return transaction([&]() -> expected_t<> {
            if (auto sql = load_file(migration.path); !sql){
                return std::unexpected(sql.error());
            }
            else if (auto r = exec(sql.value()); !r){
                return r;
            }

            auto checksum = compute_checksum(migration.path);
            if (!checksum){
                return std::unexpected(checksum.error());
            }

            return exec(
                R"(
                INSERT INTO schema_migrations
                (version, filename, checksum)
                VALUES (?, ?, ?)
                )",
                migration.version,
                migration.path.filename().string(),
                checksum.value()
            );
        });
    }
    std::vector<migration> sqlite::find_migrations(const std::filesystem::path& path) {
        std::vector<migration> migrations;

        for (auto const& entry : std::filesystem::directory_iterator(path)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".sql")
                continue;

            auto name = entry.path().filename().string();
            auto pos = name.find('_');

            if (pos == std::string::npos){
                continue;
            }

            auto parseRes = util::parse::to<int>(name.substr(0, pos));
            if (!parseRes){
                continue;
            }

            int version = parseRes.value();

            migrations.push_back({
                version,
                entry.path()
            });
        }

        std::sort(
            migrations.begin(),
            migrations.end(),
            [](auto const& a, auto const& b) { return a.version < b.version; }
        );

        return migrations;
    }

    sqlite::expected_t<> sqlite::run_migrations(const std::filesystem::path& path) {
        DB_DEBUG("Creating migrations table if needed");
        if (auto res = create_migration_table(); !res) {
            DB_CRITICAL("Failed to create migrations table");
            return res;
        }

        DB_DEBUG("Verifying migrations on {}", path.string());
        if (auto r = verify_migrations(path); !r) {
            DB_ERROR("Failed migrations check");
            return r;
        }

        DB_DEBUG("Migrations verified successfuly");
        DB_INFO("Starting migrations");

        auto applied = get_applied_versions();
        auto migrations = find_migrations(path);

        for (auto const& migration : migrations) {
            if (applied.contains(migration.version)){
                continue;
            }

            DB_INFO("Applying migration {}", migration.path.string());

            if (auto r = apply_migration(migration); !r) {
                DB_ERROR("Failed migration application");
                return r;
            }
        }

        return {};
    }

    /// Integrity verification

    sqlite::expected_t<std::string> sqlite::compute_checksum(const std::filesystem::path& path) {
        auto sql = load_file(path);

        if (!sql){
            return std::unexpected(sql.error());
        }

        return util::crypto::sha256(sql.value());
    }
    sqlite::expected_t<> sqlite::verify_migrations(const std::filesystem::path& path) {
        auto rows = query([](sqlite3_stmt* stmt) {
                return std::tuple{
                    sqlite3_column_int(stmt, 0),
                    column_text(stmt, 1),
                    column_text(stmt, 2)
                };
            },
            R"(
                SELECT version, filename, checksum
                FROM schema_migrations
            )"
        );

        if (!rows){
            return {};
        }

        for (auto const& [version, filename, stored] : rows.value()) {
            auto current = compute_checksum(path / filename);

            if (!current){
                return std::unexpected(current.error());
            }

            if (current.value() != stored) {
                DB_ERROR("Migration {} was modified after application", filename);
                return std::unexpected(Error::FAILED);
            }
        }

        return {};
    }
}
