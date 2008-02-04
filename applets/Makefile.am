SUBDIRS = brightness clock launcher systray showdesktop windowselector

if HAVE_LIBAPM
  SUBDIRS += battery
endif

if HAVE_LIBSN
  SUBDIRS += startup
if HAVE_LIBNOTIFY
    SUBDIRS += startup-notify
endif
endif


MAINTAINERCLEANFILES = Makefile.in
