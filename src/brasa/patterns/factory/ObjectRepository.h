#pragma once

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace brasa::pattern {

/** A class that implements an object repository. */
template <typename BASE, typename TYPE_ID>
class ObjectRepository {
public:
    template <typename U>
    bool add(TYPE_ID id, std::unique_ptr<U> u);
    template <typename U, typename... ARGS>
    bool add(TYPE_ID id, ARGS&&... args);

    BASE& get(TYPE_ID id);

private:
    std::unordered_map<TYPE_ID, std::unique_ptr<BASE>> objects_;
};

template <typename BASE, typename TYPE_ID>
template <typename U>
bool ObjectRepository<BASE, TYPE_ID>::add(TYPE_ID id, std::unique_ptr<U> u) {
    return objects_.insert({ id, std::move(u) }).second;
}

template <typename BASE, typename TYPE_ID>
template <typename U, typename... ARGS>
bool ObjectRepository<BASE, TYPE_ID>::add(TYPE_ID id, ARGS&&... args) {
    return add(id, std::make_unique<U>(std::forward<ARGS>(args)...));
}

template <typename BASE, typename TYPE_ID>
BASE& ObjectRepository<BASE, TYPE_ID>::get(TYPE_ID id) {
    auto it = objects_.find(id);
    if (it == objects_.end()) {
        using std::to_string;
        using namespace std::string_literals;
        throw std::logic_error("brasa::pattern::ObjectRepository::get object not found for "s
                               + typeid(BASE).name() + " and id "
                               + to_string(id));
    }
    return *it->second;
}

}
