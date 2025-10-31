#pragma once
#include <string>
#include <cctype>
#include <optional>
#include <sstream>
#include <civetweb.h>
#include <nlohmann/json.hpp>

using namespace std;
using Json = nlohmann::json;

inline string query_param(const mg_request_info *ri, const char *key)
{
    if (!ri || !ri->query_string)
        return {};
    char buf[128];
    int len = mg_get_var(ri->query_string, strlen(ri->query_string),
                         key, buf, sizeof(buf));
    if (len < 0)
        return {};
    return string(buf, len);
}

inline string read_body(mg_connection *conn)
{
    const mg_request_info *ri = mg_get_request_info(conn);
    if (!ri || ri->content_length <= 0)
        return {};

    string body(static_cast<size_t>(ri->content_length), '\0');
    int read_bytes = mg_read(conn, body.data(), body.size());
    body.resize(max(0, read_bytes));
    return body;
}

inline void write_json(mg_connection *conn, int status, const Json &j)
{
    string body = j.dump();
    mg_printf(conn,
              "HTTP/1.1 %d OK\r\n"
              "Content-Type: application/json\r\n"
              "Content-Length: %zu\r\n"
              "Connection: close\r\n\r\n",
              status, body.size());
    mg_write(conn, body.c_str(), body.size());
}
