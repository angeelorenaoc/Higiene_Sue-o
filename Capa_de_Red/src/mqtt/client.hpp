#pragma once
#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <functional>

#include <mosquitto.h>

#include "headers/settings.hpp"
#include "headers/topics.hpp"
#include "headers/payload.hpp"

namespace mqtt {

    using message_callback = std::function<void(const topic& topic, const payload& payload)>;

    class client {
    public:
        client(const std::string& host, int port);
        ~client();

        void start();
        void publish(const topic& topic, const payload& payload);
        void subscribe(const topic& topic);
        void loop();

        void setMessageCallback(message_callback cb);

    private:
        mosquitto* mosq = nullptr;
        std::string host = setting::HOST;
        int port = setting::PORT;
        int qos = setting::QOS;
        message_callback cb = [](const topic&, const payload&){};

        static void on_connect(struct mosquitto*, void*, int);
        static void on_message(struct mosquitto*, void*, const struct mosquitto_message*);
    };
}

#endif
