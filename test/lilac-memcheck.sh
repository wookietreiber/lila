#!/bin/bash

if ! which valgrind &> /dev/null ; then
  echo "no valgrind, skipping test"
  exit 77
fi

LILAC=../lilac/lilac

MEMCHECK_OUT=$(mktemp)

trap "rm -f $MEMCHECK_OUT" EXIT

valgrind --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 \
  $LILAC -o /dev/null <<< '20 + 22' \
    2> $MEMCHECK_OUT

VALGRIND_EXIT_STATUS=$?

cat $MEMCHECK_OUT

exit $VALGRIND_EXIT_STATUS
