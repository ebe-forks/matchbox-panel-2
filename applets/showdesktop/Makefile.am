AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libshowdesktop.la

libshowdesktop_la_SOURCES = showdesktop.c
libshowdesktop_la_LDFLAGS = -avoid-version

icondir = $(datadir)/pixmaps
icon_DATA = mb-applet-showdesktop.png mb-applet-showdesktop-active.png

MAINTAINERCLEANFILES = Makefile.in
