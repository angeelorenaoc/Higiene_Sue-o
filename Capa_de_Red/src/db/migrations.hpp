#pragma once
#ifndef MIGRATIONS_HPP
#define MIGRATIONS_HPP

#include "sqlite_db.hpp"

namespace db {
    sqlite::result_t<> create_schema(sqlite& db);
}

#endif
