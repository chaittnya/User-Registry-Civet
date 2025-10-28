#pragma once
#include <string>
#include <sstream>
#include "../models/user.hpp"

// Escape a string for safe JSON output
inline std::string json_escape(const std::string &s) {
    std::string out; out.reserve(s.size()+8);
    for (unsigned char c : s) {
        switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 32) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else out += c;
        }
    }
    return out;
}

inline std::string json_kv(const char *k, const std::string &v, bool quote=true) {
    std::ostringstream os;
    os << "\"" << k << "\":";
    if (quote) os << "\"" << json_escape(v) << "\"";
    else os << v;
    return os.str();
}

inline std::string user_to_json(const User& u, const std::string& source = {}) {
    std::ostringstream os;
    os << "{";
    if (!source.empty()) {
        os << json_kv("source", source) << ",";
        os << "\"user\":{"
           << json_kv("id", u.id) << ","
           << json_kv("name", u.name) << ","
           << json_kv("mobile", u.mobile) << ","
           << json_kv("created_at", u.created_at)
           << "}";
    } else {
        os << json_kv("id", u.id) << ","
           << json_kv("name", u.name) << ","
           << json_kv("mobile", u.mobile) << ","
           << json_kv("created_at", u.created_at);
    }
    os << "}";
    return os.str();
}
