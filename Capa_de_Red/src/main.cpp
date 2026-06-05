#include <thread>

//#include "logger/shorthands.hpp"
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

    while (running) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mqtt.disconnect();
    logger::shutdown();
    return 0;
}
