#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation "42"
def foo = 21
foo + foo
EOF
