#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation "42.000000"
val a = 21

val b = {
  val a = 2
  a * 2
}

a * 2
EOF
