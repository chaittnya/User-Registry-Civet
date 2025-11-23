#pragma once
#include <unordered_map>
#include <list>
#include <mutex> 
#include <shared_mutex>   // C++17
#include <optional>
#include "../models/user.hpp"

using namespace std;

class Cache  // now FCFS/FIFO cache based on id
{
    struct Node
    {
        string key; // id
        User value;
    };

    size_t capacity;
    list<Node> queue; // front = newest, back = oldest (for eviction)
    unordered_map<string, list<Node>::iterator> mapById;

    mutable shared_mutex rwlock; // shared for reads, unique for writes

public:
    explicit Cache(size_t cap) : capacity(cap ? cap : 1) {}

    optional<User> get_by_id(const string &id);
    void put(const User &u);
};

