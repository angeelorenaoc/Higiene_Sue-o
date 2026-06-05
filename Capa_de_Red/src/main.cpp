#include <thread>

//#include "logger/shorthands.hpp"
#include "logger/setup.hpp"
#include "mqtt/client.hpp"

#include "mqtt/headers/payload.hpp"
#include "mqtt/headers/commands.hpp"
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

    mqtt.setMessageCallback([](mqtt::client* self, const mqtt::topic& topic, const mqtt::payload& pl){
        if (pl.message == "ON") {
            self->publish(mqtt::topic::PUB, mqtt::payload::from(mqtt::cmd::ON, "cmd"));
        } else if (pl.message == "OFF") {
            self->publish(mqtt::topic::PUB, mqtt::payload::from(mqtt::cmd::OFF, "cmd"));
        }
    });

    mqtt.subscribe("#");
    mqtt.subscribe(mqtt::topic::SUB);
    mqtt.publish(mqtt::topic::PUB, mqtt::payload::from(mqtt::cmd::INIT));

    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mqtt.disconnect();
    logger::shutdown();
    return 0;
}
