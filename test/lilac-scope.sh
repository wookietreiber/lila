#!/bin/bash

source test-compilation.sh

cat << EOF | test_compilation "42.000000"
val a = 21

val b = {
  val a = 1
  a * 3
}

def foo(a,b) = {
  def bar(a,b) = {
    def baz(a,b) = {
      a * b
    }

    baz(a, b + 2)
  }

  bar(a, b + 2)
}

foo(3, 2) + a + b
EOF
