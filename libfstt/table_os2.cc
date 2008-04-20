/*
 * OS2 Info Table
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

#include "ttf.h"

OS2Table::OS2Table(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/* s16_t version =*/ readUShort();
	avg_width = readUShort();
	weightClass = readUShort();
	/* s16_t usWidthClass = */ readUShort();
	/* s16_t fsType = */ readUShort();
	/* s16_t ySubXSize = */ readSShort();
	/* s16_t ySubYSize = */ readSShort();
	/* s16_t ySubXOffset = */ readSShort();
	/* s16_t ySubYOffset = */ readSShort();
	/* s16_t ySuperXSize = */ readSShort();
	/* s16_t ySuperYSize = */ readSShort();
	/* s16_t ySuperXOffset = */ readSShort();
	/* s16_t ySuperYOffset = */ readSShort();
	/* s16_t yStrikeoutSize = */ readSShort();
	/* s16_t yStrikeoutPos = */ readSShort();
	/* s16_t familyClass = */ readUShort();
	panose[0] = readUByte();
	panose[1] = readUByte();
	panose[2] = readUByte();
	panose[3] = readUByte();
	panose[4] = readUByte();
	panose[5] = readUByte();
	panose[6] = readUByte();
	panose[7] = readUByte();
	panose[8] = readUByte();
	panose[9] = readUByte();
	unicodeRange[0] = readUInt();
	unicodeRange[1] = readUInt();
	unicodeRange[2] = readUInt();
	unicodeRange[3] = readUInt();
	/* int vendorId = */ readUInt();
	selection = readUShort();
	firstCharNo = readUShort();
	lastCharNo = readUShort();
	typoAscent = readUShort();
	typoDescent = readUShort();
	typoGap = readUShort();
	winAscent = readUShort();
	winDescent = readUShort();
}

