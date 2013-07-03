AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
              -DGETTEXT_PACKAGE=\"matchbox-panel\" \
              $(MATCHBOX_PANEL_CFLAGS) \
              $(DBUS_CFLAGS) $(SN_CFLAGS) \
              -I$(top_srcdir) -I$(top_builddir)
AM_CFLAGS = $(WARN_CFLAGS)

appletdir = $(pkglibdir)
applet_LTLIBRARIES = libstartup-notify.la

libstartup_notify_la_SOURCES = startup.c $(MARSHALS)
libstartup_notify_la_LIBADD = $(SN_LIBS) $(DBUS_LIBS)
libstartup_notify_la_LDFLAGS = -avoid-version

MARSHALS = marshal.c marshal.h
%.c: %.list
	$(AM_V_GEN) (echo "#include \"marshal.h\""; \
	  $(GLIB_GENMARSHAL) --internal --prefix=mb_marshal $^ --body) > $@
%.h: %.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=mb_marshal $^ --header > $@

BUILT_SOURCES = $(MARSHALS)
CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

include ../Makefile.applets

test_linkage_LDADD += libstartup-notify.la
