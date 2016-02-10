#!/bin/bash

if ! which valgrind &> /dev/null ; then
  echo "no valgrind, skipping test"
  exit 77
fi

LILAC=../src/bootstrap/lilac

MEMCHECK_OUT=$(mktemp)

trap "rm -f $MEMCHECK_OUT" EXIT

valgrind --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=1 \
$LILAC -v -o /dev/null 2> $MEMCHECK_OUT << EOF
def foo = 2
val a = 10 + 8 + foo
val b = (a + 1) * foo
b
EOF

VALGRIND_EXIT_STATUS=$?

cat $MEMCHECK_OUT

exit $VALGRIND_EXIT_STATUS
