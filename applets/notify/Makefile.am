include ../Makefile.applets

applet_LTLIBRARIES = libnotify.la

libnotify_la_SOURCES = applet.c \
	notify-store.c notify-store.h \
	mb-notification.c mb-notification.h \
	$(DBUS_GLUE) \
	$(MARSHALS)

libnotify_la_CPPFLAGS = $(AM_CPPFLAGS) $(DBUS_CFLAGS)
libnotify_la_LDFLAGS = -avoid-version -module
libnotify_la_LIBADD = $(MATCHBOX_PANEL_LIBS) $(DBUS_LIBS)

DBUS_GLUE=notification-manager-glue.h
%-glue.h: %.xml
	$(AM_V_GEN) dbus-binding-tool --mode=glib-server --output=$@ --prefix=$(subst -,_,$*) $^

MARSHALS = marshal.c marshal.h
%.c: %.list
	$(AM_V_GEN) (echo "#include \"marshal.h\""; \
	  $(GLIB_GENMARSHAL) --internal --prefix=mb_marshal $^ --body) > $@
%.h: %.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --internal --prefix=mb_marshal $^ --header > $@

BUILT_SOURCES = $(DBUS_GLUE) $(MARSHALS)

test_linkage_LDADD += libnotify.la

-include $(top_srcdir)/git.mk
