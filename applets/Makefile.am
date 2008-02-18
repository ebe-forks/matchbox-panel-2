SUBDIRS = brightness clock launcher notify systray showdesktop windowselector

if HAVE_LIBAPM
  SUBDIRS += battery
endif

if HAVE_LIBSN
  SUBDIRS += startup
if HAVE_DBUS
    SUBDIRS += startup-notify
endif
endif


MAINTAINERCLEANFILES = Makefile.in
