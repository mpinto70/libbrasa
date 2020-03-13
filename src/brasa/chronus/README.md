# Chronus package
This is the package of timing and waiting facilities.

## `Chronometer` component

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
* `reset`: sets the begin of the timing to the current instant (now).

## `Now` component

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

## `SleepStd` component

`SleepStd` component has a set of functions to stop the current program for a
specified amount of time. They are a wrapper around
`std::this_thread::sleep_for`:

* `nano_sleep`: suspends the current thread for `sleep_length` nanoseconds.
* `micro_sleep`: suspends the current thread for `sleep_length` microseconds.
* `milli_sleep`: suspends the current thread for `sleep_length` milliseconds.

## `Waiter` component
The `Waiter` class is parameterized by the function or functor that returns the
current instant in time (`NOW_FUNC`) and the sleep function (`SLEEP_FUNC`) that
will sleep for one tenth of the wait time. The `Waiter` component is responsible
to guarantee that a certain amount of time has elapsed before continuing. 

The helper function `make_waiter` creates a `Waiter` without the need to specify
the types of now and sleep functions (because it is deduced from the arguments).
For example, suppose you want to guarantee that a certain processing would not
take less than 15ms. You could create a `waiter` that will wait for 10ms and
then run the computation, after the computation returned, you would just
`waiter.wait()` and the total time would be a little more than the 15ms:

```cpp
auto waiter = make_waiter(brasa::chronus::milli_now, 15, brasa::chronus::milli_sleep);
// run the fast process
waiter.wait(); // assure that current thread will wait until 15ms have passed
```

`Waiter`'s member functions are:

* `elapsed`: returns a boolean indicating if the wait time has passed
* `wait`: will repeatedly suspend current thread until wait time has passed
* `reset`: to restart counting wait time
