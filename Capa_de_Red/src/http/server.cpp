#include "server.hpp"

#include <chrono>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "../vendor/httplib.h"
#include "../logger/shorthands.hpp"


namespace http {
    // ── helpers ──────────────────────────────────────────────────────────────────

    std::shared_ptr<spdlog::logger> setup_http_logger() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%d/%m/%Y %T] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            "./http.log",
            true
        );
        file_sink->set_level(spdlog::level::info);
        file_sink->set_pattern("[%d/%m/%Y %T] | %v");

        auto logger = std::make_shared<spdlog::logger>(
            "http",
            spdlog::sinks_init_list{console_sink, file_sink}
        );

        logger->set_level(spdlog::level::info);

        return logger;
    }

    // ── constructor / start ───────────────────────────────────────────────────────

    server::server(repo::repository& repository) : repo(repository) {
        _svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");

            if (req.method == "OPTIONS") {
                res.status = 204;
                return httplib::Server::HandlerResponse::Handled;
            }
            return httplib::Server::HandlerResponse::Unhandled;
        });

        this->http_logger = setup_http_logger();

        _svr.set_logger([this](const httplib::Request& req, const httplib::Response& res){
            using namespace std::chrono;
            auto ms = duration_cast<milliseconds>(steady_clock::now() - req.start_time_);

            this->http_logger->info("[{}] >> {} {} [{}] -> {} ({} ms, {} B)",
                    req.remote_addr,
                    req.method,
                    req.target,
                    req.matched_route,
                    res.status,
                    ms.count(),
                    res.body.size()
            );
        });
    }

    void server::start(const std::string& host, int port) {
        HTTP_DEBUG("Server listening on {}:{}", host, port);
        _svr.listen(host, port);
    }
    void server::stop(){
        _svr.stop();
    }

    server::handler_t hardenedHandler(server::handler_t handle){
        return [handle](const httplib::Request& req, httplib::Response& res) {
            try {
                HTTP_DEBUG("Incoming {} petition on '{}'", req.method, req.matched_route);
                handle(req, res);
            } catch (const std::exception& e) {
                HTTP_WARN("Handler exception: {}", e.what());
                res.status = 500;
                res.set_content("internal error", "text/plain");
            } catch (...) {
                HTTP_DEBUG("Unknown handler exception");
                res.status = 500;
            }
        };
    }

    // --- Methods ------------------------------------------------------------------

    void server::Post(const std::string& route, handler_t handle){
        _svr.Post(route, hardenedHandler(handle));
        HTTP_DEBUG("Registered POST '{}'", route);
    }
    void server::Get(const std::string& route, handler_t handle){
        _svr.Get(route, hardenedHandler(handle));
        HTTP_DEBUG("Registered GET '{}'", route);
    }
    void server::Put(const std::string& route, handler_t handle){
        _svr.Put(route, hardenedHandler(handle));
        HTTP_DEBUG("Registered PUT '{}'", route);

    }
    void server::Patch(const std::string& route, handler_t handle){
        _svr.Patch(route, hardenedHandler(handle));
        HTTP_DEBUG("Registered PATCH '{}'", route);
    }
    void server::Delete(const std::string& route, handler_t handle){
        _svr.Delete(route, hardenedHandler(handle));
        HTTP_DEBUG("Registered DELETE '{}'", route);
    }

    // ── routes ────────────────────────────────────────────────────────────────────



} // namespace http
