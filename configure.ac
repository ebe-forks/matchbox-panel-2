AC_PREREQ(2.53)
AC_INIT(matchbox-panel, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(matchbox-panel/mb-panel.c)
AM_CONFIG_HEADER(config.h)

AM_DISABLE_STATIC
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

# base deps
PKG_CHECK_MODULES(MATCHBOX_PANEL,
                  glib-2.0
                  gtk+-2.0 >= 2.6
                  gmodule-export-2.0)

# startup-notification
AC_ARG_ENABLE(startup_notification,
     AC_HELP_STRING([--disable-startup-notification], [disable startup notification support]),
     enable_startup_notification=$enableval, enable_startup_notification=yes )

if test x$enable_startup_notification != xno; then
  PKG_CHECK_MODULES(SN, libstartup-notification-1.0, ,
    AC_MSG_ERROR([*** Required Startup Notification library not installed ***]))

  AC_DEFINE(USE_LIBSN, [1], [Has Startup Notification Support])
fi
AM_CONDITIONAL(HAVE_LIBSN, test x$enable_startup_notification = xyes)


# libnotify
AC_ARG_ENABLE(libnotify,
     AC_HELP_STRING([--disable-libnotify], [disable libnotify support]),
     enable_libnotify=$enableval, enable_libnotify=yes )

if test x$enable_libnotify != xno; then
  PKG_CHECK_MODULES(LIBNOTIFY, libnotify, ,
    AC_MSG_ERROR([*** Required libnotify library not installed ***]))

  AC_DEFINE(USE_LIBNOTIFY, [1], [Has libnotify Support])
fi
AM_CONDITIONAL(HAVE_LIBNOTIFY, test x$enable_libnotify = xyes)


# DBus
AC_ARG_ENABLE(dbus,
     AC_HELP_STRING([--disable-dbus], [disable DBus support]),
     enable_dbus=$enableval, enable_dbus=yes )

if test x$enable_dbus != xno; then
  PKG_CHECK_MODULES(DBUS, dbus-glib-1, ,
    AC_MSG_ERROR([*** Required DBus library not installed ***]))

  AC_DEFINE(USE_DBUS, [1], [Has DBus Support])
fi
AM_CONDITIONAL(HAVE_DBUS, test x$enable_dbus = xyes)

# apm
AC_CHECK_HEADERS(apm.h, enable_linux_apm=yes, enable_linux_apm=no)
AM_CONDITIONAL(HAVE_LIBAPM, test x$enable_linux_apm = xyes)


# glib-genmarshal
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`  
AC_SUBST(GLIB_GENMARSHAL)

# gettext
GETTEXT_PACKAGE=matchbox-panel
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], 
  [Define the gettext package to be used])
  
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

# output stuff
AC_OUTPUT([
Makefile
matchbox-panel.pc
matchbox-panel/Makefile
applets/Makefile
applets/clock/Makefile
applets/launcher/Makefile
applets/notify/Makefile
applets/startup/Makefile
applets/startup/data/Makefile
applets/startup-notify/Makefile
applets/systray/Makefile
applets/showdesktop/Makefile
applets/windowselector/Makefile
applets/battery/Makefile
applets/battery/data/Makefile
applets/brightness/Makefile
applets/brightness/data/Makefile
po/Makefile.in
po/Makefile
])
