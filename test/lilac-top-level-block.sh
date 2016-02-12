#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation "42.000000"
val a = 10 + 10
val b = (a + 1) * 2
b
EOF
