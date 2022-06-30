# Singletons

`Singleton` **is thread safe**.

## Simple singleton

[`Singleton`](./singleton/Singleton.h) is a simple implementtion of a singleton. In order to create
a singleton of a type you simply call the appropriate `create_instance` with the correct type and
the arguments to construction:

```cpp
Singleton<MyClass>::create_instance(1, 2, "name");
```

In this example, `1, 2, "name"` are the parameters that will be passed to the constructor of `MyClass`.

After creation (and before destruction), you can get a reference to the instance by calling the function
(checks notes) `instance`:

```cpp
auto& instance = Singleton<MyClass>::instance();
```

## Polymorphic singleton

The `Singleton` class allows for polymorphic singletons also. In order to create a instance of a
derived class that will be retrieved by the base class, you call `create_instance` parameterized by
the derived class:

```cpp
Singleton<MyBase>::create_instance<MyDerived>("one", 2, THREE);
```

In this example, `"one", 2, THREE` are the paramters that will be passed to the constructor of
`MyDerived`.

After creating like shown above, the singleton are retrieved the same way as before:

```cpp
auto& instance = Singleton<MyBase>::instance();
```

Note that `Singleton<MyDerived>` is a totally different singleton. One big advantage of the
polymorphic behavior is that the code that gets the singleton, don´t have to depend on `MyDerived`.

See [tests](/test/brasa/patterns/singleton/SingletonTest.cpp) for some code that creates and uses
singletons.
