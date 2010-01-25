## Process this file with automake to produce Makefile.in
#
# xfstt Makefile
#
# Copyright © 2002, 2004, 2010 Guillem Jover <guillem@hadrons.org>
#

man_MANS = xfstt.1x

EXTRA_DIST = xfstt.1x.in xfstt.lsm FAQ examples
CLEANFILES = $(man_MANS)

edit = sed \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@datadir\@,$(pkgdatadir),g' \
	-e 's,@localstatedir\@,$(localstatedir),g'

xfstt.1x: Makefile $(srcdir)/xfstt.1x.in
	$(AM_V_GEN) $(edit) $(srcdir)/xfstt.1x.in > $@

