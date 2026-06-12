cmake_minimum_required(VERSION 3.23)
project(sleep_monitor)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(src/vendor/spdlog)

add_executable(sleep_monitor
    src/main.cpp
    src/logger/setup.cpp
    src/db/sqlite_db.cpp
    src/db/errors.cpp
    src/db/setup.cpp
    src/mqtt/client.cpp
    src/mqtt/setup.cpp
    src/http/server.cpp
    src/http/setup.cpp
    src/repository/repository.cpp
    src/repository/models.cpp
    src/datetime/datetime.cpp
    src/utils/parse.cpp
    src/utils/crypto.cpp
)

target_link_libraries(sleep_monitor
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
    mosquitto
    spdlog::spdlog
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_compile_definitions(sleep_monitor
    PRIVATE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
)

target_include_directories(sleep_monitor PRIVATE vendor)

add_custom_command(
    TARGET sleep_monitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE_DIR:sleep_monitor>/.env
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/.env
        $<TARGET_FILE_DIR:sleep_monitor>/.env
    COMMENT "cymlinking .env into build directory"
)

add_custom_target(run
    COMMAND sleep_monitor
    DEPENDS sleep_monitor
    WORKING_DIRECTORY $<TARGET_FILE_DIR:sleep_monitor>
)

add_custom_target(run-only
    COMMAND $<TARGET_FILE:sleep_monitor>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:sleep_monitor>
)
