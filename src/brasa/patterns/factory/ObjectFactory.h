#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <unordered_map>

namespace brasa::pattern {

/**
 * A class that implements an object factory.
 * Objects are created by registered creator functions identified by a value of type \b TYPE_ID.
 * This class is thread safe.
 */
template <typename BASE, typename TYPE_ID>
class ObjectFactory final {
public:
    using creator_t = std::function<std::unique_ptr<BASE>()>;
    // Non-copyable, non-movable
    ObjectFactory() noexcept = default;
    ~ObjectFactory() noexcept = default;
    ObjectFactory(const ObjectFactory&) = delete;
    ObjectFactory(ObjectFactory&&) = delete;
    ObjectFactory& operator=(const ObjectFactory&) = delete;
    ObjectFactory& operator=(ObjectFactory&&) = delete;
    /**
     * Register a creator function for objects identified by \b id.
     *
     * @param id      object identifier
     * @param creator function that creates a new instance of BASE
     * @return true if added, false if a creator for that id already existed
     */
    bool add(TYPE_ID id, creator_t creator);
    /**
     * Remove the creator registered for \b id.
     *
     * @param id    object identifier
     * @return true if removed, false if no creator existed for identifier \b id
     */
    bool remove(TYPE_ID id);
    /**
     * Create and return a new object using the creator registered for \b id.
     *
     * @param id    object identifier
     * @return a newly created object
     * @throw std::logic_error if no creator is registered for identifier \b id
     */
    std::unique_ptr<BASE> get(TYPE_ID id) const;
    /**
     * Returns whether a creator is registered for \b id.
     *
     * @param id    object identifier
     * @return true if a creator exists, false otherwise
     */
    bool has(TYPE_ID id) const;

private:
    std::unordered_map<TYPE_ID, creator_t> creators_;
    mutable std::shared_mutex mutex_;
};

template <typename BASE, typename TYPE_ID>
bool ObjectFactory<BASE, TYPE_ID>::add(TYPE_ID id, creator_t creator) {
    std::unique_lock lock(mutex_);
    return creators_.insert({ id, std::move(creator) }).second;
}

template <typename BASE, typename TYPE_ID>
bool ObjectFactory<BASE, TYPE_ID>::remove(TYPE_ID id) {
    std::unique_lock lock(mutex_);
    return creators_.erase(id) > 0;
}

template <typename BASE, typename TYPE_ID>
std::unique_ptr<BASE> ObjectFactory<BASE, TYPE_ID>::get(TYPE_ID id) const {
    std::shared_lock lock(mutex_);
    auto it = creators_.find(id);
    if (it == creators_.end()) {
        using std::to_string;
        using namespace std::string_literals;
        throw std::logic_error(
              "brasa::pattern::ObjectFactory::get object not found for "s + typeid(BASE).name()
              + " and id " + to_string(id));
    }
    auto creator = it->second;
    lock.unlock();
    return creator();
}

template <typename BASE, typename TYPE_ID>
bool ObjectFactory<BASE, TYPE_ID>::has(TYPE_ID id) const {
    std::shared_lock lock(mutex_);
    return creators_.find(id) != creators_.end();
}

} // namespace brasa::pattern
