#include "client.hpp"

#include <cerrno>
#include <mosquitto.h>
#include <mosquitto/defs.h>
#include <mosquitto/libcommon_string.h>
#include <mosquitto/libcommon_topic.h>
#include <mosquitto/libmosquitto.h>
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
//#include "headers/commands.hpp"

namespace mqtt {
    client::client(const std::string& host, int port) : host(host), port(port) {
        MQTT_DEBUG("Initializing mosquitto");
        int rc = mosquitto_lib_init();
        if (rc != MOSQ_ERR_SUCCESS) {
            MQTT_ERROR("Initialization failed: {}", mosquitto_strerror(rc));
            return;
        }

        MQTT_TRACE("Creating mosquitto client instance");
        mosq = mosquitto_new(NULL, true, this);
        if (mosq == NULL) {
            MQTT_ERROR("Mosquitto creation failed: {}", mosquitto_strerror(errno));
            return;
        }
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
        start();
    }

    client::~client() {
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    bool client::start() {
        MQTT_INFO("Connecting to {}:{}", host, port);
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
                MQTT_DEBUG("Mosquitto thread started succesfully");
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
        auto msg = payload.marshal();
        int rc = mosquitto_publish(mosq, NULL, topic, msg.size(), msg.c_str(), 1, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            MQTT_ERROR("Failed to publish payload: {}", mosquitto_strerror(rc));
            return false;
        }

        return true;
    }

    bool client::subscribe(const topic& topic) {
        MQTT_DEBUG("Subscribing to topic '{}'", topic);
        int rc = mosquitto_subscribe(mosq, nullptr, topic, setting::QOS);
        if (rc != MOSQ_ERR_SUCCESS) {
            MQTT_ERROR("Failed to subscribe to topic '{}': {}", topic, mosquitto_strerror(rc));
            return false;
        }

        MQTT_DEBUG("Subscribed to topic '{}'", topic);
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

        std::string payload = std::string((char*)msg->payload, msg->payloadlen);
        auto pl = payload::unmarshal(payload);
        if (pl.illformed){
            MQTT_WARN("Illformed message received on topic '{}': {}", msg->topic, payload);
            return;
        }

        MQTT_DEBUG("On topic '{}': {}", msg->topic, payload);
        self->cb(self, msg->topic, pl);


        for (const auto&[topic, topicCallbacks] : self->topicCBs){
            bool match = false;
            int rc = mosquitto_topic_matches_sub(topic.c_str(), msg->topic, &match);
            MQTT_DEBUG("Comparing '{}' == '{}'", topic.c_str(), msg->topic);
            MQTT_DEBUG("Topic compare result: {}", match);

            if (rc != MOSQ_ERR_SUCCESS){
                MQTT_WARN("Invalid input parameters, skipping");
                continue;
            }
            if (!match){
                MQTT_DEBUG("Topics didnt match: {} != {}", topic, msg->topic);
                continue;
            }

            for (const auto &callback : topicCallbacks){
                callback(self, msg->topic, pl);
            }
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

    void client::setMessageCallback(message_callback_t cb){
        this->cb = cb;
    }
    void client::on(const topic& topic, const message_callback_t& cb){
        topicCBs[topic].push_back(cb);
    }
}
