#pragma once
#include <unordered_map>
#include <list>
#include <mutex>
#include <optional>
#include "../models/user.hpp"

using namespace std;

class LRU
{
    struct Node
    {
        string key;
        User value;
    };
    size_t capacity;
    list<Node> mru_list; // front = MRU
    unordered_map<string, list<Node>::iterator> mapById;
    unordered_map<string, string> mapByMobile; // mobile -> id
    mutex lock;

public:
    explicit LRU(size_t cap) : capacity(cap ? cap : 1) {}
    optional<User> get_by_id(const string &id);
    optional<User> get_by_mobile(const string &mobile);
    void put(const User &u);
};
