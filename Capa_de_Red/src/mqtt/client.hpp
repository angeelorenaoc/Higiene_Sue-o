#pragma once
#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <unordered_map>
#include <functional>

#include <mosquitto.h>

#include "headers/settings.hpp"
#include "headers/topics.hpp"
#include "headers/payload.hpp"

namespace mqtt {

    class client;

    using message_callback_t = std::function<void(client* self, const topic& topic, const payload& payload)>;
    using topic_callbacks_t = std::unordered_map<std::string, std::vector<message_callback_t>>;

    class client {
    public:
        client(const std::string& host, int port);
        client(const client&) = delete;
        client(client&&) = delete;

        client& operator=(const client&) = delete;
        client& operator=(client&&) = delete;

        ~client();

        bool start();
        bool disconnect();

        bool publish(const topic& topic, const payload& payload);
        bool subscribe(const topic& topic);

        void setMessageCallback(message_callback_t cb);
        void on(const topic& topic, const message_callback_t& cb);

    private:
        mosquitto* mosq = nullptr;
        std::string host = setting::HOST;
        int port = setting::PORT;

        std::string certsPath = setting::CERTS_PATH;
        message_callback_t cb = [](auto, auto, auto){};
        topic_callbacks_t topicCBs = {};

        static void on_connect(struct mosquitto*, void*, int);
        static void on_message(struct mosquitto*, void*, const struct mosquitto_message*);
        static void on_disconnect(struct mosquitto *mosq, void *obj, int rc);
    };
}

#endif
