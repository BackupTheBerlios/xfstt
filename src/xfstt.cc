/*
 * X Font Server for *.ttf Files
 *
 * $Id: xfstt.cc,v 1.9 2003/06/25 04:37:30 guillem Exp $
 *
 * Copyright (C) 1997-1999 Herbert Duerr
 * portions are (C) 1999 Stephen Carpenter and others
 * portions are (C) 2002 Guillem Jover
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/* the unstrap limit is set to 10500 characters in order to limit
 * getextent replies to (10500|255)*24Bytes < 256kBytes;
 * if you are sure your X11 server doesn't request more
 * than it can handle, increase the limit up to 65535
 */
#define UNSTRAPLIMIT	10500U

#define TTINFO_LEAF	"ttinfo.dir"
#define TTNAME_LEAF	"ttname.dir"

#define MAXOPENFONTS	256
#define MAXREPLYSIZE	(1 << 22)
#define MAXFONTBUFSIZE	(1 << 24)
#define MINFONTBUFSIZE	(1 << 18)


#include "config.h"
#include "gettext.h"
#include <locale.h>
#define _(str) gettext(str)
#define N_(str) gettext_noop(str)
#include "ttf.h"
#include "ttfn.h"
#include "xfstt.h"
#include "encoding.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <time.h>
#include <X11/fonts/FS.h>
#include <X11/fonts/FSproto.h>
#include <pwd.h>


// if you want to read good code skip this hacked up file!

typedef struct {
	Font		fid;
	TTFont		*ttFont;
	FontInfo	fi;
	FontExtent	fe;
	Encoding	*encoding;
} XFSFont;

XFSFont xfsFont[MAXOPENFONTS];

#ifndef MAGNIFY
int MAGNIFY = 0;
#endif /* MAGNIFY */

U16 maxLastChar = 255;

static unsigned infoSize, nameSize, aliasSize;
static char *infoBase, *nameBase, *aliasBase;
char *fontdir = FONTDIR;
char *cachedir = CACHEDIR;
char *pidfilename = PIDFILE;

int defaultres = 0;
const int default_port = 7101;
int noTCP = 0;

uid_t newuid = (uid_t)(-2);
gid_t newgid = (uid_t)(-2);

char *sockname;
char *sockdir = "/tmp/.font-unix";

#define MAXENC 16	/* Maximum number of encodings */
Encoding *encodings[MAXENC];


static void
usage(int verbose)
{
	printf(_("xfstt %s, X font server for truetype fonts\n"), VERSION);
	printf(_("Usage: xfstt [--gslist][--sync][--port portno][--unstrap]"
		"[--user username]\n"
		"\t\t[--dir ttfdir][--cache ttfcachedir][--pidfile pidfile]\n"
		"\t\t[--encoding list_of_encodings][--res resolution]\n"
		"\t\t[--notcp][--daemon][--inetd]\n\n"));

	if (!verbose)
		return;

	printf(_("\t--sync     sync database with font directory content\n"));
	printf(_("\t--gslist   print ghostscript style ttf fontlist\n"));
	printf(_("\t--port     change port number (default %i)\n"),
	       default_port);
	printf(_("\t--notcp    don't open TCP socket, use unix domain only\n"));
	printf(_("\t--dir      change font directory\n"));
	printf(_("\t           (default %s)\n"), FONTDIR);
	printf(_("\t--cache    change font cache directory\n"));
	printf(_("\t           (default %s)\n"), CACHEDIR);
	printf(_("\t--pidfile  change pid file location\n"));
	printf(_("\t           (default %s)\n"), PIDFILE);
	printf(_("\t--res      force default resolution to this value\n"));
	printf(_("\t--encoding change encoding (default %s)\n"),
	       encodings[0]->strName);
	printf(_("\t--unstrap  !DANGER! serve all unicodes !DANGER!\n"));
	printf(_("\t--user     username that children should run as\n"));
	printf(_("\t--once     exit after the font client disconnects\n"));
	printf(_("\t--daemon   run in the background\n"));
	printf(_("\t--inetd    run as inetd service\n"));
	printf(_("\n\tattach to X Server by \"xset fp+ unix/:%i\"\n"),
	       default_port);
	printf(_("\t\tor \"xset fp+ inet/127.0.0.1:%i\"\n"), default_port);
	printf(_("\tdetach from X Server by \"xset -fp unix/:%u\"\n"),
	       default_port);
	printf(_("\t\tor \"xset -fp inet/127.0.0.1:%i\"\n"), default_port);
}

static int
ttSyncDir(FILE *infoFile, FILE *nameFile, char *ttdir, int gslist)
{
	int nfonts = 0;
	if (!gslist)
		printf(_("xfstt: sync in directory \"%s/%s\"\n"), fontdir, ttdir);
	DIR *dirp = opendir(".");

	while (dirent *de = readdir(dirp)) {
		int namelen = strlen(de->d_name);
		if (namelen - 4 <= 0)
			continue;
		char *ext = &de->d_name[namelen - 4];
		if (ext[0] != '.')
			continue;
		if (tolower(ext[1]) != 't')
			continue;
		if (tolower(ext[2]) != 't')
			continue;
		if (tolower(ext[3]) != 'f')
			continue;

		struct stat statbuf;
		stat(de->d_name, &statbuf);
		if (!S_ISREG(statbuf.st_mode))
			continue;

		char pathName[256];	// XXX: remove constant path size
		strcpy(pathName, ttdir);
		strcat(pathName, "/");
		strcat(pathName, de->d_name);

		TTFont *ttFont = new TTFont(de->d_name, 1);
		if (ttFont->badFont() || !ttFont) {
			delete ttFont;
			continue;
		}

		FontInfo fi;
		ttFont->getFontInfo(&fi);

		TTFNdata info;
		info.nameOfs = ftell(nameFile);
		info.nameLen = fi.faceLength;
		info.pathLen = strlen(pathName);

		fwrite((void *)fi.faceName, 1, info.nameLen, nameFile);
		fputc('\0', nameFile);
		fwrite((void *)pathName, 1, info.pathLen, nameFile);
		fputc('\0', nameFile);

		if (gslist)
			printf("(%s)\t(/%s/%s)\t;\n",
			       fi.faceName, fontdir, pathName);
		
		pathName[0] = '-';
		if (*ttdir == '.')
			strcpy(pathName + 1, "ttf");
		else
			strcpy(pathName + 1, ttdir);
		info.xlfdLen = ttFont->getXLFDbase(pathName);
		fwrite((void *)pathName, 1, info.xlfdLen, nameFile);
		fputc('\0', nameFile);

		info.charSet = 'U';
		info.slant = 0;
		info.bFamilyType = fi.panose[0];
		info.bSerifStyle = fi.panose[1];
		info.bWeight = fi.panose[2];
		info.bProportion = fi.panose[3];
		info.bContrast = fi.panose[4];
		info.bStrokeVariation = fi.panose[5];
		info.bArmStyle = fi.panose[6];
		info.bLetterForm = fi.panose[7];
		info.bMidLine = fi.panose[8];
		info.bXHeight = fi.panose[9];

		fwrite((void *)&info, 1, sizeof(info), infoFile);

		delete ttFont;
		++nfonts;
	}
	if (dirp)
		closedir(dirp);

	return nfonts;
}

static char *
cachefile(const char *leafname)
{
	if (chdir(cachedir)) {
		fprintf(stderr, _("xfstt: \"%s\" does not exist!\n"), cachedir);
		return 0;
	}

	long len = strlen(cachedir) + strlen(leafname) + 2;
	long maxlen = pathconf(cachedir, _PC_PATH_MAX);

	if (maxlen != -1 && len > maxlen) {
		fputs(_("xfstt: Cache directory name is too long\n"), stderr);
		return 0;
	}

	char *buf = new char [len];
	sprintf(buf, "%s/%s", cachedir, leafname);

	return buf;
}

static int
ttSyncAll(int gslist = 0)
{
	if (!gslist)
		debug("TrueType synching\n");

	if (chdir(fontdir)) {
		fprintf(stderr, _("xfstt: \"%s\" does not exist!\n"), fontdir);
		return -1;
	}
	DIR *dirp = opendir(".");

	char *ttinfofilename = cachefile(TTINFO_LEAF);
	if (!ttinfofilename)
		return -1;
	char *ttnamefilename = cachefile(TTNAME_LEAF);
	if (!ttnamefilename) {
		delete ttinfofilename;
		return -1;
	}

	FILE *infoFile = fopen(ttinfofilename, "wb");
	FILE *nameFile = fopen(ttnamefilename, "wb");
	delete ttinfofilename;
	delete ttnamefilename;
	
	if (infoFile <= 0 || nameFile <= 0) {
		fputs(_("xfstt: Cannot write to font database!\n"), stderr);
		return -1;
	}

	TTFNheader info;
	strncpy(info.magic, "TTFNINFO", 8);
	info.version = TTFN_VERSION;
	info.crc = 0;	// XXX
	fwrite((void *)&info, 1, sizeof(info), infoFile);
	strncpy(info.type, "NAME", 4);
	fwrite((void *)&info, 1, sizeof(info), nameFile);

	int nfonts = ttSyncDir(infoFile, nameFile, ".", gslist);
	while (dirent *de = readdir(dirp)) {
		chdir(fontdir);
		if (de->d_name[0] != '.' && !chdir(de->d_name))
			nfonts += ttSyncDir(infoFile, nameFile,
					    de->d_name, gslist);
	}

	fclose(infoFile);
	fclose(nameFile);

	if (nfonts > 0) {
		if (!gslist)
			printf(_("Found %d fonts.\n"), nfonts);
	} else {
		printf(_("No valid truetype fonts found!\n"));
		printf(_("Please put some *.ttf fonts into \"%s\"\n"), fontdir);
	}

	return nfonts;
}

// XXX: listXLFD is an ugly hack and needs a major cleanup
static int
listXLFDFonts(char *pattern0, int index, char *buf)
{
	static TTFNdata *ttfn = 0;
	static int mapIndex = 0;

	if (index == 0) {
		ttfn = (TTFNdata *)(infoBase + sizeof(TTFNheader));
		mapIndex = 0;
	} else if (mapIndex == 0 && (char *)++ttfn >= infoBase + infoSize)
		return -1;

	char *pattern = pattern0;

	char *xlfdName = nameBase + ttfn->nameOfs;
	xlfdName += ttfn->nameLen + ttfn->pathLen + 2;

	char proportion = (ttfn->bProportion == 9) ? 'm' : 'p';

	char *buf0 = buf++;
	if (pattern[0] == '*' // Thanks CK
	    || (pattern[0] == '-' && pattern[1] == '*' && pattern[2] == 0)) {
		char xlfdExt[] = "0-0-0-0-p-0-iso8859-1";
		xlfdExt[8] = proportion;
		strcpy(buf, xlfdName);
		strcpy(buf + ttfn->xlfdLen, xlfdExt);
		strcpy(buf + ttfn->xlfdLen + 12, encodings[mapIndex]->strName);
		if (!encodings[++mapIndex]) mapIndex = 0;
		*buf0 = strlen(buf);
		return *buf0 + 1;
	}

	int delim = 0;
	for (;;) {
		if (*pattern == '-')
			if (++delim >= 7)
				break;
		if (tolower(*pattern) == *xlfdName)
			*(buf++) = *(pattern++), ++xlfdName;
		else if (*(pattern++) == '*')
			while (*xlfdName && *xlfdName != '-')
				*(buf++) = *(xlfdName++);
		else
			return 0;
	}

	*(buf++) = '-';
	for (; delim <= 14 && *pattern; ++pattern) {
		char c = pattern[1];
		if (*pattern == '-')
			if (++delim == 12 && c == '*')
				c = proportion;
		*(buf++) = (c == '*') ? '0' : c;
	}

	*buf = 0;
	// XXX: this hack satisfies Mozilla, thanks Andrew Turner
	if (!strcmp(buf - 4, "-0-0")) {
		strcpy(buf - 3, encodings[mapIndex]->strName);
		buf += encodings[mapIndex]->lenName - 3;
		if (!encodings[++mapIndex])
			mapIndex = 0;
	} else
		mapIndex = 0;

	debug("match\t\"%s\"\n", buf0 + 1);

	*buf0 = buf - buf0;
	return *buf0 + 1;
}

static int
listTTFNFonts(char *pattern, int index, char *buf)
{
	static TTFNdata *ttfn = 0;
	static char *alias = 0;

	if (pattern[0] != '*' || pattern[1] != 0)
		return -1;

	if (index == 0 || ttfn == 0) {
		ttfn = (TTFNdata *)(infoBase + sizeof(TTFNheader));
		alias = aliasBase;
	} else if ((char *)++ttfn >= infoBase + infoSize)
		return -1;

#if 0
		int len = aliasSize - (alias - aliasBase);
		while (--len > 0 && *(alias++) != '\"');
		if (len <= 0)
			return -1;
		char* name = alias;
		while (--len > 0 && *(alias++) != '\"');
		*buf = alias - name - 1;
		strncpy(buf + 1, name, *(U8 *)buf);
		return *buf + 1;
	}
#endif

	char *fontName = nameBase + ttfn->nameOfs;

	TPFontName fn;
	sprintf(&fn.panose[0][0], "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		ttfn->bFamilyType, ttfn->bSerifStyle, ttfn->bWeight,
		ttfn->bProportion, ttfn->bContrast, ttfn->bStrokeVariation,
		ttfn->bArmStyle, ttfn->bLetterForm, ttfn->bMidLine,
		ttfn->bXHeight);

	fn.nameLen = sizeof(TPFontName) + ttfn->nameLen;
	fn.magic[0] = 'T';
	fn.magic[1] = 'T';
	fn.charset = ttfn->charSet;
	fn.modifier = '0';
	fn.panoseMagic = 'P';
	fn.underscore = '_';

	strncpy(buf, (char *)&fn, sizeof(fn));
	strncpy(buf + sizeof(fn), fontName, ttfn->nameLen);
	buf[fn.nameLen] = 0;
	debug("ListFont \"%s\"\n", buf);

	return fn.nameLen + 1;
}

static XFSFont *
findFont(Font fid, int sd, int seqno)
{
	XFSFont *xfs = xfsFont;
	for (int i = MAXOPENFONTS; --i >= 0; ++xfs)
		if (fid == xfs->fid)
			return xfs;

	debug("fid = %ld not found!\n", fid);

	if (sd) {
		fsError reply;
		reply.type = FS_Error;
		reply.request = FSBadFont;
		reply.sequenceNumber = seqno;
		reply.length = sizeof(reply) >> 2;
		write(sd, (void *)&reply, sizeof(reply));
	}

	return 0;
}

static XFSFont *
openFont(TTFont *ttFont, FontParams *fp, Rasterizer *raster,
	 int fid, Encoding *encoding)
{
	debug("point %d, pixel %d, res %d\n",
		fp->point[0], fp->pixel[0], fp->resolution[0]);

	if (!ttFont || ttFont->badFont())
		return 0;

	XFSFont *xfs = findFont(0, 0, 0);
	if (!xfs) {
		debug("Too many open fonts!\n");
		delete ttFont;
		return 0;
	}

	xfs->fid = fid;
	xfs->ttFont = ttFont;
	ttFont->getFontInfo(&xfs->fi);
	xfs->encoding = encoding;

	// XXX: char range hack in order to prevent XFree crashes
	FontInfo *fi = &xfs->fi;
	if (fi->lastChar > maxLastChar)
		fi->lastChar = maxLastChar;
	if (fi->lastChar > 255)
		fi->lastChar |= 255;
	if (maxLastChar > 255)
		fi->firstChar = 0;
	if (fi->firstChar > ' ')
		fi->firstChar = ' ' - 1;
	if (fi->firstChar > fi->lastChar)
		fi->firstChar = fi->lastChar;

	if (!fp->resolution[0] || !fp->resolution[1])
		fp->resolution[0] = fp->resolution[1]
				  = defaultres ? defaultres : VGARES;

	if (!fp->pixel[0] && !fp->pixel[1] && !fp->pixel[2] && !fp->pixel[3]) {
		fp->pixel[0] = (fp->point[0] * fp->resolution[0]) / 72;
		fp->pixel[1] = (fp->point[1] * fp->resolution[1]) / 72;
		fp->pixel[2] = (fp->point[2] * fp->resolution[0]) / 72;
		fp->pixel[3] = (fp->point[3] * fp->resolution[1]) / 72;
	}

	if (!fp->pixel[0] && !fp->pixel[1] && !fp->pixel[2] && !fp->pixel[3]) {
		fp->pixel[0] = fp->pixel[1] = 12;
		fp->pixel[2] = fp->pixel[3] = 0;
	}

	if (!fp->point[0] && !fp->point[1] && !fp->point[2] && !fp->point[3]) {
		fp->point[0] = (fp->pixel[0] * 72 + 36) / fp->resolution[0];
		fp->point[1] = (fp->pixel[1] * 72 + 36) / fp->resolution[1];
		fp->point[2] = (fp->pixel[2] * 72 + 36) / fp->resolution[0];
		fp->point[3] = (fp->pixel[3] * 72 + 36) / fp->resolution[1];
	}

	debug("point %d, pixel %d, res %d\n", fp->point[0], fp->pixel[0],
		 fp->resolution[0]);

	// init rasterizer
	raster->useTTFont(ttFont, fp->flags);
	raster->setPixelSize(
		fp->pixel[0], fp->pixel[2], fp->pixel[3], fp->pixel[1]);

	xfs->fe.buflen = MAXFONTBUFSIZE;
	while (!(xfs->fe.buffer = (U8 *)allocMem(xfs->fe.buflen)))
		if ((xfs->fe.buflen >>= 1) < MINFONTBUFSIZE) {
			fputs(_("xfstt: entering memory starved mode\n"),
			      stderr);
			xfs->fid = 0;
			return 0;
		}

	raster->getFontExtent(&xfs->fe);

	int used = (xfs->fe.bitmaps + xfs->fe.bmplen) - xfs->fe.buffer;
	int bmpoff = xfs->fe.bitmaps - xfs->fe.buffer;
	xfs->fe.buffer = (U8 *)shrinkMem(xfs->fe.buffer, used);
	if (xfs->fe.buffer) {
		xfs->fe.buflen = used;
		xfs->fe.bitmaps = xfs->fe.buffer + bmpoff;
	} else {
		xfs->fid = 0;	// XXX
		xfs = 0;
	}

	return xfs;
}

static XFSFont *
openTTFN(Rasterizer *raster, char *ttfnName, FontParams *fp, int fid)
{
	if (tolower(ttfnName[0]) != 't' || tolower(ttfnName[1]) != 't')
		return 0;

	// parse attributes
	debug("point %d, pixel %d, res %d\n",
		fp->point[0], fp->pixel[0], fp->resolution[0]);

	int m_index = 0, p_index = 0, r_index = 0;
	for (ttfnName += 2;;) {
		char c = *(ttfnName++);
		if (c == '\0')
			return 0;
		if (c == '_')
			break;
		int neg = 0;
		if (*ttfnName == '-') {
			++ttfnName;
			neg = -1;
		}
		int val = 0;
		while (isdigit(*ttfnName))
			val = 10 * val + *(ttfnName++) - '0';
		if (neg)
			val = -val;

		switch (tolower(c)) {
		case 'm':	// pointsize
			if (m_index < 4)
				fp->point[m_index++] = val;
			break;
		case 'p':	// pixelsize
			if (p_index < 4)
				fp->pixel[p_index++] = val;
			break;
		case 'r':	// resolution
			if (r_index < 2)
				fp->resolution[r_index++] = val;
			break;
		case 'f':	// flags
			fp->flags = val;
			break;
		default:
			return 0;
		}
	}

	// set attribute defaults

	switch (m_index) {
	case 0:	// use fp defaults		//fall through
	case 1:	fp->point[1] = fp->point[0];	//fall through
	case 2:	fp->point[2] = 0;		//fall through
	case 3:	fp->point[3] = -fp->point[2];	//fall through
	default: break;
	}

	switch (p_index) {
	case 0:	// use fp defaults		//fall through
	case 1:	fp->pixel[1] = fp->pixel[0];	//fall through
	case 2:	fp->pixel[2] = 0;		//fall through
	case 3:	fp->pixel[3] = -fp->pixel[2];	//fall through
	default: break;
	}

	switch (r_index) {
	case 0:	break; // use fp defaults
	case 1:	fp->resolution[1] = fp->resolution[0]; break;
	default: break;
	}

	TTFNdata *ttfn = (TTFNdata *)(infoBase + sizeof(TTFNheader));
	// XXX: linear search should be replaced
	for (; (char *)ttfn < infoBase + infoSize; ++ttfn) {
		char *name = nameBase + ttfn->nameOfs;
		char *file = name + ttfn->nameLen + 1;
		if (!strcmp(name, ttfnName)) {
			chdir(fontdir);
			return openFont(new TTFont(file), fp, raster, fid,
					encodings[0]);
		}
	}
	return 0;
}

static int
xatoi(char *p)
{
	int result = 0;
	for (char c = *p; isdigit(c); c = *++p)
		result = 10 * result + c - '0';
	return result;
}

static XFSFont *
openXLFD(Rasterizer *raster, char *xlfdName, FontParams *fp, int fid)
{
	if (xlfdName[0] != '-')
		return 0;

	int delim = 0;
	Encoding *encoding = 0;
	for (char *p = xlfdName; *p; ++p) {
		*p = tolower(*p);
		if (*p == '-')
			switch (++delim) {
			case  7:	// pixelsize
				fp->pixel[0] = fp->pixel[1] = xatoi(++p);
				*p = 0;
				break;
			case  8:	// pointsize
				fp->point[0] = fp->point[1] = xatoi(++p)/10;
				break;
			case  9:	// x-resolution
				fp->resolution[0] = xatoi(++p);
				break;
			case 10:	// y-resolution
				fp->resolution[1] = xatoi(++p);
				break;
			case 13:
				for (char* cp = p; *cp; ++cp)
					*cp = tolower(*cp);
				encoding = Encoding::find(++p);
				break;
			}
	}

	if (!encoding)
		encoding = encodings[0];

	debug("\nopenXLFD(\"%s\"), %s\n", xlfdName, encoding->strName);
	debug("size %d, resx %d, resy %d\n",
		 fp->point[0], fp->resolution[0], fp->resolution[1]);

	TTFNdata* ttfn = (TTFNdata *)(infoBase + sizeof(TTFNheader));
	// XXX: linear search should be replaced
	for (; (char *) ttfn < infoBase + infoSize; ++ttfn) {
		char *file = nameBase + ttfn->nameOfs + ttfn->nameLen + 1;
		char *xlfd = file + ttfn->pathLen + 1;
		char *p = xlfdName;
		for (; *p; ++p, ++xlfd) {
			if (*p == '*')
				for (++p; *xlfd != *p; ++xlfd);
			if (*p != *xlfd) break;
		}
		if (*p == 0 && *xlfd == 0) {
			chdir(fontdir);
			return openFont(new TTFont(file), fp, raster, fid,
					encoding);
		}
	}

	return 0;
}

// returns > 0 if ok
int
openTTFdb()
{
	infoSize = nameSize = aliasSize = 0;

	if (chdir(fontdir)) {
		fprintf(stderr, _("xfstt: \"%s\" does not exist!\n"), fontdir);
		return 0;
	}

	char *ttinfofilename = cachefile(TTINFO_LEAF);
	if (!ttinfofilename) return 0;

	int fd = open(ttinfofilename, O_RDONLY);
	if (0 >= fd) {
		fputs(_("Can not open font database!\n"), stderr);
		delete ttinfofilename;
		return 0;
	}

	struct stat statbuf;
	stat(ttinfofilename, &statbuf);
	infoSize = statbuf.st_size;
	infoBase = (char *)mmap(0L, infoSize, PROT_READ, MAP_SHARED, fd, 0L);
	close(fd);
	delete ttinfofilename;

	if (infoSize <= sizeof(TTFNheader)
	    || strncmp(infoBase, "TTFNINFO", 8)) {
		fputs(_("Corrupt font database!\n"), stderr);
		return 0;
	}

	if (((TTFNheader *)infoBase)->version != TTFN_VERSION) {
		fputs(_("Wrong font database version!\n"), stderr);
		return 0;
	}

	char *ttnamefilename = cachefile(TTNAME_LEAF);
	if (!ttnamefilename)
		return -1;

	fd = open(ttnamefilename, O_RDONLY);
	if (0 >= fd) {
		fputs(_("Can not open font database!\n"), stderr);
		delete ttnamefilename;
		return 0;
	}

	stat(ttnamefilename, &statbuf);
	nameSize = statbuf.st_size;
	nameBase = (char *)mmap(0L, nameSize, PROT_READ, MAP_SHARED, fd, 0L);
	close(fd);
	delete ttnamefilename;

	if (nameSize <= sizeof(TTFNheader)
	    || strncmp(nameBase, "TTFNNAME", 8)) {
		fputs(_("Corrupt font database!\n"), stderr);
		return 0;
	}

	if (((TTFNheader *)nameBase)->version != TTFN_VERSION) {
		fputs(_("Wrong font database version!\n"), stderr);
		return 0;
	}

	if (!stat("fonts.alias", &statbuf)) {
		aliasSize = statbuf.st_size;
		int fd = open("fonts.alias", O_RDONLY);
		if (fd <= 0)
			return 0;
		aliasBase = (char *)mmap(0L, aliasSize, PROT_READ, MAP_SHARED,
					 fd, 0L);
		close(fd);
	}

	return 1;
}

void
closeTTFdb()
{
	if (infoSize)
		munmap(infoBase, infoSize);
	if (nameSize)
		munmap(nameBase, nameSize);
	if (aliasSize)
		munmap(aliasBase, aliasSize);

	infoSize = nameSize = aliasSize = 0;
}

static int
prepare2connect(int portno)
{
	static struct sockaddr_un s_unix;
	static struct sockaddr_in s_inet;
	static int sd_unix = 0;
	static int sd_inet = 0;

	if (!sd_unix) {
		mode_t old_umask;

		// prepare unix connection
		sd_unix = socket(PF_UNIX, SOCK_STREAM, 0);

		s_unix.sun_family = AF_UNIX;
		sprintf(s_unix.sun_path, "%s/fs%d", sockdir, portno);
		sockname = s_unix.sun_path;
		mkdir(sockdir, 01777);
		unlink(s_unix.sun_path);
		old_umask = umask(0);
		if (bind(sd_unix, (struct sockaddr *)&s_unix, sizeof(s_unix))) {
			fprintf(stderr, _("Couldn't write to %s/\n"), sockdir);
			fputs(_("Please check permissions.\n"), stderr);
		}
		umask(old_umask);
		listen(sd_unix, 1);	// only one connection
	}

	if (!noTCP && !sd_inet) {
		// prepare inet connection
		sd_inet = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		s_inet.sin_family = AF_INET;
		s_inet.sin_port = htons(portno);
		s_inet.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(sd_inet, (struct sockaddr *)&s_inet, sizeof(s_inet))) {
			fprintf(stderr, _("Cannot open TCPIP port %d\n"), portno);
			fprintf(stderr, _("Better try another port!\n"));
		}
		listen(sd_inet, 1);	// only one connection
	}

	fd_set sdlist;
	FD_ZERO(&sdlist);
	FD_SET(sd_unix, &sdlist);
	if (!noTCP)
		FD_SET(sd_inet, &sdlist);
	int maxsd = (sd_inet > sd_unix) ? sd_inet : sd_unix;
	select(maxsd + 1, &sdlist, 0L, 0L, 0L);

	int sd = 0;
	unsigned int saLength = sizeof(struct sockaddr);
	if (FD_ISSET(sd_unix, &sdlist))
		sd = accept(sd_unix, (struct sockaddr *)&s_unix, &saLength);
	else if (!noTCP && FD_ISSET(sd_inet, &sdlist))
		sd = accept(sd_inet, (struct sockaddr *)&s_inet, &saLength);
	debug("accept(saLength = %d) = %d\n", saLength, sd);

	return sd;
}

#define MAXREQSIZE 4096
U8 buf[MAXREQSIZE + 256];

static int
connecting(int sd)
{
	int i = read(sd, buf, MAXREQSIZE);	// read fsConnClientPrefix

	fsConnClientPrefix *req = (fsConnClientPrefix *)buf;
	if (i < (int)sizeof(fsConnClientPrefix))
		return 0;

	debug("Connecting\n");
	debug("%s endian connection\n",
		 (req->byteOrder == 'l') ? "little" : "big");
	debug("version %d.%d\n", req->major_version, req->minor_version);

	if ((req->byteOrder == 'l' && (*(U32 *)req & 0xff) != 'l')
	    || (req->byteOrder == 'B' && ((*(U32 *)req >> 24) & 0xff) != 'B')) {
		fputs(_("xfstt: byteorder mismatch, giving up\n"), stderr);
		return 0;
	}

	fsConnSetup replySetup;
	replySetup.status = 0;
	replySetup.major_version = 2;
	replySetup.minor_version = 0;
	replySetup.num_alternates = 0;
	replySetup.auth_index = 0;
	replySetup.alternate_len = 0;
	replySetup.auth_len = 0;

	write(sd, (void *)&replySetup, sizeof(replySetup));

	struct {
		fsConnSetupAccept s1;
		char vendor[2], pad[2];
	} replyAccept;

	replyAccept.s1.length = (sizeof(replyAccept) + 3) >> 2;
	replyAccept.s1.max_request_len = MAXREQSIZE >> 2;
	replyAccept.s1.vendor_len = sizeof(replyAccept.vendor);
	replyAccept.s1.release_number = 1;
	replyAccept.vendor[0] = 'H';
	replyAccept.vendor[1] = 'D';

	write(sd, (void *)&replyAccept, sizeof(replyAccept));

	return 1;
}

static void
fixup_bitmap(FontExtent *fe, U32 hint)
{
	int format = ((hint >> 8) & 3) + 3;
	if (format < LOGSLP) {
		fputs(_("xfstt: scanline length error!\n"), stderr);
		fprintf(stderr,
			_("recompile xfstt with LOGSLP defined as %d!\n"),
			format <= 3 ? 3 : format);
	}

	if ((hint ^ fe->bmpFormat) == 0)
		return;

	register U8 *p, *end = fe->bitmaps + fe->bmplen;
	if ((fe->bmpFormat ^ hint) & BitmapFormatMaskByte) {
		debug("slpswap SLP=%d\n", LOGSLP);
		p = fe->bitmaps;
		switch (LOGSLP) {
		case 3:
			break;
		case 4:
			for (; p < end; p += 2)
				*(U16 *)p = bswaps(*(U16 *)p);
			break;
		case 5:
			for (; p < end; p += 4)
				*(U32 *)p = bswapl(*(U32 *)p);
			break;
		case 6:
			for (; p < end; p += 8) {
				U32 tmp = *(U32 *)p;
				*(U32 *)(p + 0) = bswapl(*(U32 *)(p + 4));
				*(U32 *)(p + 4) = bswapl(tmp);
			}
			break;
		}
	}

	if ((fe->bmpFormat ^ hint) & BitmapFormatMaskBit) {
		debug("bitswap\n");
		U8 map[16] = {0, 8, 4, 12, 2, 10, 6, 14,
			      1, 9, 5, 13, 3, 11, 7, 15};
		for (p = fe->bitmaps; p < end; ++p)
			*p = (map[*p & 15] << 4) | map[(*p >> 4) & 15];
	}

	if ((format != LOGSLP) && (hint & BitmapFormatByteOrderMask == 0)) {
		debug("fmtswap SLP=%d -> fmt=%d\n", LOGSLP, format);
		p = fe->bitmaps;
		if (LOGSLP == 3 && format == 4) {
			for (; p < end; p += 2)
				*(U16 *)p = bswaps(*(U16 *)p);
		} else if (LOGSLP == 3 && format == 5) {
			for (; p < end; p += 4)
				*(U32 *)p = bswapl(*(U32 *)p);
		} else if (LOGSLP == 3 && format == 6) {
			for (; p < end; p += 8) {
				U32 tmp = *(U32 *)p;
				*(U32 *)(p + 0) = bswapl(*(U32 *)(p + 4));
				*(U32 *)(p + 4) = bswapl(tmp);
			}
		} else if (LOGSLP == 4 && format == 5) {
			for (; p < end; p += 4) {
				U16 tmp = *(U16 *)p;
				*(U16 *)(p + 0) = *(U16 *)(p + 2);
				*(U16 *)(p + 2) = tmp;
			}
		} else if (LOGSLP == 5 && format == 6) {
			for (; p < end; p += 8) {
				U32 tmp = *(U32 *)p;
				*(U32 *)(p + 0) = *(U32 *)(p + 4);
				*(U32 *)(p + 4) = tmp;
			}
		} else { // (LOGSLP == 4 && format == 6)
			for (; p < end; p += 8) {
				U16 tmp = *(U16 *)p;
				*(U16 *)(p + 0) = *(U16 *)(p + 6);
				*(U16 *)(p + 6) = tmp;
				tmp = *(U16 *)(p + 2);
				*(U16 *)(p + 2) = *(U16 *)(p + 4);
				*(U16 *)(p + 4) = tmp;
			}
		}
	}

	fe->bmpFormat = hint;
}

static int
working(int sd, Rasterizer *raster, char *replybuf)
{
	FontParams fp0 = {{0, 0, 0, 0}, {0, 0, 0, 0}, {VGARES, VGARES}, 0}, fp;
	int event_mask = 0;
	int i;

	if (defaultres)
		fp0.resolution[0] = fp.resolution[1] = defaultres;

	for (int seqno = 1;; ++seqno) {
		int l = read(sd, buf, sz_fsReq);
		if (l < sz_fsReq)
			return l;

#ifdef DEBUG
		printf("===STARTREQ=========== %d\n", l);
		for (i = 0; i < sz_fsReq; ++i)
			printf("%02X ", buf[i]);
		printf("\n");
		sync();
#endif

		int length = ((fsReq *)buf)->length << 2;
		if (length > MAXREQSIZE) {
			debug("request too big: %d bytes\n", length);

			fsError reply;
			reply.type = FS_Error;
			reply.request = FSBadLength;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;

			write(sd, (void *)&reply, sizeof(reply));
			break;
		}

		for (; l < length; l += i) {
			i = read(sd, buf + l, length-l);
			if (i <= 0)
				return i;
		}

#ifdef DEBUG
		for (i = sz_fsReq; i < length; ++i) {
			printf("%02X ", buf[i]);
			if ((i & 3) == 3)
				printf(" ");
			if ((i & 15) == (15 - sz_fsReq))
				printf("\n");
		}
		printf("\n===ENDREQ============= %d\n", length);
		sync();
#endif

		switch (buf[0]) {	// request type
		case FS_Noop:
			debug("FS_Noop\n");
			break;

		case FS_ListExtensions:
			debug("FS_ListExtensions\n");
			{
			fsListExtensionsReply reply;
			reply.type = FS_Reply;
			reply.nExtensions = 0;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_QueryExtension:
			debug("FS_QueryExtension\n");
			{
			fsQueryExtensionReply reply;
			reply.type = FS_Reply;
			reply.present = 0;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;
			reply.major_version = 0;
			reply.minor_version = 0;
			reply.major_opcode = 0;
			reply.first_event = 0;
			reply.num_events = 0;
			reply.first_error = 0;
			reply.num_errors = 0;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_ListCatalogues:
			debug("FS_ListCatalogues\n");
			{
			fsListCataloguesReply reply;
			reply.type = FS_Reply;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;
			reply.num_replies = 0;
			reply.num_catalogues = 0;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_SetCatalogues:	// resync font database
			debug("FS_SetCatalogues\n");
			{
			closeTTFdb();
			ttSyncAll();
			openTTFdb();
			}
			break;

		case FS_GetCatalogues:
			debug("FS_GetCatalogues\n");
			{
			fsGetCataloguesReply reply;
			reply.type = FS_Reply;
			reply.num_catalogues = 0;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_SetEventMask:
			{
			fsSetEventMaskReq *req = (fsSetEventMaskReq *)buf;
			event_mask = req->event_mask;
			debug("FS_SetEventMask %04X\n", event_mask);
			}
			break;

		case FS_GetEventMask:
			debug("FS_GetEventMask = %04X\n", event_mask);
			{
			fsGetEventMaskReply reply;
			reply.type = FS_Reply;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;
			reply.event_mask = event_mask;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_CreateAC:		// don't care
			debug("FS_CreateAC\n");
			{
			fsCreateACReply reply;
			reply.type = FS_Reply;
			reply.auth_index = 0;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;
			reply.status = AuthSuccess;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_FreeAC:			// don't care
			debug("FS_FreeAC\n");
			{
			fsGenericReply reply;
			reply.type = FS_Reply;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_SetAuthorization:	// don't care
			debug("FS_SetAuthorization\n");
			break;

		case FS_SetResolution:
			{
			fsSetResolutionReq *req = (fsSetResolutionReq *)buf;
			int numres = req->num_resolutions;	// XXX 1
			fsResolution *res = (fsResolution *)(req + 1);

			debug("FS_SetResolution * %d\n", numres);
			for (; --numres >= 0; ++res) {
				if (!defaultres) {
					fp0.resolution[0] = res->x_resolution;
					fp0.resolution[1] = res->y_resolution;
				}
				res->point_size /= 10;
				fp0.point[0] = fp0.point[1] = res->point_size;
				fp0.point[2] = fp0.point[3] = 0;
				debug("xres = %d, yres = %d, size = %d\n",
					 res->x_resolution, res->y_resolution,
					 res->point_size / 10);
			}
			}
			break;

		case FS_GetResolution:
			debug("FS_GetResolution\n");
			{
			struct {
				fsGetResolutionReply s1;
				fsResolution s2;
				char pad[2];
			} reply;

			reply.s1.type = FS_Reply;
			reply.s1.num_resolutions = 1;
			reply.s1.sequenceNumber = seqno;
			reply.s1.length = sizeof(reply) >> 2;
			reply.s2.x_resolution = fp0.resolution[0];
			reply.s2.y_resolution = fp0.resolution[1];
			reply.s2.point_size = fp0.point[0] * 10;

			write(sd, (void *)&reply, sizeof(reply));
			}

			break;

		case FS_ListFonts:
			{
			fsListFontsReq* req = (fsListFontsReq *)buf;
			char *pattern = (char *)(req + 1);

			pattern[req->nbytes] = 0;
			debug("FS_ListFonts \"%s\" * %ld\n",
				 pattern, req->maxNames);

			fsListFontsReply reply;
			reply.type = FS_Reply;
			reply.sequenceNumber = seqno;
			// XXX: XFree doesn't handle split up replies yet
			reply.following = 0;
			reply.nFonts = 0;

			char *buf = replybuf;
			char *endbuf = replybuf + MAXREPLYSIZE - 256;
			for (i = 0; reply.nFonts < req->maxNames; i = 1) {
				if (buf >= endbuf)
					break;
				int len = listXLFDFonts(pattern, i, buf);
				if (len == 0)
					continue;
				if (len < 0)
					break;
				buf += len;
				++reply.nFonts;
			}
			for (i = 0; reply.nFonts < req->maxNames; i = 1) {
				if (buf >= endbuf)
					break;
				int len = listTTFNFonts(pattern, i, buf);
				if (len == 0)
					continue;
				if (len < 0)
					break;
				buf += len;
				++reply.nFonts;
			}
			debug("Found %ld fonts\n", reply.nFonts);
			reply.length = (sizeof(reply) + (buf - replybuf)
				       + 3) >> 2;

			write(sd, (void *)&reply, sizeof(reply));
			write(sd, (void *)replybuf,
			      (reply.length << 2) - sizeof(reply));
			}
			break;

		case FS_ListFontsWithXInfo:
			/* standard non-scalable X Fonts get XInfo really cheap,
			 * but it means a LOT of work for scalable hinted fonts.
			 * The high cost is multiplied by the need to go through
			 * different sizes and resolutions.
			 */
			debug("FS_ListFontsWithXInfo\n");
			{
// XFSTT_X_COMPLIANT
#if 0
			fsListFontsWithXInfoReply reply;
			reply.type = FS_Reply;
			reply.nameLength = 0;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;
			reply.nReplies = 0;
			// XXX: write(sd, (void *)&reply, sizeof(reply));
			write(sd, (void *)&reply, sizeof(fsGenericReply));
#else
			fsImplementationError error;
			error.type = FS_Error;
			error.request = FSBadImplementation;
			error.sequenceNumber = seqno;
			error.length = sizeof(error) >> 2;

			debug(" fsError size = %u bytes, %u ints\n",
				 sizeof(error), error.length);
			write(sd, (void *)&error, sizeof(error));
#endif
			}
			break;

		case FS_OpenBitmapFont:
			{
			fsOpenBitmapFontReq *req = (fsOpenBitmapFontReq *)buf;
			char *fontName = (char *)(req + 1) + 1;
			fontName[*(U8 *)(req + 1)] = 0;
			debug("FS_OpenBitmapFont \"%s\"", fontName);

			raster->format = (req->format_hint >> 8) & 3;
			if (req->format_hint & 0x0c)
				raster->format = ~raster->format;

			fp = fp0;
			if (openTTFN(raster, fontName, &fp, req->fid)
			    || openXLFD(raster, fontName, &fp, req->fid)) {
				fsOpenBitmapFontReply reply;
				reply.type = FS_Reply;
				reply.otherid_valid = fsFalse;
				reply.sequenceNumber = seqno;
				reply.length = sizeof(reply) >> 2;
				reply.otherid = 0;
				reply.cachable = fsTrue;

				write(sd, (void *)&reply, sizeof(reply));
				debug(" opened\n");
			} else {
				fsError reply;
				reply.type = FS_Error;
				reply.request = FSBadName;
				reply.sequenceNumber = seqno;
				reply.length = sizeof(reply) >> 2;

				write(sd, (void *)&reply, sizeof(reply));
				debug(" not found\n");
			}
			debug("fhint = %04lX, fmask = %04lX, fid = %ld\n",
				 req->format_hint, req->format_mask, req->fid);
			}
			break;

		case FS_QueryXInfo:
			{
			fsQueryXInfoReq *req = (fsQueryXInfoReq *)buf;
			debug("FS_QueryXInfo fid = %ld\n", req->id);

			struct {
				fsQueryXInfoReply s1;
				fsPropInfo s2;
				fsPropOffset s3;
				U32 dummyName, dummyValue;
			} reply;

			reply.s1.type = FS_Reply;
			reply.s1.sequenceNumber = seqno;
			reply.s1.length = sizeof(reply) >> 2;
			reply.s1.font_header_flags = FontInfoHorizontalOverlap
						     | FontInfoInkInside;

			XFSFont *xfs = findFont(req->id, sd, seqno);
			if (!xfs)
				break;
			FontInfo *fi = &xfs->fi;
			FontExtent *fe = &xfs->fe;

			reply.s1.font_hdr_char_range_min_char_high
				= reply.s1.font_header_default_char_high
				= (U8)(fi->firstChar >> 8);
			reply.s1.font_hdr_char_range_min_char_low
				= reply.s1.font_header_default_char_low
				= (U8)fi->firstChar;
			reply.s1.font_hdr_char_range_max_char_high
				= (U8)(fi->lastChar >> 8);
			reply.s1.font_hdr_char_range_max_char_low
				= (U8)fi->lastChar;

			debug("minchar = 0x%02X%02X, ",
				 reply.s1.font_hdr_char_range_min_char_high,
				 reply.s1.font_hdr_char_range_min_char_low);
			debug("maxchar = 0x%02X%02X\n",
				 reply.s1.font_hdr_char_range_max_char_high,
				 reply.s1.font_hdr_char_range_max_char_low);

			reply.s1.font_header_draw_direction = LeftToRightDrawDirection;
			// XXX: reply.s1.font_header_default_char_high = 0;
			// XXX: reply.s1.font_header_default_char_low = ' ';

			reply.s1.font_header_min_bounds_left = fe->xLeftMin;
			reply.s1.font_header_min_bounds_right = fe->xRightMin;
			reply.s1.font_header_min_bounds_width = fe->xAdvanceMin;
			reply.s1.font_header_min_bounds_ascent = fe->yAscentMin;
			reply.s1.font_header_min_bounds_descent = fe->yDescentMin;
			reply.s1.font_header_min_bounds_attributes = 0;
			reply.s1.font_header_max_bounds_left = fe->xLeftMax;
			reply.s1.font_header_max_bounds_right = fe->xRightMax;
			reply.s1.font_header_max_bounds_width = fe->xAdvanceMax;

			reply.s1.font_header_max_bounds_ascent = fe->yAscentMax;
			reply.s1.font_header_max_bounds_descent = fe->yDescentMax;
			reply.s1.font_header_max_bounds_attributes = 0;
			reply.s1.font_header_font_ascent = fe->yWinAscent;
			reply.s1.font_header_font_descent = fe->yWinDescent;

			debug("FM= (asc= %d, dsc= %d, ",
				 fe->yAscentMax, fe->yDescentMax);
			debug("wasc= %d, wdsc= %d, ",
				 fe->yWinAscent, fe->yWinDescent);
			debug("wmin= %d, wmax= %d)\n",
				 fe->xAdvanceMin, fe->xAdvanceMax);

			// we need to have some property data, otherwise
			// the X server complains
			reply.s2.num_offsets = 1;
			reply.s2.data_len = 8;
			reply.s3.name.position = 0;
			reply.s3.name.length = reply.s3.value.position = 4;
			reply.s3.value.length = 4;
			reply.s3.type = 0; // XXX: ???

			reply.dummyName = htonl(0x464F4E54);
			reply.dummyValue = htonl(0x54544678);

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;

		case FS_QueryXExtents8:
			// convert to QueryXExtents16 request
			{
			fsQueryXExtents8Req *req = (fsQueryXExtents8Req *)buf;
			U8 *p8 = (U8 *)(req + 1);
			U16 *p16 = (U16 *)p8;
			for (i = req->num_ranges; --i >= 0;)
				p16[i] = htons(p8[i]);
			}
			// fall through
		case FS_QueryXExtents16:
			{
			fsQueryXExtents16Req *req = (fsQueryXExtents16Req *)buf;
			debug("FS_QueryXExtents%s fid = %ld, ",
				(buf[0] == FS_QueryXExtents16 ? "16" : "8"),
				req->fid);
			debug("range=%d, nranges=%ld\n",
				req->range, req->num_ranges);

			XFSFont *xfs = findFont(req->fid, sd, seqno);
			if (!xfs)
				break;

			fsXCharInfo *ext0 = (fsXCharInfo *)replybuf, *ext = ext0;
			U16 *ptr = (U16 *)(req + 1);
			int nranges = req->num_ranges;
			if (req->range) {
				ptr[nranges] = htons(xfs->fi.lastChar);
				if (!nranges) {
					nranges = 2;
					ptr[1] = ptr[0];
					ptr[0] = htons(xfs->fi.firstChar);
				}
				for (; nranges > 0; nranges -= 2, ptr += 2) {
					ptr[0] = ntohs(ptr[0]);
					ptr[1] = ntohs(ptr[1]);
					debug("rg %d..%d\n",ptr[0],ptr[1]);
					for (U16 j = ptr[0]; j <= ptr[1]; ++j)
						(ext++)->left = j;
				}
			} else
				while (--nranges >= 0)
					(ext++)->left = ntohs(*(ptr++));

			fsQueryXExtents16Reply reply;
			reply.type = FS_Reply;
			reply.sequenceNumber = seqno;
			reply.num_extents = ext - ext0;
			reply.length = (sizeof(reply) + 3
				       + ((U8 *)ext - (U8 *)ext0)) >> 2;

			CharInfo *ci = (CharInfo *)xfs->fe.buffer;

			ext = ext0;
			for (i = reply.num_extents; --i >= 0; ++ext) {
				int ch = ext->left;
				ch = xfs->encoding->map2unicode(ch);
				int glyphNo = xfs->ttFont->getGlyphNo16(ch);
				GlyphMetrics *gm = &ci[glyphNo].gm;
				
				ext->left = -gm->xOrigin;
				ext->right = gm->xBlackbox - gm->xOrigin;
				ext->width = gm->xAdvance;
				ext->ascent = gm->yBlackbox - gm->yOrigin;
				ext->descent = gm->yOrigin;
				ext->attributes = gm->yAdvance;

				// Thanks GB
				if (!glyphNo && ch != xfs->fi.firstChar) {
					ext->left = ext->right = 0;
					ext->ascent = ext->descent = 0;
					ext->width = ext->attributes = 0;
				}

#if DEBUG & 2
				debug("GM[%3d = %3d] = ", ch, glyphNo);
				debug("(l= %d, r= %d, ",
					 ext->left, ext->right);
				debug("w= %d, a= %d, d= %d);\n",
					 ext->width, ext->ascent, ext->descent);
#endif
			}
			write(sd, (void *)&reply, sizeof(reply));
			write(sd, (void *)ext0, (U8 *)ext - (U8 *)ext0);
			}
			break;

		case FS_QueryXBitmaps8:
			// convert to QueryXBitmaps16 request
			{
			fsQueryXBitmaps8Req *req = (fsQueryXBitmaps8Req *)buf;
			U8 *p8 = (U8 *)(req + 1);
			U16 *p16 = (U16 *)p8;
			for (i = req->num_ranges; --i >= 0;)
				p16[i] = ntohs(p8[i]);
			}
			// fall through
		case FS_QueryXBitmaps16:
			{
			fsQueryXBitmaps16Req *req = (fsQueryXBitmaps16Req *)buf;
			debug("FS_QueryXBitmaps16 fid = %ld, fmt = %04lX\n",
				 req->fid, req->format);
			debug("range=%d, nranges=%ld\n",
				 req->range, req->num_ranges);

			XFSFont *xfs = findFont(req->fid, sd, seqno);
			if (!xfs)
				break;

			fixup_bitmap(&xfs->fe, req->format);

			fsOffset32 *ofs0 = (fsOffset32 *)replybuf, *ofs = ofs0;
			U16 *ptr = (U16 *)(req + 1);
			int nranges = req->num_ranges;
			if (req->range) {
				ptr[nranges] = htons(xfs->fi.lastChar);
				if (!nranges) {
					nranges = 2;
					ptr[1] = ptr[0];
					ptr[0] = htons(xfs->fi.firstChar);
				}
				for (; nranges > 0; nranges -= 2, ptr += 2) {
					ptr[0] = ntohs(ptr[0]);
					ptr[1] = ntohs(ptr[1]);
					debug("rg %d..%d\n",ptr[0],ptr[1]);
					for (U16 j = ptr[0]; j <= ptr[1]; ++j)
						(ofs++)->position = j;
				}
			} else
				while (--nranges >= 0)
					(ofs++)->position = ntohs(*(ptr++));

			fsQueryXBitmaps16Reply reply;
			reply.type = FS_Reply;
			reply.sequenceNumber = seqno;
			reply.num_chars = ofs - ofs0;
			reply.nbytes = xfs->fe.bmplen;
			reply.replies_hint = 0;

			CharInfo *cia = (CharInfo *)xfs->fe.buffer;
			for (i = xfs->fe.numGlyphs; --i >= 0; ++cia)
				cia->tmpofs = -1;
			cia = (CharInfo *)xfs->fe.buffer;

			char *bmp0 = (char *)ofs, *bmp = bmp0;
			ofs = ofs0;
			char *replylimit = replybuf + MAXREPLYSIZE;
			for (i = reply.num_chars; --i >= 0; ++ofs) {
				int ch = ofs->position;
				ch = xfs->encoding->map2unicode(ch);
				int glyphNo = xfs->ttFont->getGlyphNo16(ch);
				CharInfo *ci = &cia[glyphNo];

				ofs->length = ci->length;
				if (ci->tmpofs < 0) {
					if (bmp + ci->length < replylimit) {
						U8 *src = xfs->fe.bitmaps;
						src += ci->offset;
						memcpy(bmp, src, ci->length);
						ci->tmpofs = bmp - bmp0;
						bmp += ci->length;
					} else {
						ci->tmpofs = 0;
						ofs->length = 0;
					}
				}
				ofs->position = ci->tmpofs;

#if DEBUG & 2
				debug("OFS[%3d = %3d] = %ld\n",
					 ch, glyphNo, ofs->position);
#endif
			}
			reply.nbytes = bmp - bmp0;
#if 1
			reply.length = (sizeof(reply) + reply.nbytes + 3
				       + ((U8 *)ofs - (U8 *)ofs0)) >> 2;
			write(sd, (void *)&reply, sizeof(reply));
			write(sd, (void *)ofs0, (U8 *)ofs - (U8 *)ofs0);
			write(sd, (void *)bmp0, (reply.nbytes + 3) & ~3);
#else
{
			int nbytes = reply.nbytes;
			reply.nbytes = 0;
			reply.replies_hint = 1;
			reply.length = (sizeof(reply)
				       + ((U8 *)ofs - (U8 *)ofs0)) >> 2;
			write(sd, (void *)&reply, sizeof(reply));
			write(sd, (void *)ofs0, (U8 *)ofs - (U8 *)ofs0);

			reply.nbytes = nbytes;
			reply.replies_hint = 0;
			reply.sequenceNumber = ++seqno;
			reply.length = (sizeof(reply) + (bmp - bmp0)) >> 2;
			write(sd, (void *)&reply, sizeof(reply));
			write(sd, (void *)bmp0, (reply.nbytes + 3) & ~3);
}
#endif
			}
			break;

		case FS_CloseFont:
			{
			fsCloseReq *req = (fsCloseReq *)buf;
			debug("FS_CloseFont fid = %ld\n", req->id);

			XFSFont *xfs = findFont(req->id, sd, seqno);
			if (xfs) {
				deallocMem(xfs->fe.buffer, xfs->fe.buflen);
				delete xfs->ttFont;
				xfs->fid = 0;
			}
			}
			break;

		default:
			debug("Unknown FS request 0x%02X !\n", buf[0]);
			{
			fsRequestError reply;
			reply.type = FS_Error;
			reply.request = FSBadRequest;
			reply.sequenceNumber = seqno;
			reply.length = sizeof(reply) >> 2;
			reply.timestamp = 0;
			reply.major_opcode = buf[0];
			reply.minor_opcode = buf[1];

			write(sd, (void *)&reply, sizeof(reply));
			}
			break;
		}
	debug("done.\n");
	}

	return 0;
}

/* This is a cheesy little signal handler to make sure that the
 * pid file is properly disposed of when we are killed
 * possibly a better (more robust) signal handler could be written - sjc
 */
void
delPIDfile(int signal)
{
	unlink(pidfilename);
	if (sockname) {
		unlink(sockname);
		rmdir(sockdir);
	}
	exit(0);
}

void
setuidgid(char *name)
{
	struct passwd *pwent;

	setpwent();
	while ((pwent = getpwent()))
		if (strcmp(pwent->pw_name, name) == 0) {
			newuid = pwent->pw_uid;
			newgid = pwent->pw_gid;
		}
	endpwent();
}

int
main(int argc, char **argv)
{
	int multiConnection = 1;
	int inetdConnection = 0;
	int portno = default_port;
	int gslist = 0;
	int sync_db = 0;
	int daemon = 0;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	Encoding::getDefault(encodings, MAXENC);

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--gslist")) {
			gslist = sync_db = 1;
		} else if (!strcmp(argv[i], "--sync")) {
			sync_db = 1;
		} else if (!strcmp(argv[i], "--port")) {
			if (i <= argc)
				portno = xatoi(argv[++i]);
			if (!portno) {
				fputs(_("Illegal port number!\n"), stderr);
				portno = default_port;
			}
		} else if (!strcmp(argv[i], "--notcp")) {
			noTCP = 1;
		} else if (!strcmp(argv[i], "--res")) {
			if (i <= argc)
				defaultres = xatoi(argv[++i]);
			if (!defaultres)
				fputs(_("Illegal default resolution!\n"), stderr);
		} else if (!strcmp(argv[i], "--dir")) {
			fontdir = argv[++i];
		} else if (!strcmp(argv[i], "--user")) {
			setuidgid(argv[++i]);
		} else if (!strcmp(argv[i], "--pidfile")) {
			pidfilename = argv[++i];
		} else if (!strcmp(argv[i], "--cache")) {
			cachedir = argv[++i];
		} else if (!strcmp(argv[i], "--encoding")) {
			char *maplist = argv[++i];
			if (!Encoding::parse(maplist, encodings, MAXENC)) {
				fprintf(stderr, _("Illegal encoding!\n"));
				fprintf(stderr, _("Valid encodings are:\n"));
				for (Encoding *maps = 0;;) {
					maps = Encoding::enumerate(maps);
					if (!maps)
						exit(0);
					fprintf(stderr, "\t%s\n", maps->strName);
				}
			}
		} else if (!strcmp(argv[i], "--help")) {
			usage(1);
			return 0;
		} else if (!strcmp(argv[i], "--inetd")) { // thanks Feanor
			inetdConnection = 1;
		} else if (!strcmp(argv[i], "--multi")) {
			multiConnection = 1;
		} else if (!strcmp(argv[i], "--once")) {
			multiConnection = 0;
		} else if (!strcmp(argv[i], "--unstrap")) {
			maxLastChar = UNSTRAPLIMIT;
			printf(_("xfstt unstrapped: you must start X11 "
				"with \"-deferglyphs 16\" option!\n"));
		} else if (!strcmp(argv[i], "--daemon")) {
			daemon = 1;
		} else {
			usage(0);
			return -1;
		}
	}

	if (sync_db) {
		if (ttSyncAll(gslist) <= 0)
			fputs(_("xfstt: sync failed\n"), stderr);
		cleanupMem();
		return 0;
	}

	if (inetdConnection && multiConnection) {
		multiConnection = 0;
		fprintf(stderr,
			_("xfstt: --inetd and --multi option collission\n"));
		// we don't know what to do ...so exit.
		return 1;
	}

	if (daemon) {
		if (fork())
			_exit(0);
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		setsid();
		if (fork())
			_exit(0);
	}

	if (newuid == (uid_t)(-2)) {
		newuid = getuid();
		newgid = getgid();
	}

	// Make a pid file for easy starting and killing like
	// a good little daemon
	if (multiConnection) {
		
		FILE *pidfile = fopen(pidfilename, "w");
		if (pidfile) {
			pid_t pid = getpid();
			fprintf(pidfile, "%d\n", pid);
			fclose(pidfile);
			// setup signal handlers to die better
			signal(SIGINT, delPIDfile);
			signal(SIGTERM, delPIDfile);
		}
	}

	int retry;
	for (retry = 1; retry > 0; --retry) {
		if (openTTFdb() > 0)
			break;
		closeTTFdb();

		fprintf(stderr, _("xfstt: error opening TTF database!\n"));
		fprintf(stderr, _("xfstt: reading \"%s\" to build it.\n"),
			fontdir);

		if (ttSyncAll() > 0 && openTTFdb() > 0)
			break;
		fputs(_("Creating a font database failed\n"), stderr);
		unlink(pidfilename);
	}

	signal(SIGCHLD, SIG_IGN); // We don't need no stinkinig zombies -sjc

	if (retry <= 0)
		fputs(_("xfstt: Good bye.\n"), stderr);
	else do {
		int sd = inetdConnection ? 0 : prepare2connect(portno);

		if (connecting(sd)) {
			if (!multiConnection || !fork()) {
				setuid(newuid);
				setgid(newgid);

				Rasterizer *raster = new Rasterizer();
				char *replybuf = (char *)allocMem(MAXREPLYSIZE);

				working(sd, raster, replybuf);
				deallocMem(replybuf, MAXREPLYSIZE);
				delete raster;

				if (!inetdConnection)
					shutdown(sd, 2);
				close(sd);

				debug("xfstt: closing a connection\n");
				cleanupMem();

				return 0;
			} else if (multiConnection) {
				// Redundant on most systems
				// Needed for BSD Thanks David Lowe
				int status;

				waitpid(-1, &status, WNOHANG);
			}
		}
		close(sd);
	} while (multiConnection);

	if (sockname) {
		unlink(sockname);
		rmdir(sockdir);
	}

	debug("xfstt: server down\n");
	cleanupMem();

	return 0;
}

