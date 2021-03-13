/*
   bug-3003.c

   The wrong location is used for a local variable; observable effects vary by backend.
   For stm8, this results in overwriting other local variables.
   For z80, it results in use of location below the stack.
*/

#include <testfwk.h>

#include <stdint.h>
#include <string.h>

#pragma disable_warning 88

void g(void)
{
}

static volatile uint8_t* f(const uint8_t id)
{
    volatile uint8_t* result = 0; // Bug: For stm8, this variable is accessed at stack at sp+255, overwriting part of buffer.
    switch (id)
    {
        case 0:
            result = (volatile uint8_t *)0x5000;
            break;
        case 1:
            result = (volatile uint8_t *)0x5005;
            break;
        case 2:
            result = (volatile uint8_t *)0x500a;
            break;
    }
    g(); // This call overwrites result by hte return address for z80.
    return result;
}

#if defined (__SDCC_pdk14) || defined (__SDCC_pdk15) || defined (__SDCC_mcs51) // Lack of memory
#define BUFFERSIZE 20
#else
#define BUFFERSIZE 300
#endif

void testBug(void)
{
	unsigned char buffer[BUFFERSIZE]; // Bug: Overwritten in f() on stm8.
	
	memset(buffer, 0xa5, BUFFERSIZE);

	ASSERT(f(0) == (volatile uint8_t *)0x5000);
	ASSERT(f(1) == (volatile uint8_t *)0x5005);
	ASSERT(f(2) == (volatile uint8_t *)0x500a);

	for(size_t i = 0; i < BUFFERSIZE; i++)
		ASSERT (buffer[i] == 0xa5);
}

