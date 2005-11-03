/*
 * Vertical Device Metrics
 *
 * $Id$
 *
 * Copyright (C) 1997-1998  Herbert Duerr
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

VdmxTable::VdmxTable(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/* int version = */ readUShort();
	nRecords = readSShort();
	nRatios = readUShort();
}

int
VdmxTable::getYmax(int pelHeight, int xres, int yres, int *ymax, int *ymin)
{
	u16_t offset = 0;

	for (int i = nRatios; --i >= 0;) {
		/* u8_t charSet = */ readUByte();
		u8_t xRatio = readUByte();
		u8_t yStartRatio= readUByte();
		u8_t yEndRatio = readUByte();
		u16_t tmp = readUShort();

		if ((yres * xRatio >= xres * yStartRatio) &&
		    (yres * xRatio <= xres * yEndRatio)) {
			offset = tmp;
			break;
		}
	}

	if (offset == 0)
		return 0;

	seekAbsolute(offset);

	int nrecs = readUShort();
	u8_t startSize = readUByte();
	u8_t endSize = readUByte();

	if (pelHeight < startSize || pelHeight > endSize)
		return 0;

	// XXX: should be a binary search
	while (--nrecs >= 0) {
		u16_t ph = readUShort();
		s16_t y1 = readSShort();
		s16_t y2 = readSShort();

		if (pelHeight == ph) {
			*ymax = y1;
			*ymin = y2;
			return 1;
		}
	}

	return 1;
}

