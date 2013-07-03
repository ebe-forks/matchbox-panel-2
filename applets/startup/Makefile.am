include ../Makefile.applets

SUBDIRS = data

applet_LTLIBRARIES = libstartup.la
libstartup_la_SOURCES = startup.c
libstartup_la_CPPFLAGS = $(AM_CPPFLAGS) $(SN_CFLAGS) -DDATADIR=\"$(pkgdatadir)/startup/\"
libstartup_la_LIBADD = $(SN_LIBS)
libstartup_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += libstartup.la

-include $(top_srcdir)/git.mk
