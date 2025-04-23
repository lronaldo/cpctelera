/** push/pop tests for mcs51.
*/
#include <testfwk.h>

__data unsigned char _SP0, _SP1, _SP2;

void test(void)
{
#ifdef __SDCC_mcs51
	__asm
		mov	__SP0,SP
		push	SP
		mov	r0,SP
		mov	__SP1,@r0
		mov	@r0,#0xFE
		pop	SP
		mov	__SP2,SP
		mov	SP,__SP0
	__endasm;

	ASSERT(_SP0 == _SP1);
	ASSERT(_SP2 == 0xFE);
#endif
}
