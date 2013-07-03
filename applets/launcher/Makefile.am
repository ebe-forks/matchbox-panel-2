AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              $(MATCHBOX_PANEL_CFLAGS) $(SN_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = liblauncher.la

liblauncher_la_SOURCES = launcher.c
liblauncher_la_LIBADD = $(SN_LIBS)
liblauncher_la_LDFLAGS = -avoid-version

MAINTAINERCLEANFILES = Makefile.in

include ../Makefile.applets

test_linkage_LDADD += liblauncher.la
