AC_PREREQ(2.53)
AC_INIT(matchbox-panel, 0.1, http://www.openedhand.com/)
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR(matchbox-panel/mb-panel.c)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL

PKG_CHECK_MODULES(MATCHBOX_PANEL,
                  glib-2.0
                  gtk+-2.0 >= 2.6
                  gmodule-2.0)

AC_ARG_ENABLE(startup_notification,
  [  --enable-startup-notification   enable startup notification support],
     enable_startup_notification=$enableval, enable_startup_notification=yes )

if test x$enable_startup_notification != xno; then
  PKG_CHECK_MODULES(SN, libstartup-notification-1.0, ,
    AC_MSG_ERROR([*** Required Startup Notification library not installed ***]))

  AC_DEFINE(USE_LIBSN, [1], [Has Startup Notification Support])
fi

GETTEXT_PACKAGE=matchbox-panel
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], 
  [Define the gettext package to be used])
  
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

AC_OUTPUT([
Makefile
matchbox-panel.pc
matchbox-panel/Makefile
applets/Makefile
po/Makefile.in
po/Makefile
])
