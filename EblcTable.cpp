// Embedded Bitmap Loca Table
// (C) Copyright 1997-1998 Herbert Duerr

#include "ttf.h"

EblcTable::EblcTable( RandomAccessFile& f, int offset, int length)
:	RandomAccessFile( f, offset, length)
{
	/*int version = */readUInt();	// should be 0x00020000
	int numSizes = readUInt();
}

void EblcTable::dummy()
{
	/*idxSTAO =*/ readUInt();
	/*U32 colorRef = */readUInt();	// should be 0

	//horizontal SBit metrics
	/*hAscend		=*/ readSByte();
	/*hDescend	=*/ readSByte();
	/*hmaxWidth	=*/ readUByte();
	/*hcsNumerator	=*/ readSByte();
	/*hcsDenominator	=*/ readSByte();
	/*hcaretOffset	=*/ readSByte();
	/*hminOriginSB	=*/ readSByte();
	/*hminAdvanceSB	=*/ readSByte();
	/*hmaxBeforeBL	=*/ readSByte();
	/*hminAfterBL	=*/ readSByte();
	/*pad1 = */readSByte();
	/*pad2 = */readSByte();

	//vertical SBit metrics
	/*vAscend		=*/ readSByte();
	/*vDescend	=*/ readSByte();
	/*vmaxWidth	=*/ readUByte();
	/*vcsNumerator	=*/ readSByte();
	/*vcsDenominator	=*/ readSByte();
	/*vcaretOffset	=*/ readSByte();
	/*vminOriginSB	=*/ readSByte();
	/*vminAdvanceSB	=*/ readSByte();
	/*vmaxBeforeBL	=*/ readSByte();
	/*vminAfterBL	=*/ readSByte();
	/*pad1 = */readSByte();
	/*pad2 = */readSByte();

	/*startGlyph	=*/ readUShort();
	/*endGlyph	=*/ readUShort();
	/*ppemx		=*/ readUByte();
	/*ppemy		=*/ readUByte();
	/*bitDepth	=*/ readUByte();	// should be 1
	/*flags		=*/ readSByte();
}

