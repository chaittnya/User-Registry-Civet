#pragma once
#include <unordered_map>
#include <list>
#include <mutex>
#include <optional>
#include "../models/user.hpp"

class LRU {
    struct Node { std::string key; User value; };
    size_t cap_;
    std::list<Node> list_; // front = MRU
    std::unordered_map<std::string, std::list<Node>::iterator> byId_;
    std::unordered_map<std::string, std::string> byMobile_; // mobile -> id
    std::mutex mu_;
public:
    explicit LRU(size_t cap): cap_(cap?cap:1) {}
    std::optional<User> getById(const std::string& id);
    std::optional<User> getByMobile(const std::string& mobile);
    void put(const User& u);
};
