include ../Makefile.applets

applet_LTLIBRARIES = libclock.la
libclock_la_SOURCES = clock.c
libclock_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libclock.la

-include $(top_srcdir)/git.mk
