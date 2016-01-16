#!/bin/bash

if which valgrind &> /dev/null ; then
  LILAC=../lilac/lilac

  MEMCHECK_OUT=$(mktemp)

  trap "rm -f $MEMCHECK_OUT" EXIT

  # TODO add --show-leak-kinds=all when travis gets newer valgrind
  valgrind --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 \
    $LILAC -o /dev/null <<< '42' \
      2> $MEMCHECK_OUT

  VALGRIND_EXIT_STATUS=$?

  cat $MEMCHECK_OUT

  exit $VALGRIND_EXIT_STATUS
else
  echo "no valgrind, skipping test"
  exit 77
fi
