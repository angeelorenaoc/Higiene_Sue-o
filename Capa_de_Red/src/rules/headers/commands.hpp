#pragma once
#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "../../mqtt/headers/payload.hpp"

namespace rules::cmd {
    inline mqtt::payload from(const char* cmd){return mqtt::payload::from(cmd, "cmd");}
    inline mqtt::payload custom(const char* cmd, const char* type){return mqtt::payload::from(cmd, type);}

    const auto INIT = from("init");
    const auto REST = from("rest");

    const auto ON = from("on");
    const auto OFF = from("off");

    const auto MOVE_A = from("1");
    const auto MOVE_B = from("1");
    const auto MOVE_C = from("1");


}

#endif
