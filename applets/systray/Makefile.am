include ../Makefile.applets

%.c: %.list
	$(AM_V_GEN) (echo "#include \"na-marshal.h\""; \
	  $(GLIB_GENMARSHAL) --internal --prefix=_na_marshal $^ --body) > $@
%.h: %.list
	$(AM_V_GEN) $(GLIB_GENMARSHAL) --prefix=_na_marshal $^ --header > $@

applet_LTLIBRARIES = libsystray.la
libsystray_la_SOURCES = $(BUILT_SOURCES) \
                        na-tray-manager.c \
			na-tray-manager.h \
			systray.c
libsystray_la_LDFLAGS = -avoid-version -module

EXTRA_DIST = na-marshal.list

BUILT_SOURCES = na-marshal.c na-marshal.h
CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES = $(BUILT_SOURCES)
MAINTAINERCLEANFILES += $(BUILT_SOURCES)

test_linkage_LDADD += libsystray.la
