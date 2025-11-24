#include "cache.hpp"

using namespace std;

optional<User> Cache::get_by_id(const string &id)
{
    shared_lock<shared_mutex> rlock(rwlock);

    auto it = mapById.find(id);
    if (it == mapById.end())
        return nullopt;
    return it->second->value;
}

void Cache::put(const User &u)
{
    unique_lock<shared_mutex> wlock(rwlock);

    auto it = mapById.find(u.id);
    if (it != mapById.end())
    {
        it->second->value = u;
        return;
    }

    if (queue.size() >= capacity)
    {
        auto &oldest = queue.back();
        mapById.erase(oldest.key);
        queue.pop_back();
    }
    queue.push_front({u.id, u});
    mapById[u.id] = queue.begin();
}
