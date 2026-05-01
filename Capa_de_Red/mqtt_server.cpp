#include <mosquitto/defs.h>
#include <iostream>
#include <mosquitto.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>

namespace settings {
    constexpr auto HOST = "localhost";
    constexpr auto PORT = 1883;
    constexpr auto KEEP_ALIVE = 60;
    constexpr auto TIMEOUT = -1; // Default: -1 (1000ms)
}
namespace topic {
    constexpr auto PUB = "esp32/control";
    constexpr auto SUB = "test/topic";
}
namespace command {
    constexpr auto INIT = "init";
    constexpr auto REST = "rest";

    constexpr auto CMD1 = "cmd1";
    constexpr auto CMD2 = "cmd2";
    constexpr auto CMD3 = "cmd3";
    constexpr auto CMD4 = "cmd4";
    constexpr auto CMD5 = "cmd5";
    constexpr auto CMD6 = "cmd6";
}

void send_command(mosquitto *mosq, const char *cmd) {
    mosquitto_publish(mosq, NULL, topic::PUB, strlen(cmd), cmd, 1, false);
}

void on_connect(mosquitto *mosq, void *obj, int rc) {
    if (rc == 0) {
        std::cout << "Connected\n";
        mosquitto_subscribe(mosq, NULL, topic::SUB, 0);
        send_command(mosq, command::INIT);
    }
    else {
        std::cout << "Connection failed: " << rc << "\n";
    }
}

void on_message(mosquitto *mosq, void *obj, const mosquitto_message *msg) {
    std::string_view payload(static_cast<char*>(msg->payload), msg->payloadlen);

    std::cout << "Received on " << msg->topic << ": " << payload << "\n";

    if (payload == "hmmm") {
        std::cout << "Turning ON device\n";
        send_command(mosq, command::INIT);   // forward to ESP32
    }
    else if (payload == "kys") {
        std::cout << "Turning OFF device\n";
        send_command(mosq, command::REST);
    }
}

int main() {
    printf("Starting\n");
    if (mosquitto_lib_init() != MOSQ_ERR_SUCCESS){
        std::cout << "Mosquitto failed to init, terminating...\n";
        exit(-1);
    }
    std::cout << "Mosquitto lib init finish\n";

    mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    std::cout << "Mosquitto instance created\n";

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);
    std::cout << "Callbacks set\n";

    if (mosquitto_connect(mosq, settings::HOST, settings::PORT, settings::KEEP_ALIVE) != MOSQ_ERR_SUCCESS) {
            std::cerr << "Unable to connect\n";
            return 1;
        }
    std::cout << "Mosquitto connected to " << settings::HOST << " on port " << settings::PORT << "\n";

    mosquitto_loop_forever(mosq, settings::TIMEOUT, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
