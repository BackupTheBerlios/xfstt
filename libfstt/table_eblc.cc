/*
 * Embedded Bitmap Location Table
 *
 * $Id: table_eblc.cc,v 1.1 2002/11/14 12:08:10 guillem Exp $
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

#include "ttf.h"

EblcTable::EblcTable(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/*int version = */readUInt();	// should be 0x00020000
	int numStrikes = readUInt();

	for (int i = 0; i < numStrikes; ++i)
		readStrike(0, 0, 0);
}

void
EblcTable::readStrike(int glyphNo, int _ppemx, int _ppemy)
{
	int strikeOfs = readUInt();
	/* strikeSize = */ readUInt();
	int strikeNum = readUInt();
	/* U32 colorRef = */ readUInt();	// should be 0

	// horizontal sbit metrics
	/* ascend = */ readSByte();
	/* descend = */ readSByte();
	/* maxWidth = */ readUByte();
	/* csNumerator = */ readSByte();
	/* csDenominator = */ readSByte();
	/* caretOffset = */ readSByte();
	/* minOriginSB = */ readSByte();
	/* minAdvanceSB = */ readSByte();
	/* maxBeforeBL = */ readSByte();
	/* minAfterBL = */ readSByte();
	/* pad1 = */ readSByte();
	/* pad2 = */ readSByte();

	// vertical sbit metrics
	/* ascend = */ readSByte();
	/* descend = */ readSByte();
	/* maxWidth = */ readUByte();
	/* csNumerator = */ readSByte();
	/* csDenominator = */ readSByte();
	/* caretOffset = */ readSByte();
	/* minOriginSB = */ readSByte();
	/* minAdvanceSB = */ readSByte();
	/* maxBeforeBL = */ readSByte();
	/* minAfterBL = */ readSByte();
	/* pad1 = */ readSByte();
	/* pad2 = */ readSByte();

	int startGlyph = readUShort();
	int endGlyph = readUShort();
	int ppemx = readUByte();
	int ppemy = readUByte();
	/* bitDepth = */ readUByte();	// should be 1
	int flags = readSByte();	// 1 hmetric, 2 vmetric

	printf("EBLC\nglyph(%3d - %3d), size(%2d, %2d), flags %d\n",
	       startGlyph, endGlyph, ppemx, ppemy, flags);

	int ofs = tell();
	seekAbsolute(strikeOfs);
	for (int i = 0; i < strikeNum; ++i)
		readSubTableArray(glyphNo, strikeOfs);
	seekAbsolute(ofs);
}

void
EblcTable::readSubTableArray(int glyphNo, int ofsSTA)
{
	int firstGlyph = readUShort();
	int lastGlyph = readUShort();
	int addOffset = readUInt();
	printf("SubTable glyphs %3d - %3d, addofs 0x%04X\n",
	       firstGlyph, lastGlyph, addOffset);
	int ofs = tell();
	seekAbsolute(ofsSTA + addOffset);
	readSubTable(firstGlyph, lastGlyph);
	seekAbsolute(ofs);
}

void
EblcTable::readSubTable(int first, int last)
{
	int idxFormat = readUShort();
	int imgFormat = readUShort();
	int imageOffset = readUInt();

	printf("idxfmt %d, imgfmt %d, imgofs 0x%05X\n",
	       idxFormat, imgFormat, imageOffset);

	int i;
	switch (idxFormat) {
	case 1:
		for (i = first; i <= last; ++i)
			printf("ofs%02X = %04X\n", i, readUInt());
		break;
	case 2:
		printf("imgsize %d\n", readUInt());
		printf("bigGlyphMetrics\n");
		break;
	case 3:
		for (i = first; i <= last; ++i)
			printf("ofs%04X = %04X\n", i, readUShort());
		break;
	case 4:
		i = readUInt();
		printf("numGlyphs %d\n", i);
		while (--i >= 0)
			printf("ofs%04X = %04X\n", readUShort(), readUShort());
		break;
	case 5:
		printf("imgsize %d\n", readUInt());
		printf("bigGlyphMetrics\n");
		seekRelative(8);
		i = readUInt();
		printf("numGlyphs %d\n", i);
		while (--i >= 0)
			printf("ofs%04X = %04X\n", readUShort(), readUShort());
		break;
	default:
		printf("Illegal index format!\n");
		break;
	}
}

