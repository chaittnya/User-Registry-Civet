#include "lru.hpp"

using namespace std;

optional<User> LRU::get_by_id(const string &id)
{
    lock_guard<mutex> l(lock);
    auto it = mapById.find(id);
    if (it == mapById.end())
        return nullopt;
    mru_list.splice(mru_list.begin(), mru_list, it->second);
    return it->second->value;
}

optional<User> LRU::get_by_mobile(const string &mobile)
{
    lock_guard<mutex> l(lock);
    auto i = mapByMobile.find(mobile);
    if (i == mapByMobile.end())
        return nullopt;
    auto j = mapById.find(i->second);
    if (j == mapById.end())
    {
        mapByMobile.erase(i);
        return nullopt;
    }
    mru_list.splice(mru_list.begin(), mru_list, j->second);
    return j->second->value;
}

void LRU::put(const User &u)
{
    lock_guard<mutex> l(lock);
    auto it = mapById.find(u.id);
    if (it != mapById.end())
    {
        it->second->value = u;
        mapByMobile[u.mobile] = u.id;
        mru_list.splice(mru_list.begin(), mru_list, it->second);
        return;
    }
    mru_list.push_front({u.id, u});
    mapById[u.id] = mru_list.begin();
    mapByMobile[u.mobile] = u.id;
    if (mru_list.size() > capacity)
    {
        auto &tail = mru_list.back();
        mapByMobile.erase(tail.value.mobile);
        mapById.erase(tail.key);
        mru_list.pop_back();
    }
}
