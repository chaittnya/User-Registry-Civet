#pragma once
#include "../cache/lru.hpp"

struct App {
    LRU cache{1024};
};
