include ../Makefile.applets

SUBDIRS = data

applet_LTLIBRARIES = libbrightness.la
libbrightness_la_SOURCES = brightness.c
libbrightness_la_CPPFLAGS = $(AM_CPPFLAGS) -DDATADIR=\"$(pkgdatadir)/brightness/\"
libbrightness_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libbrightness.la
