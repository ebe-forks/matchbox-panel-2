include ../Makefile.applets

applet_LTLIBRARIES = liblauncher.la

liblauncher_la_SOURCES = launcher.c
liblauncher_la_CPPFLAGS = $(AM_CPPFLAGS) $(SN_CFLAGS)
liblauncher_la_LIBADD = $(SN_LIBS)
liblauncher_la_LDFLAGS = -avoid-version -module

test_linkage_LDADD += liblauncher.la
