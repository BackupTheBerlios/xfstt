/*
 * Grid-fitting And Scan-conversion Procedure Table
 *
 * $Id: table_gasp.cc,v 1.2 2003/06/18 05:42:03 guillem Exp $
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

GaspTable::GaspTable(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/* int version = */ readUShort();
	nRanges = readUShort();
}

int
GaspTable::getFlags(int mppem)
{
	seekAbsolute(4);

	int flags = 0;

	for (int i = nRanges; --i >= 0;) {
		int rangeMax = readUShort();
		flags = readUShort();
		if (rangeMax >= mppem)
			break;
	}

	debug("gasp::getFlags(mppem = %d) = 0x%02X\n", mppem, flags);

	return flags;
}

