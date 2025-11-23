#include "cache.hpp"

using namespace std;

optional<User> Cache::get_by_id(const string &id)
{
    shared_lock<shared_mutex> rlock(rwlock); // allow concurrent readers

    auto it = mapById.find(id);
    if (it == mapById.end())
        return nullopt;

    // No reordering: FCFS/FIFO, just return the value
    return it->second->value;
}

void Cache::put(const User &u)
{
    unique_lock<shared_mutex> wlock(rwlock); // writers are exclusive

    auto it = mapById.find(u.id);
    if (it != mapById.end())
    {
        // Update existing entry, keep its position (FCFS)
        it->second->value = u;
        return;
    }

    // If full, evict the oldest (back of the list)
    if (queue.size() >= capacity)
    {
        auto &oldest = queue.back();
        mapById.erase(oldest.key);
        queue.pop_back();
    }

    // Insert new as "newest" at the front
    queue.push_front({u.id, u});
    mapById[u.id] = queue.begin();
}
