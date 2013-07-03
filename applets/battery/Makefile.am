include ../Makefile.applets

applet_LTLIBRARIES = libbattery.la

libbattery_la_SOURCES = battery.c battery.h
libbattery_la_CPPFLAGS = $(AM_CPPFLAGS) -DDATADIR=\"$(pkgdatadir)/battery/\"

if HAVE_LIBAPM
libbattery_la_LIBADD = -lapm
libbattery_la_SOURCES += battery-apm.c
endif

if HAVE_LIBACPI
libbattery_la_LIBADD = -lacpi
libbattery_la_SOURCES += battery-acpi.c
endif

libbattery_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libbattery.la

-include $(top_srcdir)/git.mk
