#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>

#include "logger/setup.hpp"
#include "db/sqlite_db.hpp"
#include "db/setup.hpp"
#include "mqtt/client.hpp"
#include "mqtt/setup.hpp"
#include "mqtt/headers/payload.hpp"
#include "http/server.hpp"
#include "http/setup.hpp"
#include "repository/repository.hpp"

std::atomic<bool> running{true};

void stopProgram(int sig){
    SPDLOG_INFO("[SIGNAL]: Process stop requested ({}), terminating...", sig);
    running.store(false);
}

int main() {
    pid_t pid = getpid();
    std::cout << "Process ID: " << pid << "\n";

    std::signal(SIGINT, stopProgram);
    std::signal(SIGTERM, stopProgram);

    logger::setup();

    db::sqlite mqttDB;
    db::setup(mqttDB);

    repo::repository mqttRepo(mqttDB);
    mqtt::client mqttClient(mqtt::setting::HOST, mqtt::setting::PORT);
    mqtt::setup(mqttClient, mqttRepo);


    db::sqlite httpDB;
    db::setup(httpDB);
    repo::repository httpRepo(httpDB);

    auto http_srv = std::make_unique<http::server>(httpRepo);
    http::register_routes(*http_srv);
    std::thread http_thread([&]{ http_srv->start(); });

    // This thread will send messages every minute to check if an alarm should be activated
    // Triggers at the start of every minute
    std::thread timePoller([&mqttClient]() {
        using namespace std::chrono;

        while (running.load()) {
            auto now = system_clock::now();
            auto tt = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            localtime_r(&tt, &tm);

            auto time_str = std::format("{:02}:{:02}:{:02}",
                                        tm.tm_hour,
                                        tm.tm_min,
                                        tm.tm_sec);
            //auto now_sec = floor<seconds>(now);

            //auto time_str = std::format("{:%H:%M:%S}", now_sec);
            mqttClient.publish("sensor/time", mqtt::payload::from(time_str));

            auto next_minute = time_point_cast<minutes>(now) + minutes(1);
            std::this_thread::sleep_until(next_minute);
        }
    });

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    http_srv->stop();
    timePoller.join();
    http_thread.join();
    mqttClient.disconnect();
    logger::shutdown();
    return 0;
}
