/*
 * Horizontal Device Metrics Table
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
	debug("hdmx(mppemx = %d) ", mppemx);

	seekAbsolute(8);
	// XXX: is it possible to avoid the linear search?
	int i;
	for (i = nRecords; --i >= 0; seekRelative(recordLen)) {
		u8_t ppem = readUByte();
		if (ppem >= mppemx) {
			int maxWidth = readUByte();
			if (ppem > mppemx)
				debug("<");
			debug("= %d\n", maxWidth);

			return maxWidth;
		}
	}

	debug("not found!\n");

	return 0;
}

int
HdmxTable::getGlyphWidth(int mppemx, int glyphNo)
{
	debug("hdmx(mppemx = %d, glyphNo = %d)", mppemx, glyphNo);

	seekAbsolute(8);
	// XXX: is it possible to avoid the linear search?
	int i;
	for(i = nRecords; --i >= 0; seekRelative(recordLen)) {
		int ppem = readUByte();
		if (ppem == mppemx)
			break;
	}
	if (i < 0) {
		debug(" not found!\n");
		return 0;
	}

	seekRelative(glyphNo + 1);
	int width = readUByte();
	debug(" = %d\n", width);

	return width;
}

