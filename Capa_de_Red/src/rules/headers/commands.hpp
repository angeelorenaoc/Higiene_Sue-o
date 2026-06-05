#pragma once
#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "../../mqtt/headers/payload.hpp"

namespace rules::cmd {
    const auto INIT = mqtt::payload::from("init", "cmd");
    const auto REST = mqtt::payload::from("rest", "cmd");

    const auto ON = mqtt::payload::from("on", "cmd");
    const auto OFF = mqtt::payload::from("off", "cmd");

    const auto MOVE_A = mqtt::payload::from("1", "cmd");
    const auto MOVE_B = mqtt::payload::from("1", "cmd");
    const auto MOVE_C = mqtt::payload::from("1", "cmd");

    inline mqtt::payload from(const char* cmd){return mqtt::payload::from(cmd, "cmd");}
}

#endif
