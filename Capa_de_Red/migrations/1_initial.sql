-- Table for reading types
CREATE TABLE reading_types (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL UNIQUE,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME,
    deleted_at  DATETIME
);

-- Table for condition types
CREATE TABLE condition_types (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL UNIQUE,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME,
    deleted_at  DATETIME
);

-- Table for actuator types
CREATE TABLE actuator_types (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL UNIQUE,
    created_at  DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at  DATETIME,
    deleted_at  DATETIME
);

-- Table for readings
CREATE TABLE readings (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reading_type  INTEGER NOT NULL,
    value            REAL NOT NULL,
    created_at       DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at       DATETIME,
    deleted_at       DATETIME,
    FOREIGN KEY (id_reading_type) REFERENCES reading_types(id)
);

-- Table for rules
CREATE TABLE rules (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    id_reading_type    INTEGER NOT NULL,
    id_condition_type  INTEGER NOT NULL,
    id_actuator_type   INTEGER NOT NULL,
    condition_value    REAL NOT NULL,
    created_at         DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at         DATETIME,
    deleted_at         DATETIME,
    FOREIGN KEY (id_reading_type) REFERENCES reading_types(id),
    FOREIGN KEY (id_condition_type) REFERENCES condition_types(id),
    FOREIGN KEY (id_actuator_type) REFERENCES actuator_types(id)
);

-- Table for actuator logs
CREATE TABLE actuator_log (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    id_actuator_type  INTEGER NOT NULL,
    id_rule           INTEGER NOT NULL,
    id_reading        INTEGER NOT NULL,
    command           TEXT NOT NULL,
    created_at        DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at        DATETIME,
    deleted_at        DATETIME,
    FOREIGN KEY (id_actuator_type) REFERENCES actuator_types(id),
    FOREIGN KEY (id_rule) REFERENCES rules(id),
    FOREIGN KEY (id_reading) REFERENCES readings(id)
);

-- Indexes for performance
CREATE INDEX idx_readings_type ON readings(id_reading_type);
CREATE INDEX idx_readings_deleted ON readings(deleted_at);
CREATE INDEX idx_rules_deleted ON rules(deleted_at);
CREATE INDEX idx_actuator_log_rule ON actuator_log(id_rule);
CREATE INDEX idx_actuator_log_reading ON actuator_log(id_reading);
CREATE INDEX idx_actuator_log_deleted ON actuator_log(deleted_at);

-- Views

CREATE VIEW active_rules AS
SELECT *
FROM rules
WHERE deleted_at IS NULL;
