## Process this file with automake to produce Makefile.in
#
# xfstt Makefile
#
#	$Id: Makefile.am,v 1.2 2003/01/05 10:16:28 guillem Exp $
#
# Copyright (C) 2002 Guillem Jover <guillem.jover@menta.net>
#

localedir = $(datadir)/locale
fontdir = $(datadir)/fonts/truetype
cachedir = $(localstatedir)/cache/xfstt
pidfile = $(localstatedir)/run/xfstt.pid

LDADD = @LIBINTL@ -L$(top_srcdir)/libfstt -lfstt

AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)\"" -DFONTDIR=\""$(fontdir)\"" \
	      -DCACHEDIR=\""$(cachedir)\"" -DPIDFILE=\""$(pidfile)\"" \
	      -DMAGNIFY=0 -I$(top_srcdir)/libfstt

bin_PROGRAMS = xfstt
EXTRA_PROGRAMS = patchttf perftest showttf

xfstt_SOURCES = xfstt.cc xfstt.h ttfn.h encoding.cc encoding.h gettext.h
xfstt_CXXFLAGS = $(X_CFLAGS)

patchttf_SOURCES = patchttf.cc gettext.h

showttf_SOURCES = showttf.cc gettext.h
showttf_CXXFLAGS = $(X_CFLAGS)
showttf_LDADD = $(LDADD) $(X_LIBS) $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)

perftest_SOURCES = perftest.cc gettext.h

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(fontdir)
	$(INSTALL) -d $(DESTDIR)$(cachedir)

