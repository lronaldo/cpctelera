/* bug-3079.c
   A crash in code generation for 8-bit parameters to __smallc functions.
 */
 
#include <testfwk.h>

#if !defined(__SDCC_z80) && !defined(__SDCC_z180) && !defined(__SDCC_gz80)
#define __smallc
#endif

unsigned char func2()
{
	return 0xa5;
}

int esxdos_f_read(unsigned char handle) __smallc
{
	ASSERT (handle == 0xa5);
	return 0;
}

void
testBug(void)
{
        unsigned char h;

        h =func2();
        esxdos_f_read(h); // SIGSEGV when h allocated to lower half of register pair.
}

