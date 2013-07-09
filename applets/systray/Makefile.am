include ../Makefile.applets

applet_LTLIBRARIES = libsystray.la
libsystray_la_SOURCES = systray.c \
                        na-tray.c \
			na-tray.h \
                        na-tray-manager.c \
			na-tray-manager.h \
                        na-tray-child.c \
			na-tray-child.h \
			fixedtip.c \
			fixedtip.h

libsystray_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libsystray.la

-include $(top_srcdir)/git.mk
