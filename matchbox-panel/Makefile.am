AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" \
            -DGETTEXT_PACKAGE=\"matchbox-panel\" \
	    -DDEFAULT_APPLET_PATH=\"$(pkglibdir)\" \
            $(MATCHBOX_PANEL_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

matchbox_panelincdir = $(pkgincludedir)

matchbox_panelinc_HEADERS = mb-panel.h mb-panel-scaling-image.h mb-panel-scaling-image2.h

bin_PROGRAMS = matchbox-panel

matchbox_panel_SOURCES = mb-panel.c mb-panel-scaling-image.c mb-panel-scaling-image2.c

matchbox_panel_LDADD = $(MATCHBOX_PANEL_LIBS)

-include $(top_srcdir)/git.mk
