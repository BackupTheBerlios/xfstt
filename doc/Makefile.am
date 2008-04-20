## Process this file with automake to produce Makefile.in
#
# xfstt Makefile
#
# Copyright (C) 2002,2004 Guillem Jover <guillem@hadrons.org>
#

man_MANS = xfstt.1x

EXTRA_DIST = $(man_MANS) xfstt.lsm FAQ examples

edit = sed \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@datadir\@,$(pkgdatadir),g' \
	-e 's,@localstatedir\@,$(localstatedir),g'

xfstt.1x: Makefile $(srcdir)/xfstt.1x.in
	$(edit) $(srcdir)/xfstt.1x.in > $@

