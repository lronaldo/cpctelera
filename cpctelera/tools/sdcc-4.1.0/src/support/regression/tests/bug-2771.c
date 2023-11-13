/* bug-2761.c
   z80 peephole optimizer segfault when it incorrectly parsed the set_id label as a set instruction.
 */

#include <testfwk.h>

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka)
unsigned int port;

static unsigned char cpu_id;
static int cpu_cache = 0;
static const char *cpu_bugs = "";
static signed char z80_nmos = -1;

#define CPU_Z80_Z280		2
static void cpu_ident(void)
{
    __asm
set_id:
        ld (_cpu_id),a
    __endasm;

    switch(cpu_id) {
    case CPU_Z80_Z280:	/* FIXME R800.. */
        cpu_cache = 256;
        break;
    }
    if (z80_nmos == 1)
        cpu_bugs = "iff";
}
#endif

void testBug(void)
{
}

