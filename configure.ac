# Prelude.
AC_PREREQ([2.69])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/vbrute.c])
AC_INIT([project-main], [0.1], [aidan.marlin+github@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
#PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
PKG_CHECK_MODULES([LIBEVENT], [libevent >= 2.0])
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
# Checks for libraries.
#AC_CHECK_LIB([efence],[malloc],,AC_MSG_ERROR([Could not find malloc in efence lib]))
# Checks for header files.
# Checks for typedefs, structures, and compiler characteristics.
# Checks for library functions.
# Output files.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
