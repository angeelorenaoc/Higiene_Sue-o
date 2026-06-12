#pragma once
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "../repository/repository.hpp"
#include "../vendor/httplib.h"
#include "headers/settings.hpp"

namespace http {

    class server {
    public:
        using handler_t = std::function<void(const httplib::Request &, httplib::Response &)>;
        // Yes, I maybe should do a service, but im out of time now
        // and this fits our needs well
        repo::repository& repo;

    public:
        explicit server(repo::repository& repository);
        void start(const std::string& host = http::HOST, int port = http::PORT);
        void stop();

        void Post(const std::string& route, handler_t handle);
        void Get(const std::string& route, handler_t handle);
        void Put(const std::string& route, handler_t handle);
        void Patch(const std::string& route, handler_t handle);
        void Delete(const std::string& route, handler_t handle);

    private:
        httplib::Server  _svr;
        std::shared_ptr<spdlog::logger> http_logger;
    };

} // namespace http

#endif
