## Process this file with automake to produce Makefile.in
#
# xfstt Makefile
#
# Copyright © 2002, 2010 Guillem Jover <guillem@hadrons.org>
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
EXTRA_PROGRAMS = patchttf perftest

xfstt_SOURCES = xfstt.cc xfstt.h ttfn.h encoding.cc encoding.h mesg.h gettext.h
xfstt_CXXFLAGS = $(X_CFLAGS)

patchttf_SOURCES = patchttf.cc gettext.h

if WITH_X11
EXTRA_PROGRAMS += showttf

showttf_SOURCES = showttf.cc gettext.h
showttf_CXXFLAGS = $(X_CFLAGS)
showttf_LDADD = $(LDADD) $(X_LIBS) $(X_PRE_LIBS) -lX11 $(X_EXTRA_LIBS)
endif

perftest_SOURCES = perftest.cc gettext.h

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(fontdir)
	$(INSTALL) -d $(DESTDIR)$(cachedir)

