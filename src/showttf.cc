/*
 * Show glyphs
 *
 * Copyright (C) 1997-1998 Herbert Duerr
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
 * License along with this library; if not, write to the Free Softaware
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define STARTGLYF	6
#define DEFAULT_FONT	"times.ttf"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <math.h>

#include "ttf.h"

#if (MSB_BYTE_FIRST != 1 || MSB_BIT_FIRST != 2)
	#error "showttf assumes MSB_BYTE_FIRST=1 and MSB_BIT_FIRST=2"
#endif

static Display *display;
static Visual *visual;
static int screen;
static int depth;
static Colormap colorMap;
static Window rootWindow;

static GC textGC;
static int color[5];
static int yellow;
static int black;
static int white;

static Rasterizer *raster;
static u8_t *pixmap;

#define BMPSIZE 1048*2048
u8_t *bitmap = 0;

#define DEFAULT_WIDTH	200
#define DEFAULT_HEIGHT	220

#ifndef MAGNIFY
int MAGNIFY = 1;
#endif /*MAGNIFY*/

void
glyph2image(int glyphNo, int size, int angle, XImage *img)
{
	int length;
	static int old_size = 0, old_angle = 0;

	if (!bitmap)
		bitmap = (u8_t *)allocMem(BMPSIZE);
	if (!bitmap)
		return;

	if (size != old_size || angle != old_angle) {
		old_size = size;
		old_angle = angle;
		int xcos = (int)(size * cos(angle * M_PI / 180));
		int xsin = (int)(size * sin(angle * M_PI / 180));
		raster->setPixelSize(xcos, -xsin, xsin, xcos);
		printf("fontsize = %d, angle = %d\n", size, angle);
	}

	GlyphMetrics gm;
	length = raster->putGlyphBitmap(glyphNo, bitmap, bitmap + BMPSIZE, &gm);

	img->width = 0;
	if (length == 0)
		return;

	
	if (raster->anti_aliasing <= 0)
		switch (SCANLINEPAD) {
		case 8:
			break;
		case 16: {
			u16_t *p = (u16_t *)bitmap;
			for (int i = length; --i >= 0; ++p)
				*p = bswaps(*p);
			}
			break;
		case 32: {
			u32_t *p = (u32_t *)bitmap;
			for (int i = length; --i >= 0; ++p)
				*p = bswapl(*p);
			}
			break;
		case 64: {
			u32_t *p = (u32_t *)bitmap;
			for (int i = length; --i >= 0; p += 2) {
					u32_t tmp = p[0];
					p[0] = bswapl(p[1]);
					p[1] = bswapl(tmp);
				}
			}
			break;
		}

	if (raster->anti_aliasing > 0)
		for (int i = length; --i >= 0;)
			pixmap[i] = color[bitmap[i]];
#ifndef MAGNIFY
	else if (MAGNIFY) {
		s8_t *p1 = (s8_t *)bitmap;
		u8_t *p2 = pixmap;
		raster->dX *= 8;	// 1 bpp -> 8 bpp
		for (int h = 0; h < raster->height; ++h) {
			int cy = h / MAGNIFY;
			s8_t m;
			for (int w = 0; w < raster->dX; ++w) {
				if ((w & 7) == 0)
					m = *(p1++);
				int c = ((w / MAGNIFY) ^ cy) & 1;
				*(p2++) = color[(m < 0) ? 3 + c : c];
				m <<= 1;
			}
		}
	}
#endif

	int isPixmap = (MAGNIFY || raster->anti_aliasing > 0);

	img->width = raster->width;
	img->height = raster->height;
	img->xoffset = 0;
	img->format = isPixmap ? ZPixmap : XYBitmap;
	img->data = (char *)(isPixmap ? pixmap : bitmap);
	img->byte_order = LSBFirst;
	img->bitmap_unit = 8;
	img->bitmap_bit_order = MSBFirst;
	img->bitmap_pad = 0;
	img->depth = isPixmap ? 8 : 1;
	img->bytes_per_line = raster->dX;
	img->bits_per_pixel = isPixmap ? 8 : 1;
	img->red_mask = 0xFF0000;
	img->green_mask = 0x00FF00;
	img->blue_mask = 0x0000FF;
	img->obdata = 0;
}

int
xAllocColor(int rgb)
{
	XColor c;
	c.red = (rgb >> 8) & 0xff00;
	c.green = rgb & 0xff00;
	c.blue = (rgb << 8) & 0xff00;
	XAllocColor(display, colorMap, &c);

	return c.pixel;
}

int
main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: showttf fontfile.ttf\n");
		fprintf(stderr, "use the cursor keys to navigate\n");
		return -1;
	}

	char *ttFileName = (argc == 2) ? argv[1] : DEFAULT_FONT;

	pixmap = new u8_t[1024 * 1024];

	int done = 0;

	display = XOpenDisplay(":0");
	visual = XDefaultVisual(display, screen);
	screen = XDefaultScreen(display);
	depth = XDefaultDepth(display, screen);
	rootWindow = XDefaultRootWindow(display);
	colorMap = XDefaultColormap(display, screen);

	color[0] = xAllocColor(0xF0F0F0);
	color[1] = xAllocColor(0xB4B4B4);
	color[2] = xAllocColor(0x787878);
	// XXX: color[3] = xAllocColor(0x3C3C3C);
	color[3] = xAllocColor(0x505050);
	color[4] = xAllocColor(0x000000);

	white = xAllocColor(0xF0F0F0);
	black = xAllocColor(0x000000);
	yellow = xAllocColor(0xFFFF00);

	XGCValues gcv;
	gcv.function = GXcopy;
	gcv.foreground = black;
	gcv.background = white;
	gcv.line_width = 1;
	gcv.line_style = LineSolid;
	unsigned long valuemask = GCFunction | GCForeground;
	textGC = XCreateGC(display, rootWindow, valuemask, &gcv);

	unsigned int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT;

#if 0
	int fid = XLoadFont(display, "TTM20_Bitstream Cyberbit");
	XSetFont(display, textGC, fid);
	XChar2b *tststr = (XChar2b *)"\0a\0b\0c\0\x61";
	//char *tststr = "abcd";
#endif

	Window win = XCreateSimpleWindow(display, rootWindow, 0, 0,
					 width, height, 1,
					 gcv.foreground, gcv.background);
	XStoreName(display, win, "TrueType Viewer");

	int selection_mask = StructureNotifyMask | ExposureMask |
			//   EnterWindowMask | LeaveWindowMask |
			     ButtonPressMask | ButtonReleaseMask |
			//   PointerMotionMask |
			     KeyPressMask | KeyReleaseMask;
	XSelectInput(display, win, selection_mask);

	XMapWindow(display, win);

	raster = new Rasterizer();
	TTFont *ttFont = new TTFont(ttFileName);
	raster->useTTFont(ttFont);

	int fontsize = 16;
	int angle = 0;
	XImage img;

	XEvent event;
	do {
		static int glyphNo = STARTGLYF;
		XNextEvent(display, &event);
		switch (event.type) {
		drawglyph:
#ifndef MAGNIFY
			if (MAGNIFY && fontsize) {
				MAGNIFY = height / (fontsize + 1);
				if (!MAGNIFY) MAGNIFY = 1;
			}
#endif
		{
			int gno2 = glyphNo;
			//ttFont->getGlyphNo16(glyphNo);
			fprintf(stderr, "gno2 %d\n", gno2);
			fflush(stderr);
			glyph2image(gno2, fontsize, angle, &img);
		}
			/* fall through */
		case Expose:
		expose:
			XSetForeground(display, textGC, yellow);
			XFillRectangle(display, win, textGC, 0, 0, width, height);
			XSetForeground(display, textGC, black);
			if (img.width)
				XPutImage(display, win, textGC, &img,
					  0, 0, 4, 4, img.width, img.height);
#if 0
			tststr[3].byte2 = glyphNo;
			tststr[3].byte1 = glyphNo >> 8;
			fprintf(stderr, "glyphNo = %d\n", glyphNo);
			XDrawImageString16(display, win, textGC, 4, 120,
					   tststr, 4);
#endif
			break;

		case KeyPress:
		{
			char c[8];
			unsigned long ks;
			XComposeStatus xcs;
			XLookupString(&event.xkey, c, 8, &ks, &xcs);

			switch (ks) {
			case XK_Escape:
				done = 1;
				break;
			case XK_F2:
				raster->grid_fitting = !raster->grid_fitting;
				goto drawglyph;
			case XK_F3:
				raster->anti_aliasing = !raster->anti_aliasing;
				goto drawglyph;
			case XK_F4:
#ifndef MAGNIFY
				MAGNIFY = !MAGNIFY;
#endif /*MAGNIFY*/
				goto drawglyph;
			case XK_Left:
				--glyphNo;
				goto drawglyph;
			case XK_Right:
				++glyphNo;
				goto drawglyph;
			case XK_Up:
				fontsize += 1;
				goto drawglyph;
			case XK_Down:
				fontsize -= 1;
				if (fontsize < 6)
					fontsize = 4;
				goto drawglyph;
			case XK_Page_Up:
				fontsize += (fontsize >> 2) ? fontsize >> 2 : 1;
				goto drawglyph;
			case XK_Page_Down:
				fontsize -= (fontsize >> 2) ? fontsize >> 2 : 1;
				goto drawglyph;
				if (fontsize < 4)
			case XK_Begin:
				angle = 0;
				fontsize = 8;
#ifndef MAGNIFY
				MAGNIFY = 0;
#endif /*MAGNIFY*/
				goto drawglyph;
			case XK_End:
				angle = 0;
				fontsize = height - 10;
#ifndef MAGNIFY
				MAGNIFY = 0;
#endif /*MAGNIFY*/
				goto drawglyph;
			case XK_Shift_L:
			case XK_Shift_R:
			case XK_Meta_L:
			case XK_Meta_R:
			case XK_Control_L:
			case XK_Control_R:
				goto expose;
			default:
				// XXX: glyphNo = ttFont->getGlyphNo16(0x20AC);
				glyphNo = ttFont->getGlyphNo16(c[0]);
				printf("key = \"%s\" -> %d\n", c, glyphNo);
				glyph2image(glyphNo, fontsize, angle, &img);
			}
			goto expose;
		}

		case ConfigureNotify:
			width = event.xconfigure.width;
			height = event.xconfigure.height;
			goto drawglyph;
			//break;

		case DestroyNotify:
			done = 1;
			break;

		default:
			//printf("XEvent.type = %d\n", event.type);
			break;
		}
	} while (!done);

	if (bitmap)
		deallocMem(bitmap, BMPSIZE);

	delete raster;
	delete ttFont;
	delete [] pixmap;

	cleanupMem();

	XUnmapWindow(display, win);
	XCloseDisplay(display);

	return 0;
}

