#!/bin/sh
echo "You need to execute this on a Windows machine within msys (http://www.mingw.org)"
echo "You also need InnoSetup (http://www.innosetup.org) with iscc in your PATH"
echo "Make sure gibbon and all its dependencies have been installed correctly to /c/MinGW"

# This file is mostly stolen from gedit and a mess.  It should be re-written
# from scratch.

#set -x
ISCC=iscc

mingw_prefix="/c/MinGW"
gibbon_prefix="/c/MinGW"
#gibbon_prefix="/local"

if test -f $0.include; then
	. $0.include
fi

revision=$1
if test "$revision" = ''; then
  echo "Installer revision not provided, assuming 1"
  revision=1
fi

echo "Cleanup..."
if test -e installer; then
  rm installer -Rf || exit;
fi

mkdir -p installer || exit

echo "Copying the docs..."
mkdir -p installer/gibbon/share/doc || exit
cp ../COPYING installer/gibbon/share/doc || exit
cp ../AUTHORS installer/gibbon/share/doc || exit
cp ../README installer/gibbon/share/doc || exit

echo "Copying gtk DLL files..."

#----------------------------- gtk ------------------------------------
mkdir -p installer/gtk/bin

cp "${mingw_prefix}/bin/libglib-2.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libgio-2.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libgmodule-2.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libgobject-2.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libgthread-2.0-0.dll" installer/gtk/bin || exit 1

cp "${mingw_prefix}/bin/libatk-1.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libcairo-2.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libpng14-14.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libjpeg-8.dll" installer/gtk/bin || exit 1

cp "${mingw_prefix}/bin/libpango-1.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libpangocairo-1.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libpangowin32-1.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libpangoft2-1.0-0.dll" installer/gtk/bin || exit 1

cp "${mingw_prefix}/bin/libgdk-win32-2.0-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libgdk_pixbuf-2.0-0.dll" installer/gtk/bin || exit 1

cp "${mingw_prefix}/bin/libgtk-win32-2.0-0.dll" installer/gtk/bin || exit 1

cp "${mingw_prefix}/bin/libfontconfig-1.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libexpat-1.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/freetype6.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/zlib1.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libsqlite3-0.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/librsvg-2-2.dll" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/libcroco-0.6-3.dll" installer/gtk/bin || exit 1

echo "Stripping DLL files..."
strip installer/gtk/bin/*.dll || exit 1


# stripping libxml2.dll renders it unusable (although not changing it in size).
# We therefore copy it after having stripped the rest. Same with the other DLLs
# here. Perhaps those were built with MSVC.
mkdir -p installer/gibbon/bin
cp "${mingw_prefix}/bin/libxml2-2.dll" installer/gibbon/bin || exit 1
cp "${mingw_prefix}/bin/intl.dll" installer/gibbon/bin || exit 1
cp "${mingw_prefix}/bin/libiconv-2.dll" installer/gibbon/bin || exit 1


echo "Copying modules..."

mkdir -p installer/gtk/lib/gdk-pixbuf-2.0/2.10.0/loaders
cp "${mingw_prefix}/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.dll" installer/gtk/lib/gdk-pixbuf-2.0/2.10.0/loaders || exit 1

mkdir -p installer/gtk/lib/gtk-2.0/2.10.0/engines
cp "${mingw_prefix}/lib/gtk-2.0/2.10.0/engines/libsvg.dll" installer/gtk/lib/gtk-2.0/2.10.0/engines || exit 1


mkdir -p installer/gtk/lib/gtk-2.0/2.10.0/engines || exit 1
cp "${mingw_prefix}/lib/gtk-2.0/2.10.0/engines/libwimp.dll" installer/gtk/lib/gtk-2.0/2.10.0/engines || exit
strip installer/gtk/lib/gtk-2.0/2.10.0/engines/libwimp.dll || exit 1

cp "${mingw_prefix}/bin/gdk-pixbuf-query-loaders.exe" installer/gtk/bin || exit 1
cp "${mingw_prefix}/bin/gtk-query-immodules-2.0.exe" installer/gtk/bin || exit 1

mkdir -p installer/gtk/share/themes || exit
cp -R "${mingw_prefix}/share/themes/MS-Windows" installer/gtk/share/themes || exit 1
mkdir -p installer/gtk/etc/gtk-2.0 || exit 1
echo "gtk-theme-name = \"MS-Windows\"" > installer/gtk/etc/gtk-2.0/gtkrc || exit

echo "Copying locales..."

# We need to keep the locale files from share/locale in share/locale and those
# from lib/locale in lib/locale:
mkdir -p installer/locale/share/ || exit
cp "${mingw_prefix}/share/locale" installer/locale/share/ -R || exit
cp "${gibbon_prefix}/share/locale" installer/locale/share/ -R || exit

find installer/locale/share/locale/ -type f | grep -v atk10.mo | grep -v gtk20.mo | grep -v GConf2.mo | grep -v glib20.mo | grep -v gibbon.mo | grep -v gtk20.mo | grep -v gtk20-properties.mo | grep -v iso_*.mo | xargs rm
find installer/locale/share/locale -type d | xargs rmdir -p --ignore-fail-on-non-empty

# GConf.
echo "Copying GSettings schemas ..."
mkdir -p installer/gibbon/share/glib-2.0/schemas || exit 1
cp "${gibbon_prefix}/share/glib-2.0/schemas/bg.Gibbon.gschema.xml" installer/gibbon/share/glib-2.0/schemas || exit 1
"${mingw_prefix}/bin/glib-compile-schemas.exe" installer/gibbon/share/glib-2.0/schemas || exit 1

# Pixmaps, etc.
echo "Copying shared data (ui files, icons, etc.)..."
mkdir -p installer/gibbon/share/pixmaps
cp -r "${gibbon_prefix}/share/pixmaps/gibbon" installer/gibbon/share/pixmaps ||exit 1

# GtkBuilder file and GeoIP database.
mkdir -p installer/gibbon/share/gibbon || exit 1
cp "${gibbon_prefix}/share/gibbon/gibbon.ui" installer/gibbon/share/gibbon || exit 1
cp "${gibbon_prefix}/share/gibbon/ip2country.csv.gz" installer/gibbon/share/gibbon || exit 1

echo "Copying executable..."
mkdir -p installer/gibbon/bin
cp "${gibbon_prefix}/bin/gibbon.exe" installer/gibbon/bin || exit 1
strip installer/gibbon/bin/gibbon.exe || exit 1

# Dummies to make sure that the file get uninstalled.
mkdir -p installer/Gibbon/etc/gtk-2.0
touch installer/Gibbon/etc/gtk-2.0/gtk.immodules || exit 1
mkdir -p installer/Gibbon/lib/gdk-pixbuf-2.0/2.10.0
touch installer/Gibbon/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache || exit 1

echo "gdk-pixbuf-query-loaders.exe > ../lib/gdk-pixbuf-2.0/2.10.0/loaders.cache" >installer/gibbon/bin/querymodules.bat
echo "gtk-query-immodules-2.0.exe > ../etc/gtk-2.0/gtk.immodules" >>installer/gibbon/bin/querymodules.bat

echo "Creating installer..."

cp license.txt installer/license.txt
cp *.isl installer

perl -pe "s/INSTALLERREVISION/$revision/" gibbon.iss >installer/gibbon.iss || exit | 1
"$ISCC" installer/gibbon.iss >/dev/null || exit 1

mv installer/Output/gibbon-setup-*.exe .

rm -r installer

echo "Successfully created " gibbon-setup-*.exe
