/*
   bug3424150.c
*/

#include <testfwk.h>

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

void
testBug(void)
{
}

