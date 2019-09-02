/*
   bug3424150.c
*/

#include <testfwk.h>

#ifndef __SDCC_pdk14 // Lack of memory
unsigned char _pInputBuf[80];
unsigned char _iInputPos;

void OnEditCancel()
{
	if (!_pInputBuf[_iInputPos] && _iInputPos)	
	{
		_iInputPos --;
	}
	_pInputBuf[_iInputPos] = 0;
}
#endif

void
testBug(void)
{
}

