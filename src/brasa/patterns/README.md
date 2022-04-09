# Design patterns

## Singleton

[`Singleton`](./singleton/Singleton.h) is a simple thread safe implementtion of a singleton. In
order to create a singleton of a type you simply put it as the template parameter:
`Singleton<MyClass>::instance()`.

The class allows for polymorphic singletons also. See
[tests](/test/brasa/patterns/singleton/SingletonTest.cpp) for some code that creates a singleton of
`Base` and the instance is `Derived`.
