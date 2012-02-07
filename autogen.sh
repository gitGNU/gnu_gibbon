# /bin/sh
#
# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
#
# Gibbon is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Gibbon is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.

glib-gettextize --copy --force
if  test $? != 0; then
        echo The script glib-gettextize was not found in your path
        echo or terminated with an error.  Copying possibly outdated 
        echo versions of the files into the project!
        echo
        echo You can prevent this error by installing a recent enough
        echo version of glib completely, including developer files.
        echo Press CTRL-C for interrupt or wait otherwise to continue.
        echo -e -n "[-------------------------]\r["
        for i in 1 2 3 4 5; do
                sleep 1
                echo -n +++++
        done 
        echo
        cp -prd fallback/glib-gettextize/* . || exit 1
fi

intltoolize --copy --force --automake
if test $? != 0; then
        echo The script intltoolize was not found in your path
        echo or terminated with an error.  Copying possibly outdated 
        echo versions of the files into the project!
        echo
        echo You can prevent this error by installing a recent enough
        echo version of GNU intltool completely, including developer files.
        echo Press CTRL-C for interrupt or wait otherwise to continue.
        echo -e -n "[-------------------------]\r["
        for i in 1 2 3 4 5; do
                sleep 1
                echo -n +++++
        done 
        echo
        cp -prd fallback/intltoolize/* . || exit 1
fi

gtkdocize --flavour no-tmpl
if test $? != 0; then
        echo The script gtkdocize was not found in your path
        echo or terminated with an error.  Copying possibly outdated 
        echo versions of the files into the project!
        echo
        echo Not that you cannot create a distribution of Gibbon without
        echo the gtk-doc developer package.
        echo
        echo You can prevent this error by installing a recent enough
        echo version of gtk-doc completely, including developer files.
        echo Press CTRL-C for interrupt or wait otherwise to continue.
        echo -e -n "[-------------------------]\r["
        for i in 1 2 3 4 5; do
                sleep 1
                echo -n +++++
        done 
        echo
        cp -prd fallback/gtkdocize/* . || exit 1
fi

# The remaining build tools are considered mandatory.  Only
# libtoolize is sometimes installed as glibtoolize.
glibtoolize --version >/dev/null 2>&1
test $? = 0 && libtoolize=glibtoolize || libtoolize=libtoolize
$libtoolize --copy --force --automake || exit 1

aclocal -I m4                         || exit 1
autoheader                            || exit 1
automake --gnu --add-missing --force-missing || exit 1
autoconf                                     || exit 1

echo 
echo You can \(probably\) safely ignore all warnings above.
echo Now run the configure script with the appropriate options.
echo
