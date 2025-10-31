#include "users_handler.hpp"
#include <string>
#include <iostream>
#include <optional>
#include <cstring>

#include "../db/db.hpp"
#include "../utils/strutils.hpp"
#include "../utils/apiutils.hpp"
#include "../utils/json.hpp"
#include "../token/pbkdf2.hpp"

using namespace std;
using Json = nlohmann::json;

bool UsersHandler::handlePost(CivetServer *, mg_connection *conn)
{
    const mg_request_info *ri = mg_get_request_info(conn);
    string path = ri->local_uri ? ri->local_uri : "";

    auto body = Json::parse(read_body(conn));

    if (path == "/users")
    {
        // auto body = read_body(conn);
        // auto nm = get_key_from_json(body, "name");
        // cout << *nm <<endl;
        string name = json_get_or<string>(body, "name", "");
        string mobile = json_get_or<string>(body, "mobile", "");
        if (name.empty() || mobile.empty())
        {
            write_json(conn, 400, {{"error", "name, mobile required"}});
            return true;
        }

        auto u = Pg::instance().create_user(name, mobile);
        if (!u)
        {
            write_json(conn, 500, {{"error", "db error"}});
            return true;
        }

        app_->cache.put(*u);
        write_json(conn, 201, user_to_json(*u));
        return true;
    }

    if (path == "/token")
    {
        // auto body = read_body(conn);
        string mobile = json_get_or<string>(body, "mobile", "");
        int iters = json_get_or<int>(body, "iterations", 700000);
        // cout<<mobile<<" "<<iters<<endl;
        if (mobile.empty())
        {
            write_json(conn, 400, {{"error", "mobile required"}});
            return true;
        }
        auto tok = derive_token(mobile, iters);
        Json response = {
            {"mobile", mobile},
            {"iterations", iters},
            {"token", tok}};
        write_json(conn, 200, response);
        return true;
    }

    write_json(conn, 404, {{"error", "not found"}});
    return true;
}

bool UsersHandler::handleGet(CivetServer *, mg_connection *conn)
{
    const mg_request_info *ri = mg_get_request_info(conn);
    string path = ri->local_uri ? ri->local_uri : "";

    if (path.rfind("/users/by-phone", 0) == 0)
    {
        string mobile = query_param(ri, "mobile");
        cout << mobile << endl;
        if (mobile.empty())
        {
            write_json(conn, 400, {{"error", "missing ?mobile= param"}});
            return true;
        }

        if (auto cu = app_->cache.get_by_mobile(mobile))
        {
            write_json(conn, 200, user_to_json(*cu, "cache"));
            return true;
        }

        auto u = Pg::instance().get_user_by_mobile(mobile);
        if (!u)
        {
            write_json(conn, 404, {{"error", "not found"}});
            return true;
        }
        app_->cache.put(*u);
        write_json(conn, 200, user_to_json(*u, "db"));
        return true;
    }

    if (path == "/users/random")
    {
        const mg_request_info *ri = mg_get_request_info(conn);
        string n_str = query_param(ri, "n");

        if (n_str.empty())
        {
            write_json(conn, 400, {{"error", "missing n"}});
            return true;
        }

        int n = 0;
        try
        {
            n = std::stoi(n_str);
        }
        catch (...)
        {
            write_json(conn, 400, {{"error", "invalid n"}});
            return true;
        }

        if (n <= 0)
        {
            write_json(conn, 400, {{"error", "n must be positive"}});
            return true;
        }

        auto ids = Pg::instance().get_random_user_ids(n);
        Json arr = Json::array();
        for (auto &id : ids)
            arr.push_back(id);

        write_json(conn, 200, {{"requested", n}, {"returned", static_cast<int>(ids.size())}, {"ids", arr}});

        return true;
    }

    if (path.rfind("/users/", 0) == 0)
    {
        string id = path.substr(string("/users/").size());
        if (id.empty())
        {
            write_json(conn, 400, {{"error", "missing id"}});
            return true;
        }

        if (auto cu = app_->cache.get_by_id(id))
        {
            write_json(conn, 200, user_to_json(*cu, "cache"));
            return true;
        }
        auto u = Pg::instance().get_user_by_id(id);
        if (!u)
        {
            write_json(conn, 404, {{"error", "not found"}});
            return true;
        }
        app_->cache.put(*u);
        write_json(conn, 200, user_to_json(*u, "db"));
        return true;
    }

    if (path == "/healthz")
    {
        const char *ok = "ok";
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n%s", strlen(ok), ok);
        return true;
    }

    write_json(conn, 404, {{"error", "not found"}});
    return true;
}
