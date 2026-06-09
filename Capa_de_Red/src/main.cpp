#include <thread>
#include <csignal>
#include <atomic>
#include <vector>

#include "db/sqlite_db.hpp"
#include "logger/setup.hpp"
#include "logger/shorthands.hpp"
#include "mqtt/client.hpp"
#include "mqtt/headers/payload.hpp"
#include "mqtt/headers/topics.hpp"
#include "repository/repository.hpp"
#include "rules/headers/commands.hpp"
#include "db/sqlite_db.hpp"
#include "repository/models.hpp"
#include "repository/repository.hpp"
#include "utils/parse.hpp"

std::atomic<bool> running{true};

void onStopSignal(int sig){
    SPDLOG_INFO("[SIGNAL]: Process stop requested ({}), terminating...", sig);
    running = false;
}


const mqtt::topic BASE = "sweetdreams";
const mqtt::topic ESP32 = "esp32";

namespace reading {
    const mqtt::topic COMMON = BASE/ESP32/mqtt::topic::INFO;
    const mqtt::topic TEMP = COMMON/"temperature";
    const mqtt::topic HUM = COMMON/"humidity";
    const mqtt::topic LIGHT = COMMON/"light";
    const mqtt::topic NOISE = COMMON/"noise";
    const mqtt::topic MOTION = COMMON/"motion";
    const mqtt::topic ANY = COMMON/mqtt::topic::ANY;
    const mqtt::topic ALL = COMMON/mqtt::topic::ALL;
}
namespace control {
    const mqtt::topic COMMON = BASE/ESP32/mqtt::topic::CONTROL;
    const mqtt::topic BUZZ = COMMON/"buzzer";
    const mqtt::topic LED = COMMON/"led";
    const mqtt::topic MOTOR = COMMON/"motor";
    const mqtt::topic ANY = COMMON/mqtt::topic::ANY;
    const mqtt::topic ALL = COMMON/mqtt::topic::ALL;
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

    repo::repository rp(database);


    //mqtt.setMessageCallback([](mqtt::client* self, const mqtt::topic& topic, const mqtt::payload& pl){});

    mqtt.on(reading::ANY, [&rp](auto self, const mqtt::topic& topic, const mqtt::payload& pl){
        RULE_INFO("Message on info topic: {}", topic.last());
        auto parsed = util::parse::to<double>(pl.message);

        if (pl.prefix.empty() || !parsed.has_value()) {
            RULE_WARN("Unprocessable value: {}", parsed.error());
            return false;
        }

        double value = parsed.value();
        RULE_INFO("Got reading: '{}' -> '{}'", topic, value);

        auto reading = rp.insert_reading(topic.last(), value);
        if (!reading) {
            RULE_ERROR("Failed to insert reading for '{}': {}", topic.last(), reading.error());
            return false;
        }

        int64_t reading_id = reading.value();
        RULE_INFO("Reading stored: {} -> {} (id={})", topic.last(), value, reading_id);

        RULE_INFO("Getting rules for reading");
        auto rules = rp.get_rules_for_reading(topic.last());
        if (!rules || rules->empty()) {
            RULE_DEBUG("No rules for '{}'", topic.last());
            return true;
        }

        for (const auto& rule : rules.value()) {
            auto condition = rp.get_condition_type(rule.id_condition_type);
            if (!condition) {
                RULE_WARN("Unknown condition type id: {}", rule.id_condition_type);
                continue;
            }

            bool triggered = false;
            const std::string& cond = condition.value().name;
            if      (cond == "over")          triggered = value >  rule.condition_value;
            else if (cond == "under")         triggered = value <  rule.condition_value;
            else if (cond == "equal")         triggered = value == rule.condition_value;
            else if (cond == "different")     triggered = value != rule.condition_value;
            else if (cond == "over_or_equal") triggered = value >= rule.condition_value;
            else if (cond == "under_or_equal")triggered = value <= rule.condition_value;

            if (!triggered) continue;

            RULE_INFO("Rule {} triggered for '{}' -> {}", rule.id, topic.last(), value);
            // resolve actuator name, publish command, insert_actuator_log
            auto actuator = rp.get_actuator_type(rule.id_actuator_type);
            if (!actuator) {
                RULE_WARN("Unknown actuator type id: {}", rule.id_actuator_type);
                continue;
            }

            mqtt::topic actuator_topic = control::COMMON/actuator.value().name.c_str();
            self->publish(actuator_topic, rules::cmd::from("1"));
            RULE_INFO("Command sent to '{}'", actuator_topic);

            if (auto res = rp.insert_actuator_log(actuator->name, rule.id, reading_id, "1"); !res) {
                RULE_ERROR("Failed to log actuator action for '{}': {}", actuator->name, res.error());
            }
        }

        return true;
    });
    mqtt.on(control::ANY, [&rp](auto self, const mqtt::topic& topic, const mqtt::payload& pl){
        RULE_INFO("Message on control topic: {}", topic.last());
        auto parsed = util::parse::to<int>(pl.message);

        if (pl.prefix.empty() || !parsed.has_value()) {
            RULE_WARN("Unprocessable command: Invalid format");
            return false;
        }

        double value = parsed.value();
        RULE_INFO("Got command: '{}' -> '{}'", topic, value);
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
