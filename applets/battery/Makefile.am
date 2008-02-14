AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
            -DGETTEXT_PACKAGE=\"matchbox-panel\" \
	    -DDATADIR=\"$(pkgdatadir)/battery/\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libbattery.la

libbattery_la_SOURCES = battery.c
libbattery_la_LIBADD = -lapm
libbattery_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in
