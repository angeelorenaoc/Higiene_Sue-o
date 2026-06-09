#include <thread>
#include <csignal>
#include <atomic>
#include <vector>

#include "db/models.hpp"
#include "db/sqlite_db.hpp"
#include "logger/setup.hpp"
#include "logger/shorthands.hpp"
#include "mqtt/client.hpp"
#include "mqtt/headers/payload.hpp"
#include "mqtt/headers/topics.hpp"
#include "rules/headers/commands.hpp"
#include "db/sqlite_db.hpp"
#include "db/queries.hpp"
#include "db/migrations.hpp"
#include "utils/parse.hpp"

std::atomic<bool> running{true};

void onStopSignal(int sig){
    SPDLOG_INFO("[SIGNAL]: Process stop requested ({}), terminating...", sig);
    running = false;
}


const mqtt::topic BASE = "sweetdreams";
const mqtt::topic ESP32 = "esp32";

namespace reading {
    const mqtt::topic TEMP = BASE/ESP32/mqtt::topic::INFO/"temperature";
    const mqtt::topic HUM = BASE/ESP32/mqtt::topic::INFO/"humidity";
    const mqtt::topic LIGHT = BASE/ESP32/mqtt::topic::INFO/"light";
    const mqtt::topic NOISE = BASE/ESP32/mqtt::topic::INFO/"noise";
    const mqtt::topic MOTION = BASE/ESP32/mqtt::topic::INFO/"motion";
    const mqtt::topic ANY = BASE/ESP32/mqtt::topic::INFO/mqtt::topic::ANY;
    const mqtt::topic ALL = BASE/ESP32/mqtt::topic::INFO/mqtt::topic::ALL;
}
namespace control {
    const mqtt::topic BUZZ = BASE/ESP32/mqtt::topic::CONTROL/"buzzer";
    const mqtt::topic LED = BASE/ESP32/mqtt::topic::CONTROL/"led";
    const mqtt::topic MOTOR = BASE/ESP32/mqtt::topic::CONTROL/"motor";
    const mqtt::topic ANY = BASE/ESP32/mqtt::topic::CONTROL/mqtt::topic::ANY;
    const mqtt::topic ALL = BASE/ESP32/mqtt::topic::CONTROL/mqtt::topic::ALL;
}

int main() {
    std::signal(SIGINT, onStopSignal);
    std::signal(SIGTERM, onStopSignal);

    logger::setup();
    mqtt::client mqtt(mqtt::setting::HOST, mqtt::setting::PORT);
    db::sqlite database;


    if (auto res = database.start("sweetdreams.sqlite", "./migrations"); !res) {
        DB_CRITICAL("Database startup failed: {}", res.error());
        return -1;
    }


    //mqtt.setMessageCallback([](mqtt::client* self, const mqtt::topic& topic, const mqtt::payload& pl){});

    mqtt.on(reading::ANY, [&database](auto self, const mqtt::topic& topic, const mqtt::payload& pl){
        RULE_INFO("Message on info topic: {}", topic.last());
        auto parsed = util::parse::to<double>(pl.message);

        if (pl.prefix.empty() || !parsed.has_value()) {
            RULE_WARN("Unprocessable value: {}", parsed.error());
            return false;
        }

        double value = parsed.value();
        RULE_INFO("Got reading: '{}' -> '{}'", topic, value);
        if (auto res = db::insert_reading(database, topic.last(), value); !res) {
            RULE_ERROR("Failed to insert reading for '{}': {}", topic.last(), res.error());
            return false;
        }

        RULE_INFO("Reading stored: {} -> {}", topic.last(), value);
        return true;
    });
    mqtt.on(control::ANY, [&database](auto self, const mqtt::topic& topic, const mqtt::payload& pl){
        RULE_INFO("Message on control topic: {}", topic.last());
        auto parsed = util::parse::to<int>(pl.message);

        if (pl.prefix.empty() || !parsed.has_value()) {
            RULE_WARN("Unprocessable command: Invalid format");
            return false;
        }

        double value = parsed.value();
        RULE_INFO("Got command: '{}' -> '{}'", topic, value);
        if (auto res = db::insert_actuator_log(database, topic.last(), "activate", -1, -1); !res) {
               RULE_ERROR("Failed to insert actuator log for '{}': {}", topic.last(), res.error());
               return false;
           }

        RULE_INFO("Reading stored: {} -> {}", topic.last(), value);
        return true;
    });

    mqtt.on(reading::TEMP, [](auto self, auto topic, const mqtt::payload& pl){
        auto parsed = util::parse::to<double>(pl.message);
        if (pl.prefix != "t" || !parsed.has_value()) {
            RULE_WARN("Unprocessable value: Invalid format");
            return false;
        }

        double value = parsed.value();
        RULE_INFO("Got temperature reading: '{}'", value);
        return true;
    });
    mqtt.on(mqtt::topic::SUB, [](auto self, auto topic, const mqtt::payload& pl){
        if (pl.prefix != "cmd" || pl.message != "siuu") return false;

        RULE_INFO("SIUUUUU from '{}'", topic);
        self->publish("hmmm/esp32/xd", rules::cmd::from("send"));
        return true;
    });

    mqtt.subscribe("#");



    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mqtt.disconnect();
    logger::shutdown();
    return 0;
}
