/*
 * OS2 Info Table
 *
 * $Id: table_os2.cc,v 1.1 2002/11/14 12:08:11 guillem Exp $
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

OS2Table::OS2Table(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/* S16 version =*/ readUShort();
	avg_width = readUShort();
	weightClass = readUShort();
	/* S16 usWidthClass = */ readUShort();
	/* S16 fsType = */ readUShort();
	/* S16 ySubXSize = */ readSShort();
	/* S16 ySubYSize = */ readSShort();
	/* S16 ySubXOffset = */ readSShort();
	/* S16 ySubYOffset = */ readSShort();
	/* S16 ySuperXSize = */ readSShort();
	/* S16 ySuperYSize = */ readSShort();
	/* S16 ySuperXOffset = */ readSShort();
	/* S16 ySuperYOffset = */ readSShort();
	/* S16 yStrikeoutSize = */ readSShort();
	/* S16 yStrikeoutPos = */ readSShort();
	/* S16 familyClass = */ readUShort();
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

