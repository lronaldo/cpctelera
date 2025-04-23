/*
    bug 3361290
*/

#include <testfwk.h>

unsigned volatile char rWatchDog, _bTimer;
#define FALSE 0

void KickDog()
{
rWatchDog |= 0x08; // kick dog
}


void _MiniRun()
{
if (_bTimer)
{
_bTimer = FALSE;
}
KickDog();
}

void
testBug(void)
{
	ASSERT(1);
}

