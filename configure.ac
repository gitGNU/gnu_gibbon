dnl This file is part of Gibbon, a graphical frontend to the First Internet 
dnl Backgammon Server FIBS.
dnl Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
m4_define(gibbon_minor_version, 1)
m4_define(gibbon_micro_version, 1)
m4_define(gibbon_version, gibbon_major_version.gibbon_minor_version.gibbon_micro_version)
m4_define(gibbon_api_version, gibbon_major_version.gibbon_minor_version)
m4_define(gibbon_interface_age, 3)

AC_INIT([gibbon],[gibbon_version])
AC_CONFIG_SRCDIR(src/gibbon.c)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.10 gnu])

AM_MAINTAINER_MODE

AC_CONFIG_HEADERS(config.h)

AC_PROG_LIBTOOL

AC_PATH_PROG(GCONFTOOL, gconftool-2)
AC_PATH_PROG(GTK_BUILDER_CONVERT, gtk-builder-convert)
IT_PROG_INTLTOOL([0.40.0])

PKG_PROG_PKG_CONFIG

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PATH_PROG(GZIP, gzip)

AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")

GETTEXT_PACKAGE=gibbon
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT([external])

m4_define(gibbon_glib_requirement, [glib-2.0 >= 2.13.0])
m4_define(gibbon_gtk_requirement, [gtk+-2.0 >= 2.13.0])
m4_define(gibbon_gconf_requirement, [gconf-2.0])
m4_define(gibbon_libglade_requirement, [libglade-2.0 >= 2.6.0])
m4_define(gibbon_requirements,
    gibbon_glib_requirement
    gibbon_gtk_requirement
    gibbon_gconf_requirement
    gibbon_libglade_requirement
)

PKG_CHECK_MODULES(GIBBON, gibbon_requirements)
GIBBON_PKGCONFIG_REQUIREMENTS=["] gibbon_glib_requirement[, ] gibbon_gtk_requirement[, ]gibbon_gconf_requirement[, ]gibbon_libglade_requirement ["]
AC_SUBST(GIBBON_PKGCONFIG_REQUIREMENTS)

AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, connect)

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

GIBBON_LIBS="${GIBBON_LIBS}"
AC_SUBST(GIBBON_LIBS)
AC_SUBST(GIBBON_CFLAGS)

AM_GCONF_SOURCE_2

AC_CONFIG_FILES([ 
    Makefile
    src/Makefile
    data/Makefile
    data/gibbon.desktop.in
    po/Makefile.in
])
AC_OUTPUT

echo "

Configuration:

    Source code location:                       ${srcdir}
    Compiler:                                   ${CC}

"
