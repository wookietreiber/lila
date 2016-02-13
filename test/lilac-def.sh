#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation "42.000000"
def foo(a, b) = a + b
foo(10, 10) + foo(11, 11)
EOF
