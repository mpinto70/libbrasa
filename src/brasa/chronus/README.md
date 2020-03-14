# Chronus package

This is the package of timing and waiting facilities. 

## Chronometer utilities

You can create chronometers with specific resolutions. Chronometer resolution is
determined by the function (or functor) passed during construction:

* to create a chronometer with nanosecond resolution (`brasa::chronus::nano_now`
  has nanosecond resolution):
```cpp
    const auto nano_chron = brasa::chronus::make_chronometer(brasa::chronus::nano_now, 1); // 1 is the chronometer identifier
```
* to create a chronometer with millisecond resolution (`brasa::chronus::milli_now`
  has millisecond resolution):
```cpp
    const auto nano_chron = brasa::chronus::make_chronometer(brasa::chronus::milli_now, 2); // 2 is the chronometer identifier
```

In order to get the elapsed time with full information, you can use the `mark` function:

```cpp
const auto chron = make_chronometer(nano_now, 1234); // chronometer id is 1234
// make some computation
const auto t1 = chron.mark(4321); // mark id is 4321
// t1 is a Elapsed struct object with:
// .chrono_id == 1234
// .mark_id == 4321
// .begin == nano_now() at construction/reset time
// .end == nano_now() at mark time
```

If you only want the tick count, you should use the `count` function:

```cpp
const auto chron = make_chronometer(nano_now, 1234); // chronometer id is 1234
// make some computation
const auto t1 = chron.count();
// t1 will hold the number of nanoseconds (because nano_now was used) since construction/reset
```

## Waiting utilities

Sometimes you need some computations to hold off for a while to avoid
overloading the systems, or because its results will be timed. In those cases,
you should use `Waiter` class. To create a `Waiter` object, you have to inform
the function that represents counts units of time (now function), the amount of
units of time you want to wait, and the sleep function that will be used to
avoid a busy wait. The size of the sleep is 1/10 of the size of the full wait
(not allowed to be less than 1 unit), thus you shouldn't use wait times less
than 20. If you need wait times that is less than 20, change timing resolution.
For instance, if you have a process that should emit its results every 15
milliseconds, instead of creating a waiter of 15 milliseconds, create a waiter
of 15'000 microseconds.

To create a `Waiter` object to wait for 150 milliseconds:
```cpp
auto waiter = make_waiter(brasa::chronus::milli_now, 150, brasa::chronus::milli_sleep);
// run the fast process
waiter.wait(); // assure that current thread will wait until 150ms have passed
```

## Technical aspects
### `Chronometer` component

There are two main classes (`Elapsed` and `Chronometer`) and one helper function
(`make_chronometer`) in this component.

The `Elapsed` class serves to register elapsed times from a chronometer. The
information registered are: 

* `chrono_id`: chronometer identifier passed to `Chronometer` construction.
* `mark_id`: identifier for a specific instant in time when `Chronometer::mark`
  is called.
* `begin`: the instant in which the `Chronometer` object was created or
  `Chronometer::reset` was called.
* `end`: the instant in which `Chronometer::mark` was called and the object
  created.

The `Chronometer` class is parameterized by the function or functor (`NOW_FUNC`)
and the elapsed structure (`ELAPSED`) that it returns from `mark` function, to
mark different instants in timing.

The `NOW_FUNC` object returns the current timing with specific precision and
characteristics (see `Now.h` for examples). The type of the return should match
the type of the third and the fourth element in `ELAPSED` struct (see `Elapsed`).

The `ELAPSED` type is a struct mark the beginning and the end of the timing
identified by the chronometer id, the id passed to `Chronometer::mark` and
stores the start time of the timing (e.g. `Elapsed::begin`) and the instant when
`Chronometer::mark` was called (e.g `Elapsed::end`).

The `NOW_FUNC` object returns the current timing with specific precision and
characteristics (see `Now.h` for examples). The type of the return should match
the type of the third and the fourth element in `ELAPSED` struct (see `Elapsed`)
that mark the beginning and the end of the timing identified by the passed id.

The helper function `make_chronometer` creates a `Chronometer` without the need
to specify the type of now function (because it is deduced from the arguments),
and the type of `ELAPSED` is defaulted to `Elapsed`. For example you could
create a chronometer that returns `Elapsed` on `mark` with
`brasa::chronus::nano_now` as `NOW_FUNC` with command:

```cpp
const auto chronometer = brasa::chronus::make_chronometer(brasa::chronus::nano_now, 1); // 1 is the chronometer identifier
```

The returned `chronometer` from the above command marks time in nanoseconds.

`Chronometer` has the following (important) functions:

* `mark`: returns information for the current instant (now).
* `count`: returns the tick count since timing began.
* `reset`: sets the begin of the timing to the current instant (now).

### `Now` component

`Now` component has a set of functions to pass to `Chronometer`. These functions
use the facilities in STL chrono library:

* `nano_now`: returns the current instant in nanoseconds.
* `micro_now`: returns the current instant in microseconds.
* `milli_now`: returns the current instant in milliseconds.

One could implement a class based in `clock_gettime` function:

```cpp
#include <ctime>

struct NowClock {
    constexpr explicit NowClock(clockid_t id) noexcept
          : clock_id_(id) {}
    uint64_t operator()() const noexcept {
        timespec now;
        ::clock_gettime(clock_id_, &now);
        return now.tv_sec * brasa::chronus::NSECS_PER_SEC + now.tv_nsec;
    }

private:
    clockid_t clock_id_;
};
```

### `SleepStd` component

`SleepStd` component has a set of functions to stop the current program for a
specified amount of time. They are a wrapper around
`std::this_thread::sleep_for`:

* `nano_sleep`: suspends the current thread for `sleep_length` nanoseconds.
* `micro_sleep`: suspends the current thread for `sleep_length` microseconds.
* `milli_sleep`: suspends the current thread for `sleep_length` milliseconds.

### `Waiter` component

The `Waiter` class is parameterized by the function or functor that returns the
current instant in time (`NOW_FUNC`) and the sleep function (`SLEEP_FUNC`) that
will sleep for one tenth of the wait time. The `Waiter` component is responsible
to guarantee that a certain amount of time has elapsed before continuing. 

The helper function `make_waiter` creates a `Waiter` without the need to specify
the types of now and sleep functions (because it is deduced from the arguments).
For example, suppose you want to guarantee that a certain processing would not
take less than 150ms. You could create a `waiter` that will wait for 150ms (or
reset the waiter) just before the running the computation, run the computation,
and, after the computation returned, you would just `waiter.wait()` and the
total time would be a little more than the 150ms:

```cpp
auto waiter = make_waiter(brasa::chronus::milli_now, 150, brasa::chronus::milli_sleep);
// run the fast process
waiter.wait(); // assure that current thread will wait until 150ms have passed
```

`Waiter`'s member functions are:

* `elapsed`: returns a boolean indicating if the wait time has passed
* `wait`: will repeatedly suspend current thread until wait time has passed
* `reset`: to restart counting wait time
