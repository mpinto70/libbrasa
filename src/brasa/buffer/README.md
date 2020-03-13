# Buffer package
This is the package of buffering facilities. The driving idea behind this
package is to allow communication between processes for monitoring. The main
idea is that a memory buffer is allocated by client and passed to two circular
buffer to be managed in a producer/consumer environment. `CircularWriter` is the
producer and `CircularReader` is the consumer. In order to not impact the
producer, the access is not locked, and the writing is always successful, but
the reader may read dirty information. With this philosophy, it is necessary
that the consumer be faster than the producer.

There are two main components in `buffer` package: `CircularReader` and
`CircularWriter`; and two helper components: `Circular` and `CRC`.

## `CircularWriter` component

`CircularWriter` is the component that writes to the buffer (producer). It
exposes the following member functions:

* constructor: takes the buffer and a key number that is used to uniquely
  identify the buffer.
* `write`: that writes `data` to buffer. It **always succeed**.

The buffer must be at least `CircularWriter::BUFFER_SIZE` bytes long.

## `CircularReader` component

`CircularReader` is the component that reads from the buffer (producer). It
exposes the following member functions:

* constructor: takes the buffer and a key number that is used to uniquely
  identify the buffer.
* `read`: that reads `data` from buffer. If there is no data to be read, returns
  `false` and leaves `data` unchanged.

The buffer must be at least `CircularReader::BUFFER_SIZE` bytes long.

## `Circular` helper component

`Circular` helper component is parameterized by the type of values (`TYPE_`)
that it will store in the buffer and the number of elements (`N_`) the buffer is
able to hold. This component was designed **to be specialized and not used
directly**, thus almost all its contents is `protected`. It contains logic to
write and read data from the buffer. That is used by its descendants.

In its `public` interface, `Circular` exposes:

* `TYPE`: the type stored
* `N`: the number of elements it is able to store
* `BUFFER_SIZE`: the minimum size of the buffer passed to it during construction

In its `protected` interface, `Circular` exposes:

* constructor: taking the buffer and a key number to use in validations. The key
  number should be a unique number to avoid errors in reading/writing.
* `do_write`: writes data to buffer (always succeed).
* `do_read`: reads data from buffer into `data` and returns `true`. If there is
  no data to be read, returns `false` and does not change `data`.


The buffer passed to Circular has to have at least `Circular::BUFFER_SIZE` bytes
in it.
