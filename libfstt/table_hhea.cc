/*
 * Hhead Table
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
 * License along with this library; if not, write to the Free Softaware
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */

#include "ttf.h"

HheaTable::HheaTable(RandomAccessFile &f, int offset, int length):
	RandomAccessFile(f, offset, length)
{
	/* version = */ readUInt();
	yAscent = readSShort();
	yDescent = readSShort();
	/* yGap = */ readSShort();	// == 0
	advWidth = readSShort();
	minLeftBear = readSShort();
	minRightBear = readSShort();
	maxExtent = readSShort();
	caretSlopeNum = readUShort();
	caretSlopeDenom = readUShort();
	/* reserved0 = */ readUShort();	// == 0
	/* reserved1 = */ readUShort();	// == 0
	/* reserved2 = */ readUShort();	// == 0
	/* reserved3 = */ readUShort();	// == 0
	/* reserved4 = */ readUShort();	// == 0
	isMetric = readUShort();
	nLongHMetrics = readUShort();
}

