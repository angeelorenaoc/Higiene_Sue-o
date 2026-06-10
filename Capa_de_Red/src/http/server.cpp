#include "server.hpp"

#include <nlohmann/json.hpp>
#include <optional>

#include "../utils/parse.hpp"
#include "../logger/shorthands.hpp"


namespace http {

    using json = nlohmann::json;
// ── helpers ──────────────────────────────────────────────────────────────────

    static void send_json(httplib::Response& res, const json& body, int status = 200) {
        res.status = status;
        res.set_content(body.dump(), "application/json");
    }

    static void send_error(httplib::Response& res, const std::string& msg, int status = 500) {
        send_json(res, {{"error", msg}}, status);
    }

    // ── constructor / start ───────────────────────────────────────────────────────

    server::server(repo::repository& repository) : _repo(repository) {
        register_routes();
    }

    void server::start(const std::string& host, int port) {
        HTTP_INFO("HTTP server listening on {}:{}", host, port);
        _srv.listen(host, port);
    }

    // ── routes ────────────────────────────────────────────────────────────────────

    void server::register_routes() {

        _srv.Get("/api/ping", [this](const httplib::Request& req, httplib::Response& res) {
            send_json(res, "pong?");
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });

        _srv.Get("/api/v0/reading-types", [this](const httplib::Request& req, httplib::Response& res) {
            auto result = _repo.get_all("reading_types", repo::reading_type::db_mapper);
            if (!result) {
                send_error(res, "failed to fetch reading types");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            json arr = json::array();
            for (auto& t : *result){
                arr.push_back({{"id", t.id}, {"name", t.name}, {"created_at", t.created_at}});
            }

            send_json(res, arr);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });
        _srv.Get("/api/v0/condition-types", [this](const httplib::Request& req, httplib::Response& res) {
            auto result = _repo.get_all("condition_types", repo::condition_type::db_mapper);
            if (!result) {
                send_error(res, "failed to fetch condition types");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
            }

            json arr = json::array();
            for (auto& t : *result){
                arr.push_back({{"id", t.id}, {"name", t.name}, {"created_at", t.created_at}});
            }

            send_json(res, arr);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });
        _srv.Get("/api/v0/actuator-types", [this](const httplib::Request& req, httplib::Response& res) {
            auto result = _repo.get_all("actuator_types", repo::actuator_type::db_mapper);
            if (!result) {
                send_error(res, "failed to fetch actuator types");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            json arr = json::array();
            for (auto& t : *result){
                arr.push_back({{"id", t.id}, {"name", t.name}, {"created_at", t.created_at}});
            }

            send_json(res, arr);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });

        // GET /readings?type=<id>&from=<date>&to=<date>
        _srv.Get("/api/v0/readings", [this](const httplib::Request& req, httplib::Response& res) {
            std::optional<int>         type_id;
            std::optional<std::string> from;
            std::optional<std::string> to;

            if (req.has_param("type")) {
                if (auto parse_res = util::parse::to<int>(req.get_param_value("type")); parse_res){
                    type_id = parse_res.value();
                }

            }
            if (req.has_param("from")) from = req.get_param_value("from");
            if (req.has_param("to"))   to   = req.get_param_value("to");

            auto result = _repo.get_readings(type_id, from, to);
            if (!result) {
                send_error(res, "failed to fetch readings");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });
        _srv.Get("/api/v0/rules", [this](const httplib::Request& req, httplib::Response& res) {
            auto result = _repo.get_rules_ordered();
            if (!result) {
                send_error(res, "failed to fetch rules");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });
        _srv.Get("/api/v0/actuator-logs", [this](const httplib::Request& req, httplib::Response& res) {
            auto result = _repo.get_actuator_logs_ordered();
            if (!result) {
                send_error(res, "failed to fetch rules");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });

        // POST /rules  body: { id_reading_type, id_condition_type, id_actuator_type, condition_value }
        _srv.Post("/api/v0/rules", [this](const httplib::Request& req, httplib::Response& res) {
            json body;
            try { body = json::parse(req.body); }
            catch (...) {
                send_error(res, "invalid JSON", 400);
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            if (!body.contains("id_reading_type")   || !body.contains("id_condition_type") ||
                !body.contains("id_actuator_type")   || !body.contains("condition_value")) {
                    send_error(res, "missing required fields", 400);
                    HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                    return;
                }

            auto result = _repo.insert_rule(
                body["id_reading_type"].get<int>(),
                body["id_condition_type"].get<int>(),
                body["id_actuator_type"].get<int>(),
                body["condition_value"].get<double>()
            );
            if (!result) {
                send_error(res, "failed to insert rule");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }

            send_json(res, {{"ok", true}}, 201);
            HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
        });

        // DELETE /rules/:id
        _srv.Delete(R"(/api/v0/rules/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
            int id;
            if (auto parse_res = util::parse::to<int>(req.matches[1].str()); parse_res){
                id = parse_res.value();
            }
            else {
                return send_error(res, "invalid id", 400);
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
            }

            auto result = _repo.delete_rule(id);
            if (!result) {
                send_error(res, "failed to delete rule");
                HTTP_INFO(" | {} | > {} > GET '{}'", res.status, req.remote_addr, req.path);
                return;
            }
            send_json(res, {{"ok", true}});
        });
    }

} // namespace http
