#pragma once
#ifndef PAYLOAD_HPP
#define PAYLOAD_HPP

#include <string>

#include "../../logger/shorthands.hpp"

namespace mqtt {
    struct payload {
        std::string prefix;
        std::string message;
        bool illformed = true;

        static payload from(std::string message, std::string prefix = {}) {
            return {prefix, message, false};
        }
        std::string marshal() const {
            if (prefix.empty()) {
                return message;
            }

            return prefix + ":" + message;
        }
        static payload unmarshal(std::string payload){
            auto pos = payload.find(":");
            if (pos == payload.npos) {
                return {{}, payload, false};
                //MQTT_ERROR("Could not unmarshal message");
                //return {};
            }

            auto prefix = payload.substr(0, pos);
            auto msg = payload.substr(pos + 1);
            return {prefix, msg, false};
        }
    };
}


#endif // PAYLOAD_HPP
