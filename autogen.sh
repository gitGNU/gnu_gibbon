# /bin/sh
#
# This file is part of Gibbon, a graphical frontend to the First Internet 
# Backgammon Server FIBS.
# Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

set -x

glib-gettextize --copy --force        || exit 1
intltoolize --copy --force --automake || exit 1
libtoolize --copy --force --automake  || exit 1
gtkdocize                             || exit 1
aclocal -I m4                         || exit 1
autoheader                            || exit 1
automake --gnu --add-missing --force-missing --warnings=all || exit 1
autoconf --warnings=all                                     || exit 1

set +x

echo 
echo You can \(probably\) safely ignore all warnings above.
echo Now run the configure script with the appropriate options.
echo
