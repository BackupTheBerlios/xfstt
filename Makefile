#MAXOPT = -O6 -fomit-frame-pointer -ffast-math
MISCOPT =
OPT = $(MISCOPT) $(MAXOPT)

CFLAGS = $(OPT) -DMAGNIFY=0 -DNDEBUG
#CFLAGS = -fprofile-arcs -ftest-coverage -DMAGNIFY=0
CFLAGS = -g -Wall -pedantic $(MISCOPT) -DMAGNIFY=0
#CFLAGS = -O -Wall -pedantic -DDEBUG $(MISCOPT)

LFLAGS = -L/usr/X11R6/lib -L/usr/openwin/lib/X11
LFLAGS = -g -L/usr/X11R6/lib

CC = c++
LD = c++

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

all : xfstt

install :
	mkdir -p /usr/share/fonts/truetype ;	\
	mkdir -p /var/cache/xfstt;			\
	ln -s /DOS/windows/fonts /usr/share/fonts/truetype/winfonts ;	\
	cp xfstt /usr/X11R6/bin/ ;	\
	cp xfstt.1x /usr/X11R6/man/man1 ;\
	xfstt --sync

clean :
	rm *.o;							\
	rm xfstt showttf perftest patchttf

xfstt : $(OBJS) xfstt.o encoding.o
	$(LD) -o $@ $(OBJS) xfstt.o encoding.o $(LFLAGS) -lm

xfstt.o : xfstt.cpp xfstt.h ttf.h arch.h Makefile
	$(CC) $(CFLAGS) -c $< -I/usr/X11R6/include/X11/fonts	\
		-I/usr/X11R6/include/

encoding.o : encoding.cpp encoding.h Makefile
	$(CC) $(CFLAGS) -c $<

showttf : $(OBJS) showttf.o ttf.h arch.h Makefile
	$(LD) -o $@ $(OBJS) showttf.o $(LFLAGS) -lX11 -lm

perftest : $(OBJS) perftest.o ttf.h arch.h Makefile
	$(LD) -o $@ $(OBJS) perftest.o $(LFLAGS) -lm

patchttf : patchttf.cpp
	$(CC) $(CFLAGS) -o $@ $<

tgz:
	cp xfstt.tgz xfstt.tgz.bak	;			\
	tar cvf xfstt.tar Makefile *.h *cpp *.txt *.sh *lsm *1x &&\
	tar rvf xfstt.tar font.properties FAQ INSTALL CHANGES COPYING &&\
	gzip -9 xfstt.tar		&&			\
	cp xfstt.tar.gz /dosd/source/xfstt.tgz &&		\
	mv xfstt.tar.gz xfstt.tgz	;			\
	sync;							\
	ls -l /dosd/source/xfstt.tgz xfstt.tgz

%.o : %.cpp ttf.h arch.h Makefile
	$(CC) $(CFLAGS) -c $<

%.s : %.cpp ttf.h arch.h Makefile
	$(CC) -S $(CFLAGS) -c $<

.SUFFIXES: .cpp
.cpp.o : ttf.h arch.h Makefile
	$(CC) $(CFLAGS) -c $<

