include ../Makefile.applets

applet_LTLIBRARIES = libexit.la
libexit_la_SOURCES = exit.c
libexit_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libexit.la
