#include "client.hpp"

#include <string>
#include <stdexcept>
#include <spdlog/fmt/fmt.h>

#include "../logger/shorthands.hpp"

#include "headers/payload.hpp"
#include "headers/topics.hpp"
#include "headers/commands.hpp"

namespace mqtt {
    client::client(const std::string& host, int port)
        : host(host), port(port) {
        mosquitto_lib_init();
        mosq = mosquitto_new(NULL, true, this);

        mosquitto_connect_callback_set(mosq, client::on_connect);
        mosquitto_message_callback_set(mosq, client::on_message);
    }

    client::~client() {
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    void client::start() {
        int rc = mosquitto_connect(mosq, host.c_str(), port, 60);
        if (rc != MOSQ_ERR_SUCCESS) {
            auto errMsg = fmt::format("Failed to connect: {}", mosquitto_strerror(rc));
            MQTT_CRITICAL(errMsg);
            MQTT_DEBUG("Check Mosquitto is running using 'sudo systemctl status mosquitto'");
            throw std::runtime_error(errMsg);
        } else {
            MQTT_INFO("[MQTT] Connected to broker");
        }
    }

    void client::publish(const topic& topic, const payload& payload) {
        mosquitto_publish(mosq, NULL, topic, payload.message.size(), payload.message.c_str(), 1, false);
    }

    void client::subscribe(const topic& topic) {
        int rc = mosquitto_subscribe(mosq, nullptr, topic, 0);
        MQTT_INFO("Subscribe to '{}': {}", topic, mosquitto_strerror(rc));
    }


    void client::on_connect(mosquitto* mosq, void* obj, int rc) {
        if (rc == 0) {
            MQTT_INFO("[MQTT] Connected");

            auto* self = static_cast<client*>(obj);
            self->subscribe(topic::SUB);
            self->publish(topic::PUB, payload{cmd::INIT});
        }
    }

    void client::on_message(mosquitto* mosq, void* obj, const mosquitto_message* msg) {
        auto* self = static_cast<client*>(obj);

        auto pl = payload::unmarshal(std::string((char*)msg->payload, msg->payloadlen));
        MQTT_INFO("[MQTT] On topic '{}': {}", msg->topic, pl.message);

        if (pl.message == "ON") {
            self->publish(topic::PUB, payload{cmd::ON});
        } else if (pl.message == "OFF") {
            self->publish(topic::PUB, payload{cmd::OFF});
        }

        self->cb(msg->topic, pl);
    }

    void client::loop() {
        mosquitto_loop_forever(mosq, -1, 1);
    }

    void client::setMessageCallback(message_callback cb) {
        this->cb = cb;
    }

}
