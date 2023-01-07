#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <unordered_map>

namespace brasa::pattern {

/**
 * A class that implements an object repository.
 * The objects are identified by a value of type \b TYPE_ID.
 * This class is not thread safe.
 */
template <typename BASE, typename TYPE_ID>
class ObjectFactory {
public:
    using creator_t = std::function<std::unique_ptr<BASE>()>;
    // Non-copyable and movable
    ObjectFactory() noexcept = default;
    ~ObjectFactory() noexcept = default;
    ObjectFactory(const ObjectFactory&) = delete;
    ObjectFactory(ObjectFactory&&) = default;
    ObjectFactory& operator=(const ObjectFactory&) = delete;
    ObjectFactory& operator=(ObjectFactory&&) = default;
    bool add(TYPE_ID id, creator_t creator);
    /**
     * Remove object identified by \b id.
     *
     * @param id    object identifier
     * @return if object was removed from the repository
     */
    bool remove(TYPE_ID id);
    /**
     * Get the object identified by \b id.
     *
     * @param id    object identifier
     * @return the object
     * @throw std::logic_error if no object exists for identifier \b id
     */
    std::unique_ptr<BASE> get(TYPE_ID id) const;
    /**
     * Return if object identified by \b id exists.
     *
     * @param id    object identifier
     * @return if object exists
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
    return it->second();
}

template <typename BASE, typename TYPE_ID>
bool ObjectFactory<BASE, TYPE_ID>::has(TYPE_ID id) const {
    std::shared_lock lock(mutex_);
    return creators_.find(id) != creators_.end();
}

} // namespace brasa::pattern
