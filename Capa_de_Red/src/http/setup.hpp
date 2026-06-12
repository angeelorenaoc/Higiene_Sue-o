#pragma once
#ifndef HTTP_SETUP_HPP
#define HTTP_SETUP_HPP

#include "server.hpp"

namespace http {
    void register_routes(server& s);
}

#endif
