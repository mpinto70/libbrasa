# Singletons

- [Singletons](#singletons)
  - [Simple singleton](#simple-singleton)
  - [Polymorphic singleton](#polymorphic-singleton)

[`Singleton`](./singleton/Singleton.h) **is thread safe**.

## Simple singleton

`Singleton` is a simple implementtion of the
[singleton pattern](https://en.wikipedia.org/wiki/Singleton_pattern). In order
to create a singleton of a type you simply call the appropriate
`create_instance` with the correct type and the arguments to construction:

```cpp
Singleton<MyClass>::create_instance(1, 2, "name");
```

In this example, an object of type `MyClass` is created and become the
singleton, `1, 2, "name"` are the parameters that will be passed to the
constructor of `MyClass`.

After creation (and before destruction by calling `free_instance`), you can get
a reference to the instance by calling the function (checks notes) `instance`:

```cpp
auto& instance = Singleton<MyClass>::instance();
```

## Polymorphic singleton

The `Singleton` class allows for polymorphic singletons also. In order to create
an instance of a derived class that will be retrieved by the base class, you
call the parameterized overload of `create_instance` informing the derived
class:

```cpp
Singleton<MyBase>::create_instance<MyDerived>("one", 2, THREE);
```

In this example, an object of class `MyDerived` is created with
`"one", 2, THREE` as parameters. After created, the singleton is retrieved the
same way as before:

```cpp
auto& instance = Singleton<MyBase>::instance();
```

Note that `Singleton<MyDerived>` is a totally different singleton. One big
advantage of the polymorphic behavior is that the code that uses the singleton,
don´t have to depend on `MyDerived`, allowing for a more decoupled design.

---

See [the singleton tests](/test/brasa/patterns/singleton/SingletonTest.cpp) for
some examples on how to create and use singletons.
