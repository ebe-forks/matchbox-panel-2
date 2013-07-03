AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              $(MATCHBOX_PANEL_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libshowdesktop.la

libshowdesktop_la_SOURCES = showdesktop.c
libshowdesktop_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in

include ../Makefile.applets

test_linkage_LDADD += libshowdesktop.la
