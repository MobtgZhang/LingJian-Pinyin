#pragma once

#include <list>
#include <unordered_map>
#include <cstddef>
#include <optional>

namespace core {

template <typename Key, typename Value>
class LruCache {
public:
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    std::optional<Value> get(const Key &key) const {
        auto it = map_.find(key);
        if (it == map_.end()) return std::nullopt;
        order_.splice(order_.end(), order_, it->second);
        return it->second->second;
    }

    void put(const Key &key, Value value) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = std::move(value);
            order_.splice(order_.end(), order_, it->second);
            return;
        }
        if (map_.size() >= capacity_) {
            map_.erase(order_.front().first);
            order_.pop_front();
        }
        order_.emplace_back(key, std::move(value));
        map_[key] = std::prev(order_.end());
    }

private:
    std::size_t capacity_;
    using Entry = std::pair<Key, Value>;
    mutable std::list<Entry> order_;
    std::unordered_map<Key, typename std::list<Entry>::iterator> map_;
};

} // namespace core
