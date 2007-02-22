AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libwindowselector.la
libwindowselector_la_SOURCES = windowselector.c
libwindowselector_la_LDFLAGS = -avoid-version

icondir = $(datadir)/pixmaps
icon_DATA = mb-applet-windowselector.png

MAINTAINERCLEANFILES = Makefile.in
