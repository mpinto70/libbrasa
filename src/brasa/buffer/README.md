# Buffer package

- [Buffer package](#buffer-package)
  - [Technical details](#technical-details)
    - [`CircularWriter` component](#circularwriter-component)
    - [`CircularReader` component](#circularreader-component)
    - [`Circular` helper component (inside `impl` namespace)](#circular-helper-component-inside-impl-namespace)

This is the package of buffering facilities. The driving idea behind this
package is to allow communication between processes to allow monitoring. The
main idea is that a shared memory buffer is allocated by monitored process and
passed to a `CircularWriter`, then the monitoring process gets the address of
the shared memory and pass it to a `CircularReader`. This is a typical
producer/consumer environment, where the monitored process is the producer and
the monitoring process is the consumer. In order to not impact the producer, the
access is not locked, and the writing is always successful, but the reader may
read dirty information. With this philosophy, it is necessary that the consumer
be faster than the producer, to avoid reading too many dirty data.

Check [the demo](../../../demos/buffer/buffer.cpp) for a sample usage.

## Technical details

There are two main components in `buffer` package: `CircularReader` and
`CircularWriter`; and two helper components: `Circular` and `CRC`.

### `CircularWriter` component

`CircularWriter` is the component that writes to the buffer (**producer**). It
exposes the following member functions:

- constructor: takes the buffer and a key number that is used to uniquely
  identify the buffer.
- `write`: that stores a `value` into `data`. It **always succeeds**.

The buffer must be at least `CircularWriter::BUFFER_SIZE` bytes long.

### `CircularReader` component

`CircularReader` is the component that reads from the buffer (**consumer**). It
exposes the following member functions:

- constructor: takes the buffer and a key number that is used to uniquely
  identify the buffer.
- `read`: that reads a `value` from `data`. If there are no value to be read,
  returns `false` and leaves `value` unchanged.

The buffer must be at least `CircularReader::BUFFER_SIZE` bytes long.

### `Circular` helper component (inside `impl` namespace)

`Circular` helper component is parameterized by the type of values (`TYPE_`)
that it will store and the number of elements (`N_`) the buffer is capable to
hold. This component was designed **to be specialized and not used directly**,
thus almost all its contents is `protected`. It contains logic to write and read
data from the buffer. That is used by its descendants.

In its `public` interface, `Circular` exposes:

- `TYPE`: the type stored
- `N`: the number of elements it is able to store (`data`)
- `BUFFER_SIZE`: the minimum size of the buffer passed to it during construction

In its `protected` interface, `Circular` exposes:

- constructor: taking the buffer and a key number to use in validations. The key
  number should be a unique number to avoid errors in reading/writing.
- `do_write`: writes a value to `data` (always succeeds).
- `do_read`: reads a value from `data` into `value` and returns `true`. If there
  is no value available in `data`, returns `false` and does not change `value`.

The buffer passed to Circular has to have at least `Circular::BUFFER_SIZE` bytes
in it.
