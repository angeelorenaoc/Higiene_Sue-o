cmake_minimum_required(VERSION 3.16)
project(sleep_monitor)

set(CMAKE_CXX_STANDARD 23)

find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(src/vendor/spdlog)

add_executable(sleep_monitor
    src/main.cpp
    src/mqtt/client.cpp
    src/logger/setup.cpp
)

target_link_libraries(sleep_monitor
    SQLite::SQLite3
    nlohmann_json::nlohmann_json
    mosquitto
)

add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
target_include_directories(sleep_monitor PRIVATE vendor)
