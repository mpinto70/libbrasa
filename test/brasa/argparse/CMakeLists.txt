set(argparse_srcs
    ArgParserTest.cpp
    ParsersTest.cpp
    ValuesTest.cpp
)

set(argparse_libs
    argparse
)

add_unit_test(
    argparse
    argparse_srcs
    argparse_libs
)
