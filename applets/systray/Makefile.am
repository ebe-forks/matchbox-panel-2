AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              $(MATCHBOX_PANEL_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libsystray.la

na-marshal.h: na-marshal.list
	cd $(srcdir) \
	&& $(GLIB_GENMARSHAL) --prefix=_na_marshal na-marshal.list --header > xgen-emh \
	&& cp xgen-emh na-marshal.h \
	&& rm -f xgen-emh xgen-emh~

na-marshal.c: na-marshal.list
	cd $(srcdir) \
	&& $(GLIB_GENMARSHAL) --prefix=_na_marshal na-marshal.list --body > xgen-emc \
	&& cp xgen-emc na-marshal.c \
	&& rm -f xgen-emc xgen-emc~

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

