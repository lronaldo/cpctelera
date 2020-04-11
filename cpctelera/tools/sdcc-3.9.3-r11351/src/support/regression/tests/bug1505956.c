/*
   bug3037889.c
   All mnemonics need to be on separate lines now. So this is no longer allowed
   #define NOP2 __asm nop nop __endasm;
 */

#include <testfwk.h>

#if defined __SDCC_mcs51 || defined __SDCC_ds390

#define TEST_MACRO_20() { \
	/*this macro has 20 instructions in it*/ \
	__asm \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
		nop \
	__endasm; \
}

volatile unsigned char testvar = 0;

void bug1406219 (void)
{
	if ( testvar > 0 )
	{
		/* this code generates an error :
?ASxxxx-Error-<a> in line 493 of err.asm
              <a> machine specific addressing or addressing mode error
removing err.rel

	(the generated jnc cannot branch the entire distance of the if statement.)
*/

		TEST_MACRO_20();
		TEST_MACRO_20();

		TEST_MACRO_20();
		TEST_MACRO_20();

		TEST_MACRO_20();
		TEST_MACRO_20();

		TEST_MACRO_20();
	}
	testvar = 0;
}

// macro with __asm/__endasm pair
#define NOP1 do { __asm nop __endasm; } while (0)

// multi-line macro with __asm/__endasm pair and operands
#define NOP2 do { __asm	cpl a \
						cpl a \
				__endasm; } while (0)

#define NOP4 do { __asm	inc a \
						inc a \
						dec a \
						dec a \
				__endasm; } while (0)
#define NOP8 do { NOP4; NOP4; } while (0)
#define NOP16 do { NOP8; NOP8; } while (0)
#define NOP32 do { NOP16; NOP16; } while (0)
#define NOP64 do { NOP32; NOP32; } while (0)


volatile char t;

void bug1505956 (void)
{
	volatile unsigned char i;

	for( i=0; i<100; i++ )
	{
		t++;
		NOP64;
		NOP32;
		NOP16;
		NOP8;
//		NOP4;
		NOP2;
//		NOP1;
	}

	t++;
}

// macro without __asm/__endasm pair
#define GETC        \
    clr a           \
    movc a,@a+dptr

void bug_DS400_ROM (void)
{
    __asm
        GETC    ;comment
    __endasm;
}

// macro with other macro
#define ADDRESS _count
#define RECURSIVE __asm inc ADDRESS \
                        inc ADDRESS \
                  __endasm

void bug3407198 (void)
{
	RECURSIVE;
}

#else

#define RECURSIVE

#endif

__data char count = 0;

// __asm ... __endasm must stay one C statement
char bug3407279 (char x)
{
	if (x)
		RECURSIVE;
	return count;
}

void testBug(void)
{
	ASSERT(bug3407279(0) == 0);
}

