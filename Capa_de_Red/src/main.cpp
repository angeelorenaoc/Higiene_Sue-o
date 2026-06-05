#include <thread>

#include "logger/shorthands.hpp"
#include "logger/setup.hpp"
#include "mqtt/client.hpp"

#include "mqtt/headers/payload.hpp"
#include "mqtt/headers/commands.hpp"
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void onStopSignal(int){
    SPDLOG_INFO("[SIGNAL]: Process stop requested, terminating...");
    running = false;
}

int main() {
    std::signal(SIGINT, onStopSignal);
    logger::setup();

    mqtt::client mqtt(mqtt::setting::HOST, mqtt::setting::PORT);

    mqtt.start();
    mqtt.subscribe("#");
    mqtt.subscribe(mqtt::topic::SUB);
    mqtt.publish(mqtt::topic::PUB, mqtt::payload{mqtt::cmd::INIT});

    // Blocking loop in a thread so we can add more things later
    std::thread mqttThread([&mqtt]() {
        while (running);

    });

    mqttThread.join();

    mqtt.disconnect();
    logger::shutdown();
    return 0;
}
