#!/bin/sh
#
# autogen.sh
#
#	$Id$
#
# Authors:	Guillem Jover <guillem@hadrons.org>
#		Midnight Commander Authors
#
# License:
#
#	Copyright (C) 2002 Guillem Jover
#	Some parts of this script come from Midnight Commander's autogen.sh
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
# Requires:	gettext, automake, autoconf
#

PROJECT=xfstt
srcfile=src/xfstt.cc

set -e

# Make it possible to specify path in the environment
: ${AUTOCONF=autoconf}
: ${AUTOHEADER=autoheader}
: ${AUTOMAKE=automake}
: ${ACLOCAL=aclocal}
: ${GETTEXTIZE=gettextize}
: ${AUTOPOINT=autopoint}

srcdir=`dirname $0`
if test -e $srcdir/$srcfile
then
	:
else
	echo "autogen.sh: No $PROJECT source dir" 2>&1
	exit 1
fi

test -d config || mkdir config

# Ensure that gettext is reasonably new.
gettext_ver=`$GETTEXTIZE --version | \
  sed '2,$d;			# remove all but the first line
       s/.* //;			# take text after the last space
       s/-.*//;			# strip "-pre" or "-rc" at the end
       s/\([^.][^.]*\)/0\1/g;	# prepend 0 to every token
       s/0\([^.][^.]\)/\1/g;	# strip leading 0 from long tokens
       s/$/.00.00/;		# add .00.00 for short version strings
       s/\.//g;			# remove dots
       s/\(......\).*/\1/;	# leave only 6 leading digits
       '`

if test $gettext_ver -lt 01038; then
	echo "autogen.sh: Don't use gettext earlier than 0.10.38" 2>&1
	exit 1
fi

rm -rf intl
if test $gettext_ver -ge 01100; then
	if test $gettext_ver -lt 01105; then
		echo "autogen.sh: Upgrade gettext to at least 0.11.5 or downgrade to 0.10.40" 2>&1
		exit 1
	fi
	echo "-> $AUTOPOINT"
	$AUTOPOINT --force || exit 1
else
	echo "-> $GETTEXTIZE"
	echo "autogen.sh: Warning -- gettextize is not designed to be used automatically," 2>&1
	echo "            so problems may arise. Upgrade to at least gettext 0.11.5" 2>&1
	$GETTEXTIZE --intl --copy --force || exit 1
	#echo "  + working around a nasty bug in gettext 0.11.5"
	#sed -e 's,^\(AM_GNU_GETTEXT_VERSION([^)]\)$,\1),' < configure.ac > configure.ac.tmp
	#mv -f configure.ac.tmp configure.ac
	if test -e po/ChangeLog~; then
		rm -f po/ChangeLog
		mv po/ChangeLog~ po/ChangeLog
	fi
fi

echo "-> $ACLOCAL"
$ACLOCAL -I config

#echo "  + removing unnecessary m4/ directory found in aclocal.m4"
#rm -rf m4/

echo "-> $AUTOHEADER"
$AUTOHEADER -Wall || exit 1
echo "-> $AUTOMAKE"
$AUTOMAKE -Wall --add-missing --copy || exit 1
echo "-> $AUTOCONF"
$AUTOCONF -Wall || exit 1

echo "-> cleaning cruft files"
find . -name '*~' | xargs rm -f
rm -rf autom4te.cache/

