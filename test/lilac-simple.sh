#!/bin/bash

LILAC=../src/compiler/lilac

OBJECT=$(mktemp)
LINKED=$(mktemp)

trap "rm -f $OBJECT $LINKED" EXIT

$LILAC -v -o $OBJECT <<< '20 + 22'
# TODO remove explicit clang usage
clang -o $LINKED $OBJECT

RESULT=$($LINKED)

if [[ $RESULT == "42.000000" ]] ; then
  echo "result matches expected value"
  exit 0
else
  echo "result did not match expected value ($RESULT != 42)"
  exit 1
fi
