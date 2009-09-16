dnl This file is part of Gibbon 
dnl Copyright (C) 2009 Guido Flohr
dnl 
dnl Gibbon is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 3 of the License, or (at your option) any later version.
dnl
dnl Gibbon is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with Gibbon; if not, see <http://www.gnu.org/licenses/>.

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

AC_PATH_PROG(GCONFTOOL, gconftool-2)
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
m4_define(gibbon_requirements,
    gibbon_glib_requirement
    gibbon_gtk_requirement
    gibbon_gconf_requirement
)
PKG_CHECK_MODULES(GIBBON, gibbon_requirements)
GIBBON_PKGCONFIG_REQUIREMENTS=["] gibbon_glib_requirement[, ] gibbon_gtk_requirement[, ]gibbon_gconf_requirement ["]
AC_SUBST(GIBBON_PKGCONFIG_REQUIREMENTS)

GIBBON_LIBS="${GIBBON_LIBS}"
AC_SUBST(GIBBON_LIBS)
AC_SUBST(GIBBON_CFLAGS)

AM_GCONF_SOURCE_2

AC_CONFIG_FILES([ 
    Makefile
    src/Makefile
    data/gibbon.desktop.in
    po/Makefile.in
])
AC_OUTPUT

echo "

Configuration:

    Source code location:                       ${srcdir}
    Compiler:                                   ${CC}

"
