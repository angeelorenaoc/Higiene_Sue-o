#pragma once
#ifndef MQTT_SETUP_HPP
#define MQTT_SETUP_HPP

#include "client.hpp"

#include "../repository/repository.hpp"

namespace mqtt {
    void setup(client& c, repo::repository& repo);
}

#endif
