SUBDIRS = brightness clock exit launcher notify systray showdesktop windowselector

if HAVE_BATTERY
  SUBDIRS += battery
endif

if HAVE_LIBSN
  SUBDIRS += startup
if HAVE_DBUS
  SUBDIRS += startup-notify
endif
endif

-include $(top_srcdir)/git.mk
