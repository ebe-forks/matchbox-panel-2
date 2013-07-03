include ../Makefile.applets

applet_LTLIBRARIES = libstartup-notify.la

libstartup_notify_la_SOURCES = startup.c $(MARSHALS)
libstartup_notify_la_CPPFLAGS = $(AM_CPPFLAGS) $(DBUS_CFLAGS) $(SN_CFLAGS)
libstartup_notify_la_LIBADD = $(DBUS_LIBS) $(SN_LIBS)
libstartup_notify_la_LDFLAGS = -avoid-version -module

MARSHALS = marshal.c marshal.h
%.c: %.list
	$(AM_V_GEN) (echo "#include \"marshal.h\""; \
	  $(GLIB_GENMARSHAL) --internal --prefix=mb_marshal $^ --body) > $@
%.h: %.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=mb_marshal $^ --header > $@

BUILT_SOURCES = $(MARSHALS)

test_linkage_LDADD += libstartup-notify.la

-include $(top_srcdir)/git.mk
