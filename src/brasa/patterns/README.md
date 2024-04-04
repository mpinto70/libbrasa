# Design patterns

- [Design patterns](#design-patterns)
  - [Singleton](#singleton)
  - [Factories](#factories)
    - [Object repository](#object-repository)
    - [Object factory](#object-factory)

In tihs directory are implementations of some design patterns.

| class                                              |   is thread safe   |
| :------------------------------------------------- | :----------------: |
| [`Singleton`](./singleton/Singleton.h)             | :heavy_check_mark: |
| [`ObjectRepository`](./factory/ObjectRepository.h) | :heavy_check_mark: |
| [`ObjectFactory`](./factory/ObjectFactory.h)       | :heavy_check_mark: |

## Singleton

See [singleton documentation](./singleton/README.md) for details.

## Factories

### Object repository

[`ObjectRepository`](./factory/ObjectRepository.h) is a class that holds
instances of objects that are identified by a hashable key. The class is
parameterized by the base class of all objects `BASE` and the type of the key
`TYPE_ID`. Every object in the repository has to be of class `BASE` or one of
its descendants. Objects are added by calling `add`, removed by calling
`remove`, and retrived by calling `get`.

This class **is thread safe**.

### Object factory

[`ObjectFactory`](./factory/ObjectFactory.h) is a class that holds creators of
objects of differnt types that are identified by a hashable key. The class is
parameterized by the base class of all objects created `BASE` and the type of
the key `TYPE_ID`. Every creator in the factory to return a unique pointer to an
object of type `BASE` or one of its descendants. Creators are added by calling
`add` and removed by calling `remove`. In order to receive a new object, `get`
has to be called.

This class **is thread safe**.
