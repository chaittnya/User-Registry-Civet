#pragma once
#include "../db/db.hpp"
#include "../cache/lru.hpp"

struct App {
    Pg  db;
    LRU cache{1024};
};
