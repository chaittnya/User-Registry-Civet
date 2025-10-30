#pragma once
#include <string>
#include <cctype>
#include <optional>
#include <sstream>
#include <civetweb.h>

using namespace std;

inline string url_decode(const string& s) {
    string out(s.size(), '\0');
    int len = mg_url_decode(s.c_str(), s.size(), out.data(), out.size(), 1);
    out.resize(len);
    return out;
}
