/*
 * Horizontal Device Metrics Table
 *
 * $Id: table_hdmx.cc,v 1.1 2002/11/14 12:08:10 guillem Exp $
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

HdmxTable::HdmxTable(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/* int version = */readUShort();
	nRecords = readSShort();
	recordLen = readSInt() - 1;
	assert(nRecords >= 0 && recordLen > 0);
	assert(length+8 >= nRecords * recordLen);
}

int
HdmxTable::getMaxWidth(int mppemx)
{
	dprintf1("hdmx(mppemx = %d) ", mppemx);

	seekAbsolute(8);
	// XXX: is it possible to avoid the linear search?
	int i;
	for (i = nRecords; --i >= 0; seekRelative(recordLen)) {
		U8 ppem = readUByte();
		if (ppem >= mppemx) {
			int maxWidth = readUByte();
			if (ppem > mppemx)
				dprintf0("<");
			dprintf1("= %d\n", maxWidth);

			return maxWidth;
		}
	}

	dprintf0("not found!\n");

	return 0;
}

int
HdmxTable::getGlyphWidth(int mppemx, int glyphNo)
{
	dprintf2("hdmx(mppemx = %d, glyphNo = %d)", mppemx, glyphNo);

	seekAbsolute(8);
	// XXX: is it possible to avoid the linear search?
	int i;
	for(i = nRecords; --i >= 0; seekRelative(recordLen)) {
		int ppem = readUByte();
		if (ppem == mppemx)
			break;
	}
	if (i < 0) {
		dprintf0(" not found!\n");
		return 0;
	}

	seekRelative(glyphNo + 1);
	int width = readUByte();
	dprintf1(" = %d\n", width);

	return width;
}

