#include "models.hpp"

#include "../db/sqlite_db.hpp"

namespace repo {
    reading_type reading_type::db_mapper(sqlite3_stmt* s){
            return {
                .id = db::sqlite::column_int(s, 0),
                .name = db::sqlite::column_text(s, 1),
                .created_at = db::sqlite::column_text(s, 2)
            };
    }
    nlohmann::json reading_type::to_json() {
        return {
            {"id",                id},
            {"name",   name},
            {"created_at",        created_at}
        };
    }

    condition_type condition_type::db_mapper(sqlite3_stmt* s){
            return {
                .id = db::sqlite::column_int(s, 0),
                .name = db::sqlite::column_text(s, 1),
                .created_at = db::sqlite::column_text(s, 2)
            };
    }
    nlohmann::json condition_type::to_json() {
        return {
            {"id",                id},
            {"name",   name},
            {"created_at",        created_at}
        };
    }

    actuator_type actuator_type::db_mapper(sqlite3_stmt* s){
            return {
                .id = db::sqlite::column_int(s, 0),
                .name = db::sqlite::column_text(s, 1),
                .created_at = db::sqlite::column_text(s, 2)
            };
    }
    nlohmann::json actuator_type::to_json() {
        return {
            {"id",                id},
            {"name",   name},
            {"created_at",        created_at}
        };
    }

    reading reading::db_mapper(sqlite3_stmt* s){
            return {
                .id = db::sqlite::column_int(s, 0),
                .id_reading_type = db::sqlite::column_int(s, 1),
                .value = db::sqlite::column_double(s, 2),
                .created_at = db::sqlite::column_text(s, 2)
            };
    }
    nlohmann::json reading::to_json() {
        return {
            {"id",              id},
            {"id_reading_type", id_reading_type},
            {"value",           value},
            {"created_at",      created_at}
        };
    }

    rule rule::db_mapper(sqlite3_stmt* s){
            return {
                .id = db::sqlite::column_int(s, 0),
                .id_reading_type = db::sqlite::column_int(s, 1),
                .id_condition_type = db::sqlite::column_int(s, 2),
                .id_actuator_type = db::sqlite::column_int(s, 3),
                .condition_value = db::sqlite::column_double(s, 4),
                .created_at = db::sqlite::column_text(s, 5)
            };
    }
    nlohmann::json rule::to_json() {
        return {
            {"id",                id},
            {"id_reading_type",   id_reading_type},
            {"id_condition_type", id_condition_type},
            {"id_actuator_type",  id_actuator_type},
            {"condition_value",   condition_value},
            {"created_at",        created_at}
        };
    }

    actuator_log actuator_log::db_mapper(sqlite3_stmt* s){
            return {
                .id = db::sqlite::column_int(s, 0),
                .id_actuator_type = db::sqlite::column_int(s, 1),
                .id_rule = db::sqlite::column_int(s, 2),
                .id_reading = db::sqlite::column_int(s, 3),
                .command = db::sqlite::column_text(s, 4),
                .created_at = db::sqlite::column_text(s, 5)
            };
    }
    nlohmann::json actuator_log::to_json() {
        return {
            {"id",                  id},
            {"id_actuator_type",    id_actuator_type},
            {"id_rule",             id_rule},
            {"id_reading",          id_reading},
            {"command",             command},
            {"created_at",          created_at}
        };
    }
}
