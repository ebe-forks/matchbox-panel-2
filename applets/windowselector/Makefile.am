include ../Makefile.applets

applet_LTLIBRARIES = libwindowselector.la
libwindowselector_la_SOURCES = windowselector.c
libwindowselector_la_CPPFLAGS = $(AM_CPPFLAGS) $(SN_CFLAGS)
libwindowselector_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libwindowselector.la

-include $(top_srcdir)/git.mk
