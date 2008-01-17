AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	-I$(top_srcdir) -I$(top_builddir) -Werror \
	$(SN_CFLAGS) $(LIBNOTIFY_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libstartup-notify.la

libstartup_notify_la_SOURCES = startup.c
libstartup_notify_la_LIBADD = $(SN_LIBS) $(LIBNOTIFY_LIBS)
libstartup_notify_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in
