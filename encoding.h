// Header for remap subsystem
// (C) Copyright 1998 Herbert Duerr

class Encoding
{
public:
	virtual ~Encoding() {}
	virtual int map2unicode( int code) = 0;
	virtual int hasGlyphs( int /*unicodeRange*/[4]) { return 1;}
	static Encoding** getEncodings( char* mapnames);
	static Encoding* findEncoding( char* mapname);
	
	const char* strName;
	const int lenName;

protected:
	Encoding( char* name);
private:
	static Encoding *first, *last;
	Encoding* next;
};

