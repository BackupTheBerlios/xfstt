#
# xfstt Makefile
#

VERSION=$(shell head -1 CHANGES)

OPTIMIZE_FLAGS = -O2 -fomit-frame-pointer -ffast-math
PROFILE_FLAGS = -fprofile-arcs -ftest-coverage
DEBUG_FLAGS = -g -DDEBUG

CXXFLAGS = -Wall $(OPTIMIZE_FLAGS) 
ALL_CXXFLAGS = -DMAGNIFY=0 $(CXXFLAGS)

DIST_FILES = Makefile *.h *cpp *.sh *lsm *1x \
 font.properties README FAQ INSTALL CHANGES THANKS COPYING

OBJS =	RAFile.o	\
	TTFont.o	\
	NameTable.o	\
	HeadTable.o	\
	MaxpTable.o	\
	CmapTable.o	\
	LocaTable.o	\
	GlyfTable.o	\
	LtshTable.o	\
	HdmxTable.o	\
	VdmxTable.o	\
	FpgmTable.o	\
	PrepTable.o	\
	GaspTable.o	\
	HheaTable.o	\
	HmtxTable.o	\
	CvtTable.o	\
	Os2Table.o	\
	KernTable.o	\
	EbdtTable.o	\
	EblcTable.o	\
	RasterHints.o	\
	RasterScale.o	\
	RasterDraw.o

.PHONY: all install clean dist

all: xfstt

install:  xfstt
	install -d $(DESTDIR)/usr/X11R6/bin
	install xfstt $(DESTDIR)/usr/X11R6/bin
	install -d $(DESTDIR)/usr/X11R6/man/man1
	install xfstt.1x $(DESTDIR)/usr/X11R6/man/man1
	install -d $(DESTDIR)/usr/share/fonts/truetype
	install -d $(DESTDIR)/var/cache/xfstt

clean:
	rm -f *.o
	rm -f version.h
	rm -f xfstt showttf perftest patchttf

xfstt: $(OBJS) xfstt.o encoding.o
	$(CXX) -o $@ $^ -lm $(LDFLAGS)

xfstt.o: xfstt.cpp xfstt.h ttf.h arch.h version.h Makefile
	$(CXX) -c $< -I/usr/X11R6/include/X11/fonts	\
		-I/usr/X11R6/include/ $(ALL_CXXFLAGS)

encoding.o: encoding.cpp encoding.h Makefile
	$(CXX) -c $< $(ALL_CXXFLAGS)

showttf: $(OBJS) showttf.o ttf.h arch.h Makefile
	$(CXX) -o $@ $(OBJS) showttf.o -L/usr/X11R6/lib -lX11 -lm $(LDFLAGS)

perftest: $(OBJS) perftest.o ttf.h arch.h Makefile
	$(CXX) -o $@ $(OBJS) perftest.o -lm $(LDFLAGS)

patchttf: patchttf.cpp
	$(CXX) -o $@ $< $(ALL_CXXFLAGS)

version.h: CHANGES
	@ ( \
	echo "#ifndef xfstt_version"; \
	echo "#define xfstt_version \"$(VERSION)\""; \
	echo "#endif"; \
	) > $@

dist:
	GZIP="-9" tar cvzf xfstt-$(VERSION).tar.gz $(DIST_FILES)

%.o: %.cpp ttf.h arch.h Makefile
	$(CXX) -c $< $(ALL_CXXFLAGS)

%.s: %.cpp ttf.h arch.h Makefile
	$(CXX) -S -c $< $(ALL_CXXFLAGS)

