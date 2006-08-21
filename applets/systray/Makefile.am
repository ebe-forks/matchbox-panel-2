AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir)
	    
appletdir = $(libdir)/matchbox-panel
applet_LTLIBRARIES = libsystray.la

eggmarshalers.h: eggmarshalers.list
	cd $(srcdir) \
	&& $(GLIB_GENMARSHAL) --prefix=_egg_marshal eggmarshalers.list --header > xgen-emh \
	&& cp xgen-emh eggmarshalers.h \
	&& rm -f xgen-emh xgen-emh~

eggmarshalers.c: eggmarshalers.list
	cd $(srcdir) \
	&& $(GLIB_GENMARSHAL) --prefix=_egg_marshal eggmarshalers.list --body > xgen-emc \
	&& cp xgen-emc eggmarshalers.c \
	&& rm -f xgen-emc xgen-emc~

BUILT_SOURCES = eggmarshalers.c eggmarshalers.h

libsystray_la_SOURCES = $(BUILT_SOURCES) \
                        eggtraymanager.c \
			eggtraymanager.h \
			systray.c

EXTRA_DIST = eggmarshalers.list

CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(BUILT_SOURCES)
MAINTAINERCLEANFILES = Makefile.in $(BUILT_SOURCES)
