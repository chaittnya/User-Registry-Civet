#include "users_handler.hpp"
#include <string>
#include<iostream>
#include <optional>
#include <cstring> 
#include "../utils/strutils.hpp"
#include "../utils/json.hpp"
#include "../token/pbkdf2.hpp"

bool UsersHandler::handlePost(CivetServer*, mg_connection* conn) {
    const mg_request_info* ri = mg_get_request_info(conn);
    std::string path = ri->local_uri ? ri->local_uri : "";

    if (path == "/users") {
        auto body = read_body(conn);
        auto name   = json_extract_string(body, "name");
        auto mobile = json_extract_string(body, "mobile");
        if (!name || !mobile || name->empty() || mobile->empty()) {
            write_json(conn, 400, "{\"error\":\"name, mobile required\"}");
            return true;
        }

        auto u = app_->db.create_user(*name, *mobile);
        if (!u) { write_json(conn, 500, "{\"error\":\"db error\"}"); return true; }

        app_->cache.put(*u);
        write_json(conn, 201, user_to_json(*u));
        return true;
    }

    if (path == "/token") {
        auto body = read_body(conn);
        auto mobile = json_extract_string(body, "mobile");
        int iters   = json_extract_int(body, "iterations", 700000);
        if (!mobile || mobile->empty()) {
            write_json(conn, 400, "{\"error\":\"mobile required\"}");
            return true;
        }
        auto tok = derive_token(*mobile, iters);
        std::ostringstream os;
        os << "{"
           << json_kv("mobile", *mobile) << ","
           << json_kv("iterations", std::to_string(iters), /*quote*/false) << ","
           << json_kv("token", tok)
           << "}";
        write_json(conn, 200, os.str());
        return true;
    }

    write_json(conn, 404, "{\"error\":\"not found\"}");
    return true;
}

bool UsersHandler::handleGet(CivetServer*, mg_connection* conn) {
    const mg_request_info* ri = mg_get_request_info(conn);
    std::string path = ri->local_uri ? ri->local_uri : "";

    if (path.rfind("/users/by-phone", 0) == 0) {
        std::string mobile = query_param(ri, "mobile");
        std::cout<<mobile<<std::endl;
        if (mobile.empty()) {
            write_json(conn, 400, "{\"error\":\"missing ?mobile= param\"}");
            return true;
        }
        if (auto cu = app_->cache.getByMobile(mobile)) {
            write_json(conn, 200, user_to_json(*cu, "cache"));
            return true;
        }
        auto u = app_->db.get_user_by_mobile(mobile);
        if (!u) { write_json(conn, 404, "{\"error\":\"not found\"}"); return true; }
        app_->cache.put(*u);
        write_json(conn, 200, user_to_json(*u, "db"));
        return true;
    }

    if (path.rfind("/users/", 0) == 0) {
        std::string id = path.substr(std::string("/users/").size());
        if (id.empty()) { write_json(conn, 400, "{\"error\":\"missing id\"}"); return true; }

        if (auto cu = app_->cache.getById(id)) {
            write_json(conn, 200, user_to_json(*cu, "cache"));
            return true;
        }
        auto u = app_->db.get_user_by_id(id);
        if (!u) { write_json(conn, 404, "{\"error\":\"not found\"}"); return true; }
        app_->cache.put(*u);
        write_json(conn, 200, user_to_json(*u, "db"));
        return true;
    }

    if (path == "/healthz") {
        const char* ok = "ok";
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s", strlen(ok), ok);
        return true;
    }

    write_json(conn, 404, "{\"error\":\"not found\"}");
    return true;
}
