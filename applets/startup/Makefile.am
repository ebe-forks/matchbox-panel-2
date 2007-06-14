SUBDIRS = data

AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
            -DGETTEXT_PACKAGE=\"matchbox-panel\" \
	    -DDATADIR=\"$(pkgdatadir)/startup/\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror \
		$(SN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libstartup.la

libstartup_la_SOURCES = startup.c
libstartup_la_LIBADD = $(SN_LIBS)
libstartup_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in
