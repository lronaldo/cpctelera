/* bug-3389.c

   Missing error on return from __z88dk_calllee function with variable arguments.
 */

#ifdef TEST1
#if !defined(__SDCC_z80) && !defined(__SDCC_z180) && !defined(__SDCC_sm83) && !defined(__SDCC_stm8)
#define __z88dk_callee
#endif

void f(int i, ...) __z88dk_callee
{
} /* ERROR(SDCC_z80|SDCC_z180|SDCC_sm83|SDCC_stm8) */
#endif

