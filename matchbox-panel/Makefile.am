AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
            -DGETTEXT_PACKAGE=\"matchbox-panel\" \
	    -DDEFAULT_APPLET_PATH=\"$(pkglibdir)\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror

matchbox_panelincdir = $(pkgincludedir)

matchbox_panelinc_HEADERS = mb-panel.h mb-panel-scaling-image.h

bin_PROGRAMS = matchbox-panel

matchbox_panel_SOURCES = mb-panel.c mb-panel-scaling-image.c

matchbox_panel_LDADD = $(MATCHBOX_PANEL_LIBS)
# This is required as gmodule-export in Poky doesn't have it
matchbox_panel_LDFLAGS = -Wl,--export-dynamic

CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(BUILT_SOURCES)
MAINTAINERCLEANFILES = Makefile.in $(BUILT_SOURCES)
