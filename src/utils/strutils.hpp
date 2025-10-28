#pragma once
#include <string>
#include <cctype>
#include <optional>
#include <sstream>
#include <civetweb.h>

// URL decode "percent-encoding"
inline std::string url_decode(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (size_t i=0;i<s.size();++i) {
        if (s[i]=='%' && i+2<s.size()) {
            int v = 0;
            sscanf(s.substr(i+1,2).c_str(), "%x", &v);
            out.push_back(static_cast<char>(v)); i+=2;
        } else if (s[i]=='+') out.push_back(' ');
        else out.push_back(s[i]);
    }
    return out;
}

// Extract a query parameter from CivetWeb's mg_request_info
inline std::string query_param(const mg_request_info* ri, const char* key) {
    if (!ri || !ri->query_string) return {};
    std::string q = ri->query_string;
    std::string k = std::string(key) + "=";
    size_t p = q.find(k);
    if (p == std::string::npos) return {};
    p += k.size();
    size_t e = q.find('&', p);
    return url_decode(q.substr(p, e==std::string::npos ? std::string::npos : e-p));
}

// Read request body into string
inline std::string read_body(mg_connection* conn) {
    const mg_request_info* ri = mg_get_request_info(conn);
    long long cl = (ri ? ri->content_length : -1);

    std::string body;
    if (cl > 0) {
        body.resize(static_cast<size_t>(cl));
        size_t total = 0;
        while (total < body.size()) {
            int r = mg_read(conn, body.data() + total, body.size() - total);
            if (r <= 0) break;
            total += static_cast<size_t>(r);
        }
        body.resize(total);
    }
    return body;
}


// Write JSON response with status code
inline void write_json(mg_connection* conn, int status, const std::string& json) {
    std::ostringstream hdr;
    hdr << "HTTP/1.1 " << status << " OK\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << json.size() << "\r\n"
        << "Connection: close\r\n\r\n";
    mg_write(conn, hdr.str().c_str(), hdr.str().size());
    mg_write(conn, json.c_str(), json.size());
}

inline std::optional<std::string> json_extract_string(const std::string& body, const char* key) {
    std::string pat = "\"" + std::string(key) + "\"";
    size_t p = body.find(pat);
    if (p == std::string::npos) return std::nullopt;
    p = body.find(':', p);
    if (p == std::string::npos) return std::nullopt;
    while (p < body.size() && (body[p]==':' || body[p]==' ')) ++p;
    if (p >= body.size() || body[p] != '\"') return std::nullopt;
    ++p;
    std::ostringstream val;
    while (p < body.size()) {
        char c = body[p++];
        if (c == '\\') { if (p < body.size()) { val << body[p++]; } }
        else if (c == '\"') break;
        else val << c;
    }
    return val.str();
}

inline int json_extract_int(const std::string& body, const char* key, int defv) {
    std::string pat = "\"" + std::string(key) + "\"";
    size_t p = body.find(pat);
    if (p == std::string::npos) return defv;
    p = body.find(':', p);
    if (p == std::string::npos) return defv;
    ++p;
    while (p < body.size() && body[p]==' ') ++p;
    bool neg=false; if (p < body.size() && body[p]=='-') {neg=true; ++p;}
    long long v=0;
    while (p < body.size() && std::isdigit(static_cast<unsigned char>(body[p]))) {
        v = (long long) v*10 + (body[p]-'0'); ++p;
    }
    return neg ? -int(v) : int(v);
}
