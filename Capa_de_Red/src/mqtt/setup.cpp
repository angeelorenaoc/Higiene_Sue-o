#include "setup.hpp"

#include <chrono>
#include <expected>
#include <optional>
#include <string_view>

#include "headers/payload.hpp"
#include "headers/topics.hpp"
#include "../rules/rules.hpp"
#include "../rules/headers/commands.hpp"
#include "../repository/repository.hpp"
#include "../datetime/datetime.hpp"
#include "../logger/shorthands.hpp"

namespace mqtt {
    namespace reading {
        const topic COMMON = "sensor";

        const topic TEMP = COMMON/"temperatura";
        const topic HUMID = COMMON/"humedad";
        const topic LIGHT = COMMON/"luz";
        const topic SOUND = COMMON/"audio";
        const topic MOTION = COMMON/"motion";

        const topic TIME = COMMON/"time"; // Special topic

        const topic ANY = COMMON/topic::ANY;
        const topic ALL = COMMON/topic::ALL;
    }
    namespace control {
        const topic COMMON = "actuador";

        const topic ALARM = COMMON/"alarm";
        const topic MOTOR = COMMON/"motor";
        //const topic LED = COMMON/"led";
        //
        const topic ANY = COMMON/topic::ANY;
        const topic ALL = COMMON/topic::ALL;
    }
    namespace custom {
        namespace activate {
            const topic ACTIVATE = "activate";

            const topic ALARM = ACTIVATE/"alarm";
            const topic MOTOR = ACTIVATE/"motor";
        }

    }

    std::optional<std::string> translate_reading(std::string s) {
        if (s == "temperatura") return "temperature";
        if (s == "humedad") return "humidity";
        if (s == "luz") return "light";
        if (s == "audio") return "noise";
        if (s == "movimiento") return "motion";

        return std::nullopt;
    }

    std::optional<topic> actuator_to_topic(std::string s) {
        if (s == "buzzer") return control::ALARM;
        if (s == "motor") return control::MOTOR;

        return std::nullopt;
    }

    bool common(repo::repository& repo, std::string_view reading_name, client* self, const topic& tpic, const payload& pl){
        auto parsed = util::parse::to<double>(pl.message);
        if (!parsed.has_value()) {
            RULE_WARN("Unprocessable value: {}", parsed.error());
            return false;
        }

        double reading_value = parsed.value();
        RULE_DEBUG("Got reading: '{}' -> '{}'", tpic, reading_value);

        auto reading = repo.insert_reading(reading_name, reading_value);
        if (!reading) {
            RULE_ERROR("Failed to insert reading for '{}': {}", reading_name, reading.error());
            return false;
        }

        int64_t reading_id = reading.value();
        RULE_DEBUG("Reading stored: {} -> {} (id={})", reading_name, reading_value, reading_id);

        RULE_DEBUG("Getting rules for reading");
        auto rules = repo.get_rules_for_reading(reading_name);
        if (!rules || rules->empty()) {
            RULE_DEBUG("No rules for '{}'", reading_name);
            return true;
        }

        for (const auto& rule : rules.value()) {
            RULE_DEBUG("Getting condition type for rule");
            auto condition = repo.get_condition_type(rule.id_condition_type);
            if (!condition) {
                RULE_WARN("Unknown condition type id: {}", rule.id_condition_type);
                continue;
            }

            const std::string& cond = condition.value().name;
            RULE_DEBUG("Comparing if {} is {} {}", reading_value, cond, rule.condition_value);
            if (!rules::compare(cond, reading_value, rule.condition_value)) {
                RULE_DEBUG("Value doesnt trigger the rule, skipping");
                continue;
            }

            RULE_INFO("Rule {} triggered for '{}' -> {}", rule.id, reading_name, reading_value);
            auto actuator = repo.get_actuator_type(rule.id_actuator_type);
            if (!actuator) {
                RULE_ERROR("Could not store actuator log for actuator id {}: {}", rule.id_actuator_type, actuator.error());
                continue;
            }
            else {
                auto topic_trail = actuator_to_topic(actuator.value().name);
                if (!topic_trail){
                    RULE_ERROR("Failed to send command to '{}': Could not find the actuator topic", actuator.value().name);
                    continue;
                }

                topic actuator_topic = control::COMMON/topic_trail.value();
                if (!self->publish(actuator_topic, rules::cmd::custom("1", ""))){
                    RULE_ERROR("Failed to send command to '{}': Command publish failed", actuator.value().name);
                    continue;
                }

                RULE_INFO("Command sent to '{}'", actuator_topic);
                RULE_DEBUG("Storing actuator action log for {}", actuator_topic);
                if (auto res = repo.insert_actuator_log(actuator.value().name, rule.id, reading_id, "1"); !res) {
                    RULE_ERROR("Failed to log actuator action for '{}': {}", actuator.value().name, res.error());
                    continue;
                }
            }
        }
        return true;
    }

    void setup(client& c, repo::repository& repo) {
        c.setMessageCallback([](client* self, const topic& tpic, const payload& pl){
            RULE_INFO("Message on topic: {}", tpic);
        });
        c.on(reading::TEMP, [&repo](client* self, const topic& tpic, const payload& pl){
            return common(repo, "temperature", self, tpic, pl);
        });
        c.on(reading::HUMID, [&repo](client* self, const topic& tpic, const payload& pl){
            return common(repo, "humidity", self, tpic, pl);
        });
        c.on(reading::LIGHT, [&repo](client* self, const topic& tpic, const payload& pl){
            return common(repo, "light", self, tpic, pl);
        });
        c.on(reading::SOUND, [&repo](client* self, const topic& tpic, const payload& pl){
            return common(repo, "noise", self, tpic, pl);
        });
        c.on(reading::MOTION, [&repo](client* self, const topic& tpic, const payload& pl){
            return common(repo, "motion", self, tpic, pl);
        });
        c.on(reading::TIME, [&repo](client* self, const topic& tpic, const payload& pl){
            // This function does not insert readings,
            // it is only used to check if actuators should be activated
            // at a certain time.
            RULE_DEBUG("Got message: '{}' -> '{}'", tpic, pl.message);

            auto parsed = dt::time::from(pl.message);
            if (!parsed.has_value()) {
                RULE_ERROR("Unprocessable value: Format is hh:mm:ss, got {}", pl.message);
                return false;
            }

            dt::time reading_time = parsed.value();
            RULE_DEBUG("Got reading: '{}' -> '{}'", tpic, reading_time);

            RULE_DEBUG("Getting rules for reading");
            auto rules = repo.get_time_rules_ordered();
            if (!rules.has_value() || rules.value().empty()) {
                RULE_DEBUG("No rules for 'time'");
                return true;
            }

            RULE_DEBUG("Reading rules");
            for (const auto& rule : rules.value()) {
                RULE_DEBUG("Getting condition type for rule");
                auto condition = repo.get_condition_type(rule.id_condition_type);
                if (!condition) {
                    RULE_WARN("Unknown condition type id: {}", rule.id_condition_type);
                    continue;
                }

                const std::string& cond = condition.value().name;
                RULE_DEBUG("Comparing if {} is less than a minute away from {}", reading_time, rule.condition_time);
                auto secondsDifference = std::abs((reading_time - rule.condition_time).count());
                RULE_DEBUG("Seconds of difference: {}", secondsDifference);
                if (std::chrono::seconds{secondsDifference} > std::chrono::seconds{59}){
                    RULE_DEBUG("Values are too far appart, skipping");
                    continue;
                }
                RULE_DEBUG("Comparing if {} is {} {}", reading_time, cond, rule.condition_time);
                if (!rules::compare(cond, reading_time, rule.condition_time)) {
                    RULE_DEBUG("Value doesnt trigger the rule, skipping");
                    continue;
                }

                RULE_INFO("Rule {} triggered for 'time' -> {}", rule.id, reading_time);
                auto actuator = repo.get_actuator_type(rule.id_actuator_type);
                if (!actuator) {
                    RULE_ERROR("Could not store actuator log for actuator id {}: {}", rule.id_actuator_type, actuator.error());
                    continue;
                }
                auto command_topic = actuator_to_topic(actuator.value().name);
                if (!command_topic){
                    RULE_ERROR("Failed to send command to '{}': Could not find the actuator topic", actuator.value().name);
                    continue;
                }

                topic actuator_topic = command_topic.value();
                if (!self->publish(actuator_topic, rules::cmd::custom("1", ""))){
                    RULE_ERROR("Failed to send command to '{}': Command publish failed", actuator.value().name);
                    continue;
                }

                RULE_INFO("Command sent to '{}'", actuator_topic);
                RULE_WARN("Actuator log not available for time rules");
                // RULE_DEBUG("Storing actuator action log for {}", actuator_topic);
                // if (auto res = repo.insert_actuator_log(actuator.value().name, rule.id, -1, "1 at " + reading_time.to_string()); !res) {
                //     RULE_ERROR("Failed to log actuator action for '{}': {}", actuator.value().name, res.error());
                //     continue;
                // }
            }
            return true;
        });

        c.on(control::ANY, [&repo](auto self, const topic& topic, const payload& pl){
            RULE_DEBUG("Got command: '{}' -> '{}:{}'", topic, pl.prefix, pl.message);
            return true;
        });

        /*
        c.on(custom::activate::ALARM, [](auto self, auto topic, const payload& pl){
            RULE_INFO("Sending to alarm");
            self->publish(control::ALARM, rules::cmd::custom("1", ""));
            return true;
        });

        c.on(custom::activate::MOTOR, [](auto self, auto topic, const payload& pl){
            RULE_INFO("Sending to alarm");
            self->publish(control::MOTOR, rules::cmd::from("1"));
            return true;
        });
        */
    }
}
