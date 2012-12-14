dnl This file is part of Gibbon, a graphical frontend to the First Internet 
dnl Backgammon Server FIBS.
dnl Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
dnl
dnl Gibbon is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl Gibbon is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ(2.63)

m4_define(gibbon_major_version, 0)
m4_define(gibbon_minor_version, 2)
m4_define(gibbon_micro_version, 0)
m4_define(gibbon_version, gibbon_major_version.gibbon_minor_version.gibbon_micro_version)

AC_INIT([gibbon],[gibbon_version],[mailto:guido@imperia.net],[gibbon],[http://guido-flohr.net/projects/gibbon])
AC_CONFIG_SRCDIR(src/gibbon.c)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11.1 gnu dist-xz dist-bzip2])
AM_SILENT_RULES([yes])

AC_CANONICAL_HOST

AM_MAINTAINER_MODE

AC_MSG_CHECKING([for native Windows])
case "$host" in
	*-*-mingw*)
		AC_CHECK_TOOL(WINDRES, windres)
		gibbon_native_win32=yes
        	;;
	*)
		gibbon_native_win32=no
		;;
esac
AC_MSG_RESULT([$gibbon_native_win32])
AM_CONDITIONAL(WIN32, test x$gibbon_native_win32 = xyes)
dnl Under Windows files have a version number separated by comma.  It is
dnl important to increase that with every new version.
GIBBON_COMMA_VERSION=["]gibbon_major_version","gibbon_minor_version","gibbon_micro_version["]
AC_SUBST(GIBBON_COMMA_VERSION)

AC_LANG([C])

AC_PREPROC_IFELSE(
  [AC_LANG_PROGRAM([[#include <Carbon/Carbon.h]],
                    [[#include <CoreServices/CoreServices.h]])],
  [gibbon_native_osx=yes],
  [gibbon_native_osx=no])
AC_MSG_CHECKING([for Mac OS X Carbon support])
AC_MSG_RESULT([$gibbon_native_osx])
AM_CONDITIONAL(OSX, test x$gibbon_native_osx = xyes)

AM_CONDITIONAL(BUILD_HELP, test x$gibbon_native_win32$gibbon_native_osx = xnono)

AC_ARG_ENABLE(gtk3-migration,
              AC_HELP_STRING([--enable-gtk3-migration],
                             [Enable additional compiler flags to check for compatibility with gtk+-3.0 [default=no]]),
              [gibbon_gtk_migration_pre_flight=$enableval])
AM_CONDITIONAL(GIBBON_GTK_MIGRATION_PRE_FLIGHT, test "x$gibbon_gtk_migration_pre_flight" = "xyes")

AC_CONFIG_HEADERS(config.h)

LT_INIT

IT_PROG_INTLTOOL([0.40.6])

AC_PATH_PROG(LWP_REQUEST, "lwp-request")
AC_PATH_PROG(LYNX, "lynx")
AC_PATH_PROG(LINKS, "links")
AC_PATH_PROG(W3M, "w3m")
AC_PATH_PROG(WGET, "wget")

PKG_PROG_PKG_CONFIG

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PATH_PROG(GZIP, gzip)
AC_PROG_YACC
AC_PROG_LEX

AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

GETTEXT_PACKAGE=gibbon
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT([external])

dnl On the Mac we need the new NeXTSTEP backend for GSettings.
if test "x$gibbon_native_osx" = "xyes"; then
    gibbon_glib_version=2.31.8
else
    gibbon_glib_version=2.26.0
fi
m4_define(gibbon_glib_requirement, [glib-2.0 >= $gibbon_glib_version])
m4_define(gibbon_librsvg_requirement, [librsvg-2.0 >= 2.32.1])
m4_define(gibbon_gtk_requirement, [gtk+-2.0 >= 2.24.0])
m4_define(gibbon_gthread_requirement, [gthread-2.0 >= 2.20.5])
m4_define(gibbon_libpng_requirement, [libpng >= 0.23])
m4_define(gibbon_libxml_requirement, [libxml-2.0 >= 2.7.3])
m4_define(gibbon_sqlite3_requirement, [sqlite3 >= 3.6.19])
m4_define(gibbon_zlib_requirement, [zlib >= 1.2.3])
m4_define(gibbon_requirements,
    gibbon_glib_requirement
    gibbon_librsvg_requirement
    gibbon_gtk_requirement
    gibbon_gthread_requirement
    gibbon_libpng_requirement
    gibbon_libxml_requirement
    gibbon_sqlite3_requirement
    gibbon_zlib_requirement
)

PKG_CHECK_MODULES(GIBBON, gibbon_requirements)
GIBBON_PKGCONFIG_REQUIREMENTS=["] gibbon_glib_requirement[, ]gibbon_librsvg_requirement[, ]gibbon_gtk_requirement[, ]gibbon_gthread_requirement[, ]gibbon_libpng_requirement[, ]gibbon_libxml_requirement[, ]gibbon_sqlite3_requirement[, ]gibbon_zlib_requirement["]
AC_SUBST(GIBBON_PKGCONFIG_REQUIREMENTS)

AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(jpeg, jpeg_start_decompress, 
    test_jpeg=ok,
    AC_MSG_ERROR([jpeg library not found]))

if test $gibbon_native_win32 = no; then
  AC_CHECK_FUNC(res_query, ,
    [AC_CHECK_LIB(resolv, res_query, [],
      [ save_libs="$LIBS"
        LIBS="-lresolv $LIBS"
        AC_MSG_CHECKING([for res_query in -lresolv (alternate version)])
        AC_LINK_IFELSE(
          [AC_LANG_PROGRAM([[#include <resolv.h>]], [[res_query(0,0,0,0,0)]])], 
          [ AC_MSG_RESULT(yes) ],
          [ AC_MSG_RESULT(no)
            LIBS="$save_libs"
            AC_CHECK_LIB(bind, res_query,
              [], [ AC_MSG_ERROR(res_query not found) ] ) ] ) ] )
    ]
  )
fi

AC_CHECK_FUNCS([index rindex])

AC_CHECK_HEADERS([xlocale.h])
AC_CHECK_FUNCS([newlocale uselocale])

GIBBON_LIBS="${GIBBON_LIBS} -ljpeg"
AC_SUBST(GIBBON_LIBS)
AC_SUBST(GIBBON_CFLAGS)

GLIB_GSETTINGS

dnl gtk-doc 1.16 produces a failure for missing overview.xml after
dnl "make distcheck".
GTK_DOC_CHECK([1.17],[--flavour no-tmpl])

GCREATE_CLASS_INIT([Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.],[2009-2012 Guido Flohr, http://guido-flohr.net/])

YELP_HELP_INIT

AC_CONFIG_FILES([ 
    Makefile
    gcreate_class.pl
    download_uri
    libgsgf/Makefile
    libgsgf/examples/Makefile
    libgsgf/tests/Makefile
    libgsgf/tests/compressed-list.sgf
    libgsgf/tests/full.sgf
    libgsgf/tests/utf-8.sgf
    libsvg/Makefile
    libsvg-cairo/Makefile
    src/Makefile
    src/gibbon.rc
    data/Makefile
    data/gibbon.1
    data/gibbon-convert.1
    data/gibbon.desktop.in
    pixmaps/Makefile
    pixmaps/boards/Makefile
    pixmaps/icons/Makefile
    pixmaps/icons/16x16/Makefile
    pixmaps/icons/scalable/Makefile
    pixmaps/flags/Makefile
    pixmaps/flags/16x16/Makefile
    po/Makefile.in
    docs/Makefile
    docs/reference/Makefile
    docs/reference/libgsgf/Makefile
    docs/reference/libgsgf/version.xml
    help/Makefile
    win32/Makefile
    win32/gibbon.iss
    share/Makefile
    share/icons/Makefile
    share/icons/application/Makefile
    share/icons/application/16x16/Makefile
    share/icons/application/22x22/Makefile
    share/icons/application/24x24/Makefile
    share/icons/application/32x32/Makefile
    share/icons/application/48x48/Makefile
    share/icons/application/128x128/Makefile
    share/icons/application/256x256/Makefile
    share/icons/application/scalable/Makefile
])
AC_OUTPUT

echo "

Configuration:

    Source code location:                       ${srcdir}
    Compiler:                                   ${CC}

"
