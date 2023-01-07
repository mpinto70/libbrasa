#pragma once

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
class ObjectRepository {
public:
    // Non-copyable and movable
    ObjectRepository() noexcept = default;
    ~ObjectRepository() noexcept = default;
    ObjectRepository(const ObjectRepository&) = delete;
    ObjectRepository(ObjectRepository&&) = default;
    ObjectRepository& operator=(const ObjectRepository&) = delete;
    ObjectRepository& operator=(ObjectRepository&&) = default;
    /**
     * Create a new object inside the repository, constructing it with args.
     *
     * @tparam U    type of concrete object derived from T
     * @param id    object identifier
     * @param args  args passed to constructor
     * @return if object was added to the repository
     */
    template <typename U, typename... ARGS>
    bool add(TYPE_ID id, ARGS&&... args);
    /**
     * Add an object to the repository indexed by \b id.
     *
     * @tparam U    type of concrete object derived from T
     * @param id    object identifier
     * @param u     the object
     * @return if object was added to the repository
     */
    template <typename U>
    bool add(TYPE_ID id, std::unique_ptr<U> u);
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
    BASE& get(TYPE_ID id);
    /**
     * Return if object identified by \b id exists.
     *
     * @param id    object identifier
     * @return if object exists
     */
    bool has(TYPE_ID id) const;

private:
    std::unordered_map<TYPE_ID, std::unique_ptr<BASE>> objects_;
    mutable std::shared_mutex mutex_;
};

template <typename BASE, typename TYPE_ID>
template <typename U>
bool ObjectRepository<BASE, TYPE_ID>::add(TYPE_ID id, std::unique_ptr<U> u) {
    std::unique_lock lock(mutex_);
    return objects_.insert({ id, std::move(u) }).second;
}

template <typename BASE, typename TYPE_ID>
template <typename U, typename... ARGS>
bool ObjectRepository<BASE, TYPE_ID>::add(TYPE_ID id, ARGS&&... args) {
    return add(id, std::make_unique<U>(std::forward<ARGS>(args)...));
}

template <typename BASE, typename TYPE_ID>
bool ObjectRepository<BASE, TYPE_ID>::remove(TYPE_ID id) {
    std::unique_lock lock(mutex_);
    return objects_.erase(id) > 0;
}

template <typename BASE, typename TYPE_ID>
BASE& ObjectRepository<BASE, TYPE_ID>::get(TYPE_ID id) {
    std::shared_lock lock(mutex_);
    auto it = objects_.find(id);
    if (it == objects_.end()) {
        using std::to_string;
        using namespace std::string_literals;
        throw std::logic_error(
              "brasa::pattern::ObjectRepository::get object not found for "s + typeid(BASE).name()
              + " and id " + to_string(id));
    }
    return *it->second;
}

template <typename BASE, typename TYPE_ID>
bool ObjectRepository<BASE, TYPE_ID>::has(TYPE_ID id) const {
    std::shared_lock lock(mutex_);
    return objects_.find(id) != objects_.end();
}

} // namespace brasa::pattern
