#!/bin/sh
#
# autogen.sh
#
#	$Id: autogen.sh,v 1.1 2002/11/14 12:08:05 guillem Exp $
#
# Authors:	Guillem Jover <guillem.jover@menta.net>
#
# License:
#
#	Copyright (C) 2002 Guillem Jover
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
# Requires:	gettext, automake, autoconf
#

set -e

srcdir=`dirname $0`
if test -e $srcdir/src/xfstt.cc 
then
	:
else
	echo "Not xfstt source dir"
	exit 1
fi

test -d config || mkdir config

echo "+++ autopoint"
autopoint

#echo "+++ gettextize"
#gettextize --intl --force
#echo "  + working around a nasty bug in gettext 0.11.5"
#sed -e 's,^\(AM_GNU_GETTEXT_VERSION([^)]\)$,\1),' < configure.ac > configure.ac.tmp
#mv -f configure.ac.tmp configure.ac

echo "+++ aclocal "
aclocal -I config

#echo "  + removing unnecessary m4/ directory found in aclocal.m4"
#rm -rf m4/

echo "+++ autoheader "
autoheader
echo "+++ automake "
automake --verbose --add-missing
echo "+++ autoconf"
autoconf

echo "+++ cleaning cruft files"
find . -name '*~' | xargs rm -f
rm -rf autom4te.cache/

