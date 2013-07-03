SUBDIRS = data

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              -DDATADIR=\"$(pkgdatadir)/brightness/\" \
              $(MATCHBOX_PANEL_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libbrightness.la

libbrightness_la_SOURCES = brightness.c
libbrightness_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in

include ../Makefile.applets

test_linkage_LDADD += libbrightness.la
