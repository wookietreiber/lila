#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lila], [0.1.0], [https://github.com/wookietreiber/lila/issues])
AC_CONFIG_SRCDIR([src/bootstrap/lilac.cpp])
AC_CONFIG_HEADERS([config.h])

# Automake invocation.
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_STRTOD

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/bootstrap/Makefile
                 test/Makefile
                 test/test-compilation.sh])
AC_OUTPUT
