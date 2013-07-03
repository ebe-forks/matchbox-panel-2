AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              $(MATCHBOX_PANEL_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libexit.la

libexit_la_SOURCES = exit.c
libexit_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in

include ../Makefile.applets

test_linkage_LDADD += libexit.la
