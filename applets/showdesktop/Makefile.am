include ../Makefile.applets

applet_LTLIBRARIES = libshowdesktop.la
libshowdesktop_la_SOURCES = showdesktop.c
libshowdesktop_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libshowdesktop.la

-include $(top_srcdir)/git.mk
