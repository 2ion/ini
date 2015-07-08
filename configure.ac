AC_INIT([ini], [0.1], [dev@2ion.de])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CC_C99 

AC_CONFIG_SRCDIR([src/ini.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

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
