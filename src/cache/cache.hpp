#pragma once
#include <unordered_map>
#include <list>
#include <mutex> 
#include <shared_mutex>
#include <optional>
#include "../models/user.hpp"

using namespace std;

class Cache 
{
    struct Node
    {
        string key;
        User value;
    };

    size_t capacity;
    list<Node> queue;
    unordered_map<string, list<Node>::iterator> mapById;

    mutable shared_mutex rwlock;

public:
    explicit Cache(size_t cap) : capacity(cap ? cap : 1024) {}

    optional<User> get_by_id(const string &id);
    void put(const User &u);
};

