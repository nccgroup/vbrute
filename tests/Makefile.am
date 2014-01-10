TESTS = check_main
check_PROGRAMS = check_main
TESTS_ENVIRONMENT = \
CK_DEFAULT_TIMEOUT=600

check_main_SOURCES = check_main.c  $(top_srcdir)/src/main.c

check_main_CFLAGS = @CHECK_CFLAGS@
check_main_CPPFLAGS = -DUNITTEST_MAIN
check_main_LDADD = @CHECK_LIBS@
