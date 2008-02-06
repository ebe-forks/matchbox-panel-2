AM_CPPFLAGS=-DPKGDATADIR=\"$(pkgdatadir)\" -DGETTEXT_PACKAGE=\"matchbox-panel\"
AM_CFLAGS = -Wall -g $(MATCHBOX_PANEL_CFLAGS) $(DBUS_CFLAGS) \
	    -I$(top_srcdir) -I$(top_builddir) -Werror

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libnotify.la

libnotify_la_SOURCES = applet.c \
	notify-store.c notify-store.h \
	mb-notification.c mb-notification.h \
	$(DBUS_GLUE) \
	$(MARSHALS)

libnotify_la_LDFLAGS = -avoid-version -Wl,--no-undefined
libnotify_la_LIBADD = $(MATCHBOX_PANEL_LIBS) $(DBUS_LIBS)

DBUS_GLUE=notification-manager-glue.h
%-glue.h: %.xml
	dbus-binding-tool --mode=glib-server --output=$@ --prefix=$(subst -,_,$*) $^

MARSHALS = marshal.c marshal.h
%.c: %.list
	$(GLIB_GENMARSHAL) --prefix=mb_marshal $^ --body > $@
%.h: %.list
	$(GLIB_GENMARSHAL) --prefix=mb_marshal $^ --header > $@

BUILT_SOURCES = $(DBUS_GLUE) $(MARSHALS)
CLEANFILES = $(BUILT_SOURCES)
MAINTAINERCLEANFILES = Makefile.in
