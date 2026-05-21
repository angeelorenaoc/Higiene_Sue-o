#pragma once
#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "payload.hpp"

namespace mqtt::cmd {
    constexpr auto INIT = "init";
    constexpr auto REST = "rest";

    constexpr auto ON = "cmd1";
    constexpr auto OFF = "cmd2";
    constexpr auto CMD3 = "cmd3";
    constexpr auto CMD4 = "cmd4";
    constexpr auto CMD5 = "cmd5";
    constexpr auto CMD6 = "cmd6";

    inline payload send(const char* cmd){
        return payload{cmd};
    }
}

#endif
