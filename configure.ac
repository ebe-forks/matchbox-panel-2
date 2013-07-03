AC_PREREQ(2.53)
AC_INIT(matchbox-panel, 2.0, http://www.openedhand.com/)
AM_INIT_AUTOMAKE([-Wno-portability])
AC_CONFIG_SRCDIR(matchbox-panel/mb-panel.c)
AM_CONFIG_HEADER(config.h)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_DISABLE_STATIC
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

GNOME_COMPILE_WARNINGS

# base deps
PKG_CHECK_MODULES(MATCHBOX_PANEL,
                  glib-2.0
                  x11
                  gdk-x11-2.0
                  gtk+-2.0 >= 2.18
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

# Where to read battery state from
AC_ARG_WITH(
	[battery],
	AS_HELP_STRING([--with-battery], [where to read the battery state from]),
	[],
	[with_battery="none"]
)

case "$with_battery" in
     "acpi") AC_CHECK_HEADERS(libacpi.h, enable_linux_acpi=yes, AC_MSG_FAILURE([You need to install libacpi]))
     enable_battery=yes
     ;;
     ""|"apm") AC_CHECK_HEADERS(apm.h, enable_linux_apm=yes, AC_MSG_FAILURE([You need to install apmd]))
     enable_battery=yes
     ;;
     "none"|*) enable_battery=no
     ;;

esac

AM_CONDITIONAL(HAVE_BATTERY, test x$enable_battery = xyes)
AM_CONDITIONAL(HAVE_LIBAPM, test x$enable_linux_apm = xyes)
AM_CONDITIONAL(HAVE_LIBACPI, test x$enable_linux_acpi = xyes)

# glib-genmarshal
GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`  
AC_SUBST(GLIB_GENMARSHAL)

# gettext
GETTEXT_PACKAGE=matchbox-panel
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], 
  [Define the gettext package to be used])
AM_GLIB_GNU_GETTEXT

# output stuff
AC_OUTPUT([
Makefile
matchbox-panel.pc
matchbox-panel/Makefile
applets/Makefile
applets/battery/Makefile
applets/brightness/Makefile
applets/brightness/data/Makefile
applets/clock/Makefile
applets/exit/Makefile
applets/launcher/Makefile
applets/notify/Makefile
applets/showdesktop/Makefile
applets/startup-notify/Makefile
applets/startup/Makefile
applets/startup/data/Makefile
applets/systray/Makefile
applets/windowselector/Makefile
po/Makefile.in
po/Makefile
])
