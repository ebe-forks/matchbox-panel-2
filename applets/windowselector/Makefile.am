AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror
	    
appletdir = $(pkglibdir)
applet_LTLIBRARIES = libwindowselector.la

libwindowselector_la_SOURCES = windowselector.c

MAINTAINERCLEANFILES = Makefile.in
