// Character Map Table
// (C) Copyright 1997-1998 Herbert Duerr

#include "ttf.h"

CmapTable::CmapTable( RandomAccessFile& f, int offset, int length)
:	RandomAccessFile( f, offset, length)
,	unicodeOffset( 0), macCharOffset( 0)
{
	/*version	= */readUShort();
	S16 nSubTables	= readSShort();

	for( int i = nSubTables; --i >= 0;) {
		U16 platformID		= readUShort();
		U16 encodingID		= readUShort();
		U32 subtableOffset	= readUInt();
		if( platformID == 1 && encodingID == 0)	// MAC
			macCharOffset = subtableOffset;
		if( platformID == 3 && encodingID == 1)	// Win
			unicodeOffset = subtableOffset;
	}

	if( !unicodeOffset)
		return;

	seekAbsolute( unicodeOffset);

	/*format	= */readUShort();	// == 4
	/*length	= */readUShort();
	/*version	= */readUShort();
	nSegments	= readSShort() >> 1;

	/*searchRange	= */readUShort();
	/*entrySelector	= */readUShort();
	/*rangeShift	= */readUShort();

	//short[] endCount;
	//readShort(); // reserved
	//short[] startCount;
	//short[] idDelta;
	//short[] idRangeOffset;
}

int CmapTable::char2glyphNo( char char8)
{
	return unicode2glyphNo( char8);
}

int CmapTable::unicode2glyphNo( U16 unicode)
{
	if( !unicodeOffset) {	// oh no, we usually have a MAC only format
		if( macCharOffset == 0 || unicode > 255)
			return 0;
		seekAbsolute( macCharOffset + 6 + unicode);
		int glyphNo = readUByte();
		dprintf2( "MAC.cmap[ %d] = %d\n", unicode, glyphNo);
		return glyphNo;
	}

	// search for endCount
	int lower = 0;
	int upper = nSegments - 1;
	int index = 0;
	while( upper > lower) {
		index = (upper + lower) >> 1;
		//### read header and do more checking
		seekAbsolute( unicodeOffset + 14 + (index << 1));
		if( readUShort() >= unicode)
			upper = index;
		else
			lower = index + 1;
	}

	// check corresponding startCount
	int ofs = unicodeOffset + 16 + (nSegments << 1) + (lower << 1);
	seekAbsolute( ofs);
	U16 startCount = readUShort();
		if( unicode < startCount)
			return 0;

	// get idDelta
	seekAbsolute( ofs += (nSegments << 1));
	int idDelta = readSShort();

	// get idRangeOffset
	seekAbsolute( ofs += (nSegments << 1));
	U16 idRangeOffset = readUShort();

	// calculate GlyphIndex
	if( idRangeOffset == 0)
		return (unicode + idDelta);

	seekAbsolute( ofs + idRangeOffset + ((unicode - startCount) << 1));
	return readUShort();
}

U16 CmapTable::nextUnicode( U16 unicode)
{
	++unicode;

	if( !unicodeOffset) {	// oh no, we usually have a MAC only format
		if( macCharOffset == 0 || unicode > 255)
			return 0;
		return unicode;
	}

	int lower = 0;
	int upper = nSegments - 1;
	if( lower > upper)
	    return 0;
	while( upper > lower) {
		int index = (upper + lower) >> 1;
		seekAbsolute( unicodeOffset + 14 + (index << 1));
		if( readUShort() >= unicode)
			upper = index;
		else
			lower = index + 1;
	}

	// check corresponding startCount
	seekAbsolute( unicodeOffset + 16 + (nSegments << 1) + (lower << 1));
	int startCount = readUShort();
	if( startCount == 0xffff)
	    return 0;
	if( startCount > unicode)
		return startCount;
	return unicode;
}

U16 CmapTable::firstUnicode()
{
	if( !unicodeOffset)
		return 0;

	seekAbsolute( unicodeOffset + 16 + (nSegments << 1));
	U16 i = readUShort();
	dprintf1( "First Unicode = %d\n", i);
	return i;
}

U16 CmapTable::lastUnicode()
{
	if( !unicodeOffset)
		return 255;

	seekAbsolute( unicodeOffset + 14 + ((nSegments-2) << 1));
	U16 i = readUShort();
	dprintf1( "Last Unicode = %d\n", i);
	return i;
}

