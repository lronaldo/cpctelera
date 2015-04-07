/*
    bug-2890326.c
*/

#include <testfwk.h>

unsigned short _sLineLen;
unsigned char *_pLine;

void line_add_crlf()
{
	_pLine[_sLineLen] = '\r';
	_sLineLen ++;
	_pLine[_sLineLen] = '\n';
	_sLineLen ++;
	_pLine[_sLineLen] = 0;
}

void testBug(void)
{
	unsigned char b[4];
	_sLineLen = 0;
	_pLine = b;
	line_add_crlf();
	ASSERT(b[1] == '\n');
	ASSERT(!b[2]);
	ASSERT(_sLineLen == 2);
}

