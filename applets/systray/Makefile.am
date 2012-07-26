AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              $(MATCHBOX_PANEL_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libsystray.la

%.c: %.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_na_marshal $^ --body > $@
%.h: %.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_na_marshal $^ --header > $@

BUILT_SOURCES = na-marshal.c na-marshal.h

libsystray_la_SOURCES = $(BUILT_SOURCES) \
                        na-tray-manager.c \
			na-tray-manager.h \
			systray.c
libsystray_la_LDFLAGS = -avoid-version

EXTRA_DIST = na-marshal.list

CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(BUILT_SOURCES)
MAINTAINERCLEANFILES = Makefile.in $(BUILT_SOURCES)

