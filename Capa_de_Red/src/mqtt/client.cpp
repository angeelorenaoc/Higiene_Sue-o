#include "client.hpp"

#include <mosquitto/broker_plugin.h>
#include <string>
#include <stdexcept>
#include <spdlog/fmt/fmt.h>

#include "../logger/shorthands.hpp"

#include "headers/payload.hpp"
#include "headers/settings.hpp"
#include "headers/topics.hpp"
#include "headers/commands.hpp"

namespace mqtt {
    client::client(const std::string& host, int port)
        : host(host), port(port) {
        MQTT_INFO("Connecting to {}:{}", host, port);
        mosquitto_lib_init();
        mosq = mosquitto_new(NULL, true, this);
        mosquitto_log_callback_set(mosq, [](mosquitto*, void*, int level, const char* str) {
            MQTT_DEBUG("[{}]: {}\n", level, str);
        });

        if (setting::USE_TLS) {
            MQTT_INFO("Configuring TLS");
            MQTT_INFO("Using TLS insecure: {}", setting::USE_TLS_INSECURE);
            mosquitto_tls_insecure_set(mosq, setting::USE_TLS_INSECURE);
            int tls_rc = mosquitto_tls_set(
                mosq,
                (certsPath + "/ca.crt").c_str(),
                nullptr,
                (certsPath + "/client.crt").c_str(),
                (certsPath + "/client.key").c_str(),
                nullptr
            );

            MQTT_INFO("TLS result: {}", mosquitto_strerror(tls_rc));
        }
        else {
            MQTT_INFO("Not using TLS");
        }

        mosquitto_connect_callback_set(mosq, client::on_connect);
        mosquitto_message_callback_set(mosq, client::on_message);
    }

    client::~client() {
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    void client::start() {
        int rc = mosquitto_connect(mosq, host.c_str(), port, setting::KEEP_ALIVE);
        if (rc != MOSQ_ERR_SUCCESS) {
            auto errMsg = fmt::format("Failed to connect: {}", mosquitto_strerror(rc));
            MQTT_CRITICAL(errMsg);
            MQTT_DEBUG("Check Mosquitto is running using 'sudo systemctl status mosquitto'");
            throw std::runtime_error(errMsg);
        } else {
            MQTT_INFO("Connected to broker");
        }
    }

    void client::publish(const topic& topic, const payload& payload) {
        mosquitto_publish(mosq, NULL, topic, payload.message.size(), payload.message.c_str(), 1, false);
    }

    void client::subscribe(const topic& topic) {
        int rc = mosquitto_subscribe(mosq, nullptr, topic, setting::QOS);
        MQTT_INFO("Subscribe to '{}': {}", topic, mosquitto_strerror(rc));
    }


    void client::on_connect(mosquitto* mosq, void* obj, int rc) {
        if (rc == 0) {
            MQTT_INFO("Connected");

            auto* self = static_cast<client*>(obj);
            self->subscribe(topic::SUB);
            self->publish(topic::PUB, payload{cmd::INIT});
        }
    }

    void client::on_message(mosquitto* mosq, void* obj, const mosquitto_message* msg) {
        auto* self = static_cast<client*>(obj);

        auto pl = payload::unmarshal(std::string((char*)msg->payload, msg->payloadlen));
        MQTT_INFO("On topic '{}': {}", msg->topic, pl.message);

        if (pl.message == "ON") {
            self->publish(topic::PUB, payload{cmd::ON});
        } else if (pl.message == "OFF") {
            self->publish(topic::PUB, payload{cmd::OFF});
        }

        if (self->cb) {
            self->cb(msg->topic, pl);
        }
    }

    void client::loop() {
        mosquitto_loop_forever(mosq, -1, 1);
    }

    void client::setMessageCallback(message_callback cb) {
        this->cb = cb;
    }

}
