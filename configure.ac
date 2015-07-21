AC_INIT([ini], [0.1], [dev@2ion.de])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_PROG_CC_C99 

AC_CONFIG_SRCDIR([src/ini.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_PATH_PROG([PANDOC], pandoc, no)
AM_CONDITIONAL([HAVE_PANDOC],[test ! "x$PANDOC" = xno])
AM_COND_IF([HAVE_PANDOC],[AC_CONFIG_FILES([doc/Makefile])])

if test "x$PKG_CONFIG_PATH" = x; then
  export PKG_CONFIG_PATH=""
fi

PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$PWD/pkg-config"

PKG_CHECK_MODULES([INIPARSER], iniparser)

AH_VERBATIM([_FORTIFY_SOURCE],
[/* Fortify source in gcc */
#if __OPTIMIZE__
#define _FORTIFY_SOURCE 2
#endif
])

AC_OUTPUT
