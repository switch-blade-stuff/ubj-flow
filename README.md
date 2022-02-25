## About

`ubj-flow` is a library meant to be used as backend for serialization.

The `flow` part of the name comes from the stream-like nature of the library, extracted data "flows" into the
user-provided interface.

The main goal of this library is to be a pure UBJSON parser and leave final data representation up to the user. This is
especially useful, when one would want to build a data serialization system on top of this parser, as that way there
will be no need to copy the extracted data.

Let's say you want to implement UBJSON serialization for your favorite scripting language. You will have two choices:

* Using an off-the-shelf UBJSON library to deserialize the data, then copying that data into your target environment.
* Creating a new UBJSON parser implementation from scratch.

The first approach will work, but will be quite wasteful since you will need to copy all the data, especially if there
is a lot of stuff to deserialize.

And while UBJSON is not a complicated format a parser for which can be implemented in a couple of days, it is another
chunk of code for you to maintain and worry about. Besides, you just want to use a god-damn UBJSON parser, not write it.

This library attempts to solve this issue by not bothering to store any data it reads at all. Instead, the user is
provided with a set of events that will be invoked when the library needs to read or write input, allocate buffer for
string serialization and when value or container nodes are parsed.

### DISCLAMER

This library does not claim to be and will never be a complete standalone UBJSON serialization implementation. It is
only meant to be

### UBJSON compatibility

This library is compatible with the current (as of February 2022) version of UBJson (spec 12).

