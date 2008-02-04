SUBDIRS = data

AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
            -DGETTEXT_PACKAGE=\"matchbox-panel\" \
	    -DDATADIR=\"$(pkgdatadir)/brightness/\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libbrightness.la

libbrightness_la_SOURCES = brightness.c
libbrightness_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in
