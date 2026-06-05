#include "client.hpp"

#include <mosquitto.h>
#include <mosquitto/defs.h>
#include <mosquitto/libcommon_string.h>
#include <mosquitto/libmosquitto_callbacks.h>
#include <mosquitto/libmosquitto_connect.h>
#include <mosquitto/libmosquitto_loop.h>
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
        mosquitto_disconnect_callback_set(mosq, client::on_disconnect);
    }

    client::~client() {
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    bool client::start() {
        MQTT_TRACE("Connecting mosquitto thread");
        int rc = mosquitto_connect_async(mosq, host.c_str(), port, setting::KEEP_ALIVE);
        if (rc != MOSQ_ERR_SUCCESS) {
            auto errMsg = fmt::format("Failed to connect: {}", mosquitto_strerror(rc));
            MQTT_CRITICAL(errMsg);
            MQTT_WARN("Check Mosquitto is running using 'sudo systemctl status mosquitto'");
            throw std::runtime_error(errMsg);
        }

        MQTT_TRACE("Connected to broker");
        MQTT_TRACE("Starting mosquitto thread");

        switch (mosquitto_loop_start(mosq)) {
            case MOSQ_ERR_SUCCESS:{
                MQTT_INFO("Mosquitto thread started succesfully");
                break;
            }
            case MOSQ_ERR_INVAL:{
                MQTT_ERROR("Invalid mosquitto instance used");
                return false;
            }
            case MOSQ_ERR_NOT_SUPPORTED:{
                MQTT_ERROR("Thread support not available");
                return false;
            }
        }

        return true;
    }
    bool client::disconnect() {
        if (mosq) {
            MQTT_TRACE("Disconnect requested");
            switch (mosquitto_disconnect(mosq)) {
                case MOSQ_ERR_SUCCESS:{
                    MQTT_INFO("Mosquitto thread disconnected succesfully");
                    break;
                }
                case MOSQ_ERR_INVAL:{
                    MQTT_ERROR("Invalid mosquitto instance used");
                    return false;
                }
                case MOSQ_ERR_NO_CONN:{
                    MQTT_ERROR("Client was not connected to a broker");
                    return false;
                }
            }

            MQTT_TRACE("Stopping...");
            switch (mosquitto_loop_stop(mosq, false)) {
                case MOSQ_ERR_SUCCESS:{
                    MQTT_INFO("Mosquitto thread stopped succesfully");
                    break;
                }
                case MOSQ_ERR_INVAL:{
                    MQTT_ERROR("Invalid mosquitto instance used");
                    return false;
                }
                case MOSQ_ERR_NOT_SUPPORTED:{
                    MQTT_ERROR("Thread support not available");
                    return false;
                }
            }
        }

        return true;
    }

    bool client::publish(const topic& topic, const payload& payload) {
        MQTT_TRACE("Publishing on topic '{}'", topic);
        int rc = mosquitto_publish(mosq, NULL, topic, payload.message.size(), payload.message.c_str(), 1, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            MQTT_ERROR("Failed to publish payload: {}", mosquitto_strerror(rc));
            return false;
        }

        return true;
    }

    bool client::subscribe(const topic& topic) {
        MQTT_TRACE("Subscribing to topic '{}'", topic);
        int rc = mosquitto_subscribe(mosq, nullptr, topic, setting::QOS);
        if (rc != MOSQ_ERR_SUCCESS) {
            MQTT_ERROR("Failed to subscribe to topic '{}': {}", topic, mosquitto_strerror(rc));
            return false;
        }

        MQTT_INFO("Subscribed to topic '{}'", topic);
        return true;
    }


    void client::on_connect(mosquitto* mosq, void* obj, int rc) {
        if (rc == 0) {
            MQTT_TRACE("Connected");
            auto* self = static_cast<client*>(obj);
        } else {
            MQTT_WARN("Could not connect: {}", mosquitto_strerror(rc));
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
    }
    void client::on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
        auto* self = static_cast<client*>(obj);
            if (rc == 0) {
                MQTT_INFO("Clean MQTT disconnect");
            } else {
                MQTT_WARN("Unexpected MQTT disconnect, rc = {}", rc);
            }
    }

    void client::loop() {
        mosquitto_loop_forever(mosq, setting::TIMEOUT, 1);
    }

}
