#include "lru.hpp"

std::optional<User> LRU::getById(const std::string &id) {
    std::lock_guard<std::mutex> g(mu_);
    auto it = byId_.find(id);
    if (it == byId_.end()) return std::nullopt;
    list_.splice(list_.begin(), list_, it->second);
    return it->second->value;
}

std::optional<User> LRU::getByMobile(const std::string &mobile) {
    std::lock_guard<std::mutex> g(mu_);
    auto i = byMobile_.find(mobile);
    if (i == byMobile_.end()) return std::nullopt;
    auto j = byId_.find(i->second);
    if (j == byId_.end()) { byMobile_.erase(i); return std::nullopt; }
    list_.splice(list_.begin(), list_, j->second);
    return j->second->value;
}

void LRU::put(const User &u) {
    std::lock_guard<std::mutex> g(mu_);
    auto it = byId_.find(u.id);
    if (it != byId_.end()) {
        it->second->value = u;
        byMobile_[u.mobile] = u.id;
        list_.splice(list_.begin(), list_, it->second);
        return;
    }
    list_.push_front({u.id, u});
    byId_[u.id] = list_.begin();
    byMobile_[u.mobile] = u.id;
    if (list_.size() > cap_) {
        auto &tail = list_.back();
        byMobile_.erase(tail.value.mobile);
        byId_.erase(tail.key);
        list_.pop_back();
    }
}
