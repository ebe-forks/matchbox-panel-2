AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror
	    
appletdir = $(libdir)/matchbox-panel
applet_LTLIBRARIES = libclock.la

libclock_la_SOURCES = clock.c

MAINTAINERCLEANFILES = Makefile.in
