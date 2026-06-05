#include <thread>

//#include "logger/shorthands.hpp"
#include "logger/setup.hpp"
#include "logger/shorthands.hpp"
#include "mqtt/client.hpp"
#include "mqtt/headers/payload.hpp"
#include "rules/headers/commands.hpp"
#include "rules/rules.hpp"
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void onStopSignal(int sig){
    SPDLOG_INFO("[SIGNAL]: Process stop requested ({}), terminating...", sig);
    running = false;
}

int main() {
    std::signal(SIGINT, onStopSignal);
    std::signal(SIGTERM, onStopSignal);
    logger::setup();
    mqtt::client mqtt(mqtt::setting::HOST, mqtt::setting::PORT);

    //mqtt.setMessageCallback([](mqtt::client* self, const mqtt::topic& topic, const mqtt::payload& pl){});

    auto a = [](auto self, auto topic, const mqtt::payload& pl){
        if (pl.prefix != "cmd" || pl.message != "send") {
            RULE_INFO("Arrived but not processed '{}'", topic);
            return false;
        }

        RULE_INFO("Test call from '{}'", topic);
        self->publish(mqtt::topic::PUB, rules::cmd::from("siuu"));
        return true;
    };
    mqtt.on("+/esp32/#", a);

    auto b = [](auto self, auto topic, auto pl){
        if (pl.prefix != "cmd" || pl.message != "siuu") return false;

        RULE_INFO("SIUUUUU from '{}'", topic);
        self->publish("hmmm/esp32/xd", rules::cmd::from("send"));
        return true;
    };
    mqtt.on(mqtt::topic::SUB, b);

    mqtt.subscribe("#");
    mqtt.subscribe(mqtt::topic::SUB);
    mqtt.publish(mqtt::topic::PUB, rules::cmd::INIT);

    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mqtt.disconnect();
    logger::shutdown();
    return 0;
}
