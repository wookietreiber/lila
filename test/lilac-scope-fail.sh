#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation_fail
val a = {
  val b = 5 + 4
  b + 2
}
a + b + 2
EOF
