# Argument parser

A simple command line parameter parser that uses `getopt_long` to process
arguments inspired by Python's
[`argparse`|https://docs.python.org/3.5/howto/argparse.html].

The interface value digester from the command line is:

* `void digest(const std::string& argument)` to digest the argument and store
  the relevant value.
* `<type> value() const` to return the value parsed (and possibly converted).
* `bool can_digest() const` that return if it is still possible to digest an
  argument. This function can also be static instead of const.
* `std::string name() const` to return the name of the parameter to be used in
  documenting usage and reporting errors.
* `std::string description() const` to return the description of the parameter
  to be used in documenting usage.

Flagged options (those preceded by `-<character>` or `--<word>`) parser is a
`BooleanParser` or has interface:

* `void digest(const std::string& argument)` to digest the argument and store
  the relevant value.
* `<type> value() const` to return the value parsed (and possibly converted).
* `bool is_present() const` that return if the argument was present in command
  line.
* `std::string name() const` to return the name of the parameter to be used in
  documenting usage.
* `std::string description() const` to return the description of the parameter
  to be used in documenting usage.
* `char short_option() const` to return the short option (it has to be unique
  and cannot be `h`).
* `const std::string& long_option() const` to return the long option (it has to
  be unique and cannot be `help` and also has to be persistent so it can be
  pointed to by the command line parser).
* `constexpr static bool IS_BOOLEAN = false;`

## Example

You can check [the demo](../../../demos/argparse/argparse.cpp) for a sample usage.

The following code:
```cpp
using brasa::argparse::make_parser;
using brasa::argparse::SingleValue;
using brasa::argparse::MultiValue;
using brasa::argparse::BooleanParser;
using brasa::argparse::ValueParser;
using brasa::argparse::ParseResult;

auto parser = make_parser("Program to find minimum number of occurrences of a word in files",
      std::make_tuple(
            SingleValue<int>("MIN_OCCURRENCES", "minimum number of occurrences to look for"),
            SingleValue<std::string>("EXPRESSION", "expression to search for")),
      std::make_tuple(
            BooleanParser('l', "file-names-only", "only show file names instead of occurrences"),
            ValueParser<MultiValue<std::string>>('i', "ignore", "regex-to-ignore", "regex with file names to ignore")));
            ValueParser<SingleValue<std::string>>('o', "output", "output-file", "file to write the report. If not specified, output is directed to stdout")));

auto parse_result = parser.parse(argc, argv, std::cerr);
```
would be able to process the following command line:
```
./exe -l --ignore ".*\.h" -o output.file 127 word
```

## Limitations

Limitations of current implementation:

* there is no subparsers capability
* it is not possible to specify twice the same boolean value (like `-vv`)
