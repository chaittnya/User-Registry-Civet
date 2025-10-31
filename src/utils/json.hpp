#pragma once
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "../models/user.hpp"

using namespace std;

using Json = nlohmann::json;

optional<string> get_key_from_json(const string &body, const char *key)
{
    try
    {
        auto j = Json::parse(body);
        if (j.contains(key) && j[key].is_string())
            return j[key].get<string>();
    }
    catch (...)
    {
        return nullopt;
    }
    return nullopt;
}

inline Json user_to_json(const User &u, const string &source = {})
{
    Json j = {
        {"source", source},
        {"user", {{"id", u.id}, {"name", u.name}, {"mobile", u.mobile}, {"created_at", u.created_at}}}};
    return j;
}

template <class T>
optional<T> json_get(const nlohmann::json &j, string_view key)
{
    auto it = j.find(string(key));
    if (it == j.end())
        return nullopt;
    try
    {
        return it->get<T>();
    }
    catch (const Json::exception &)
    {
        return nullopt;
    }
}

template <class T>
T json_get_or(const nlohmann::json &j, string_view key, T defv)
{
    if (auto v = json_get<T>(j, key))
        return *v;
    return defv;
}