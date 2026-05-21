#include <thread>

#include "logger/shorthands.hpp"
#include "logger/setup.hpp"
#include "mqtt/client.hpp"

int main() {
    logger::setup();

    mqtt::client mqtt("localhost", 1883);

    mqtt.setMessageCallback([](const mqtt::topic& topic, const mqtt::payload& payload) {
        MQTT_INFO("{} -> {}", topic, payload.message);

    });

    mqtt.start();
    mqtt.subscribe("sleep/sensor/#");

    // Blocking loop in a thread so we can add more things later
    std::thread mqttThread([&mqtt]() {
        mqtt.loop();
    });

    mqttThread.join();
    return 0;
}
