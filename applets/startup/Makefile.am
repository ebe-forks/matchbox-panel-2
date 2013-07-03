SUBDIRS = data

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              -DDATADIR=\"$(pkgdatadir)/startup/\" \
              $(MATCHBOX_PANEL_CFLAGS) $(SN_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libstartup.la

libstartup_la_SOURCES = startup.c
libstartup_la_LIBADD = $(SN_LIBS)
libstartup_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in

include ../Makefile.applets

test_linkage_LDADD += libstartup.la
