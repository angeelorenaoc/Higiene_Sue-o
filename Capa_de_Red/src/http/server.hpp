#pragma once
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "../repository/repository.hpp"
#include "../vendor/httplib.h"
#include "headers/settings.hpp"

namespace http {

    class server {
    public:
        explicit server(repo::repository& repository);
        void start(const std::string& host = http::HOST, int port = http::PORT);

    private:
        httplib::Server  _srv;
        repo::repository& _repo;

        void register_routes();
    };

} // namespace http

#endif
