#include "setup.hpp"

#include "../config/env.hpp"

namespace db {

    void setup(sqlite& db){
        auto dbPath = config::env.get_or("SQLITE_DB", "sweetdreams.sqlite");
        auto migrationsPath = config::env.get_or("SQLITE_MIGRATIONS", "./migrations");

        if (auto res = db.start(dbPath, migrationsPath); !res) {
            DB_CRITICAL("Database startup failed: {}", res.error());
            exit(1);
        }
    }
}
