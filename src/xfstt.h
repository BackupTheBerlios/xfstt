/*
 * Header for the X Font Server for TT Fonts
 *
 * $Id: xfstt.h,v 1.1 2002/11/14 12:08:08 guillem Exp $
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef XFSTT_H
#define XFSTT_H

#define TTFN_VERSION 0x0102

typedef struct {
	int	nameOfs;
	U8	nameLen;
	U8	pathLen;
	U8	xlfdLen;
	U8	pad;

	U8	charSet;
	U8	slant;

	// panose attributes
	U8	bFamilyType;
	U8	bSerifStyle;
	U8	bWeight;
	U8	bProportion;
	U8	bContrast;
	U8	bStrokeVariation;
	U8	bArmStyle;
	U8	bLetterForm;
	U8	bMidLine;
	U8	bXHeight;
} TTFNdata;

typedef struct {
	char	magic[4];		// == TTFN
	char	type[4];		// == INFO or NAME
	U16	version;
	U16	key;
	U32	crc;
	//TTFNdata ttfn[];
} TTFNheader;

typedef struct {
	int	numFonts;

	int	maxGlyphs;
	int	maxPoints;
	int	maxTwilight;
	int	maxContours;
	int	maxStack;
	int	maxStore;
	int	maxCvt;
	int	maxFDefs;
	int	maxIDefs;
} TTFNinfo;

typedef struct {
	int	pixel[4];		// pixelsize
	int	point[4];		// pointsize
	int	resolution[2];		// resolution
	int	flags;
} FontParams;

#endif

