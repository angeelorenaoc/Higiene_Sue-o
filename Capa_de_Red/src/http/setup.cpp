#include "setup.hpp"

#include <nlohmann/json.hpp>

#include "server.hpp"
#include "../utils/parse.hpp"

namespace http {
    using json = nlohmann::json;

    void send_json(httplib::Response& res, const json& body, int status = 200) {
        res.status = status;
        res.set_content(body.dump(), "application/json");
    }
    void send_text(httplib::Response& res, const json& body, int status = 200) {
        res.status = status;
        res.set_content(body.dump(), "text/plain");
    }
    static void send_error(httplib::Response& res, const std::string& msg, int status = 500) {
        send_json(res, {{"error", msg}}, status);
    }

    void register_routes(server& s) {

        s.Get("/api/ping", [&](const httplib::Request& req, httplib::Response& res) {
            send_text(res, "pong?");
        });

        s.Get("/api/v0/reading-types", [&](const httplib::Request& req, httplib::Response& res) {
            auto result = s.repo.get_all("reading_types", repo::reading_type::db_mapper);
            if (!result) {
                send_error(res, "failed to fetch reading types");
                return;
            }

            json arr = json::array();
            for (auto& t : *result){
                arr.push_back({{"id", t.id}, {"name", t.name}, {"created_at", t.created_at}});
            }

            send_json(res, arr);
        });
        s.Get("/api/v0/condition-types", [&](const httplib::Request& req, httplib::Response& res) {
            auto result = s.repo.get_all("condition_types", repo::condition_type::db_mapper);
            if (!result) {
                send_error(res, "failed to fetch condition types");
            }

            json arr = json::array();
            for (auto& t : *result){
                arr.push_back({{"id", t.id}, {"name", t.name}, {"created_at", t.created_at}});
            }

            send_json(res, arr);
        });
        s.Get("/api/v0/actuator-types", [&](const httplib::Request& req, httplib::Response& res) {
            auto result = s.repo.get_all("actuator_types", repo::actuator_type::db_mapper);
            if (!result) {
                send_error(res, "failed to fetch actuator types");
                return;
            }

            json arr = json::array();
            for (auto& t : *result){
                arr.push_back({{"id", t.id}, {"name", t.name}, {"created_at", t.created_at}});
            }

            send_json(res, arr);
        });

        // GET /readings?type=<id>&from=<date>&to=<date>&limit=<count>
        s.Get("/api/v0/readings", [&](const httplib::Request& req, httplib::Response& res) {
            std::optional<int>          type_id;
            std::optional<std::string>  from;
            std::optional<std::string>  to;
            std::optional<int>          limit;

            if (req.has_param("type")) {
                if (auto parse_res = util::parse::to<int>(req.get_param_value("type")); parse_res){
                    type_id = parse_res.value();
                }
            }
            if (req.has_param("limit")) {
                if (auto parse_res = util::parse::to<int>(req.get_param_value("limit")); parse_res){
                    limit = parse_res.value();
                }
            }
            if (req.has_param("from")) from = req.get_param_value("from");
            if (req.has_param("to"))   to   = req.get_param_value("to");

            auto result = s.repo.get_readings(type_id, from, to, limit);
            if (!result) {
                send_error(res, "failed to fetch readings");
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
        });
        s.Get("/api/v0/rules", [&](const httplib::Request& req, httplib::Response& res) {
            auto result = s.repo.get_rules_ordered();
            if (!result) {
                send_error(res, "failed to fetch rules");
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
        });
        s.Get("/api/v0/time-rules", [&](const httplib::Request& req, httplib::Response& res) {
            auto result = s.repo.get_time_rules_ordered();
            if (!result) {
                send_error(res, "failed to fetch rules");
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
        });
        s.Get("/api/v0/actuator-logs", [&](const httplib::Request& req, httplib::Response& res) {
            auto result = s.repo.get_actuator_logs_ordered();
            if (!result) {
                send_error(res, "failed to fetch rules");
                return;
            }

            json arr = json::array();
            for (auto& r : *result) arr.push_back(r.to_json());
            send_json(res, arr);
        });

        // POST /rules  body: { id_reading_type, id_condition_type, id_actuator_type, condition_value }
        s.Post("/api/v0/rules", [&](const httplib::Request& req, httplib::Response& res) {
            json body;
            try { body = json::parse(req.body); }
            catch (...) {
                send_error(res, "invalid JSON", 400);
                return;
            }

            if (!body.contains("id_reading_type")   || !body.contains("id_condition_type") ||
                !body.contains("id_actuator_type")   || !body.contains("condition_value")) {
                    send_error(res, "missing required fields", 400);
                    return;
                }

            auto result = s.repo.insert_rule(
                body["id_reading_type"].get<int>(),
                body["id_condition_type"].get<int>(),
                body["id_actuator_type"].get<int>(),
                body["condition_value"].get<double>()
            );
            if (!result) {
                send_error(res, "failed to insert rule");
                return;
            }

            send_json(res, {{"ok", true}}, 201);
        });
        s.Post("/api/v0/time-rules", [&](const httplib::Request& req, httplib::Response& res) {
            json body;
            try { body = json::parse(req.body); }
            catch (...) {
                send_error(res, "invalid JSON", 400);
                return;
            }

            if (!body.contains("id_actuator_type") || !body.contains("id_condition_type")
                || !body.contains("condition_time")) {
                    send_error(res, "missing required fields", 400);
                    return;
                }

            auto result = s.repo.insert_time_rule(
                body["id_condition_type"].get<int>(),
                body["id_actuator_type"].get<int>(),
                body["condition_time"].get<std::string>()
            );
            if (!result) {
                send_error(res, "failed to insert rule");
                return;
            }

            send_json(res, {{"ok", true}}, 201);
        });

        // DELETE /rules/:id
        s.Delete(R"(/api/v0/rules/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
            int id;
            if (auto parse_res = util::parse::to<int>(req.matches[1].str()); parse_res){
                id = parse_res.value();
            }
            else {
                return send_error(res, "invalid id", 400);
            }

            auto result = s.repo.delete_rule(id);
            if (!result) {
                send_error(res, "failed to delete rule");
                return;
            }

            send_json(res, {{"ok", true}});
        });
        s.Delete(R"(/api/v0/time-rules/(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
            int id;
            if (auto parse_res = util::parse::to<int>(req.matches[1].str()); parse_res){
                id = parse_res.value();
            }
            else {
                return send_error(res, "invalid id", 400);
            }

            auto result = s.repo.soft_delete("time_rules", id);
            if (!result) {
                send_error(res, "failed to delete rule");
                return;
            }

            send_json(res, {{"ok", true}});
        });

        // SSE endpoint
        s.Get("/api/v0/events", [&](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Content-Type", "text/event-stream");

            res.set_chunked_content_provider("text/event-stream",
            [](size_t, httplib::DataSink&) {  return true; /* keep connection open */ },
            [](bool){}
            );
        });
    }
}
