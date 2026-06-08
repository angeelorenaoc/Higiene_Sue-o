#include "sqlite_db.hpp"

#include "../logger/shorthands.hpp"
#include "errors.hpp"
#include <expected>

namespace db {
    sqlite::~sqlite() {
        if (db_) {
            sqlite3_close(db_);
        }
    }

    sqlite::result_t<> sqlite::open(const std::string& path) {
        if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
            DB_ERROR("Failed to open database: {}", sqlite3_errmsg(db_));
            return std::unexpected(Error::FAILED_OPEN_DB);
        }

        DB_INFO("Database opened at {}", path);
        if (auto r = exec("PRAGMA foreign_keys = ON;"); !r) return r;
        if (auto r = exec("PRAGMA journal_mode = WAL;"); !r) return r;

        return {};
    }

    sqlite::result_t<> sqlite::exec(const std::string& query) {
        char* errMsg = nullptr;
        if (sqlite3_exec(db_, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string err = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            DB_ERROR("exec failed: {}", err);
            return std::unexpected(Error::FAILED);
        }

        return {};
    }

    sqlite::result_t<sqlite3_stmt*> sqlite::prepare(const std::string& query) {
        sqlite3_stmt* stmt = nullptr;
            if (sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
                std::string err = sqlite3_errmsg(db_);
                DB_ERROR("prepare failed: {}", err);
                return std::unexpected(Error::FAILED);
            }
            return {stmt};
    }
}
