/*
 * Header for the X Font Server for TT Fonts
 *
 * $Id$
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
	u8_t	nameLen;
	u8_t	pathLen;
	u8_t	xlfdLen;
	u8_t	pad;

	u8_t	charSet;
	u8_t	slant;

	// panose attributes
	u8_t	bFamilyType;
	u8_t	bSerifStyle;
	u8_t	bWeight;
	u8_t	bProportion;
	u8_t	bContrast;
	u8_t	bStrokeVariation;
	u8_t	bArmStyle;
	u8_t	bLetterForm;
	u8_t	bMidLine;
	u8_t	bXHeight;
} TTFNdata;

typedef struct {
	char	magic[4];		// == TTFN
	char	type[4];		// == INFO or NAME
	u16_t	version;
	u16_t	key;
	u32_t	crc;
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

