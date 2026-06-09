CREATE TABLE readings (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    reading_type TEXT NOT NULL CHECK (
        reading_type IN ('temperature','humidity','light','sound','motion')
    ),
    value        REAL NOT NULL,
    created_at   DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at   DATETIME,
    deleted_at   DATETIME
);

CREATE TABLE config (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    reading_type   TEXT NOT NULL CHECK (
        reading_type IN ('temperature','humidity','light','sound','motion')
    ),
    threshold_type TEXT NOT NULL CHECK (
        threshold_type IN ('min','max','equal')
    ),
    value          REAL NOT NULL,
    created_at     DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE rules (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME,
    deleted_at DATETIME
);

CREATE TABLE actuator_log (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    actuator   TEXT NOT NULL,
    action     TEXT NOT NULL,
    config_id  INTEGER,
    rule_id    INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME,
    deleted_at DATETIME,
    FOREIGN KEY(config_id) REFERENCES config(id),
    FOREIGN KEY(rule_id)   REFERENCES rules(id)
);

CREATE INDEX idx_readings_type ON readings(reading_type);
CREATE INDEX idx_readings_deleted ON readings(deleted_at);
