-- Table for rules
CREATE TABLE time_rules (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    id_condition_type  INTEGER NOT NULL,
    id_actuator_type   INTEGER NOT NULL,
    condition_time     DATETIME NOT NULL,
    created_at         DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at         DATETIME,
    deleted_at         DATETIME,
    FOREIGN KEY (id_condition_type) REFERENCES condition_types(id),
    FOREIGN KEY (id_actuator_type) REFERENCES actuator_types(id)
);
