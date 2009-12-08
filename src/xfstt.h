/*
 * Header for the X Font Server for TT Fonts
 *
 * Copyright © 1997-1998 Herbert Duerr
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

#ifndef XFSTT_H
#define XFSTT_H

#define TTFN_VERSION 0x0102

typedef struct {
	int	nameOfs;
	uint8_t nameLen;
	uint8_t pathLen;
	uint8_t xlfdLen;
	uint8_t pad;

	uint8_t charSet;
	uint8_t slant;

	// panose attributes
	uint8_t bFamilyType;
	uint8_t bSerifStyle;
	uint8_t bWeight;
	uint8_t bProportion;
	uint8_t bContrast;
	uint8_t bStrokeVariation;
	uint8_t bArmStyle;
	uint8_t bLetterForm;
	uint8_t bMidLine;
	uint8_t bXHeight;
} TTFNdata;

typedef struct {
	char	magic[4];		// == TTFN
	char	type[4];		// == INFO or NAME
	uint16_t version;
	uint16_t key;
	uint32_t crc;
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

