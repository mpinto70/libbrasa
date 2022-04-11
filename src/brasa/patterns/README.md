# Design patterns

In tihs directory are implementations of some design patterns.

| class                                              |   is thread safe   |
| :------------------------------------------------- | :----------------: |
| [`Singleton`](./singleton/Singleton.h)             | :heavy_check_mark: |
| [`ObjectRepository`](./factory/ObjectRepository.h) |        :x:         |
| [`ObjectFactory`](./factory/ObjectFactory.h)       |        :x:         |

## Singleton

[`Singleton`](./singleton/Singleton.h) is a simple **thread safe** implementtion of a singleton. In
order to create a singleton of a type you simply put it as the template parameter:
`Singleton<MyClass>::instance()`.

The class allows for polymorphic singletons also. See
[tests](/test/brasa/patterns/singleton/SingletonTest.cpp) for some code that creates a singleton of
`Base` and the instance is `Derived`.

## Factories

### Object repository

[`ObjectRepository`](./factory/ObjectRepository.h) is a class that holds instances of objects that
are identified by a hashable key. The class is parameterized by the base class of all objects `BASE`
and the type of the key `TYPE_ID`. Every object in the repository has to be of class `BASE` or one
of its descendants. Objects are added by calling `add`, removed by calling `remove`, and retrived by calling `get`.

This class **is not thread safe**.

## Object factory

[`ObjectFactory`](./factory/ObjectFactory.h) is a class that holds creators of objects of differnt
types that are identified by a hashable key. The class is parameterized by the base class of all
objects created `BASE` and the type of the key `TYPE_ID`. Every creator in the factory to return a
unique pointer to an object of type `BASE` or one of its descendants. Creators are added by calling
`add` and removed by calling `remove`. In order to receive a new object, `get` has to be called.

This class **is not thread safe**.
