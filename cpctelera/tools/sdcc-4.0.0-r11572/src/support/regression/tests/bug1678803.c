/*
    bug 1678803
    This should not generate error 12 "called object is not a function".
*/

#include <testfwk.h>

typedef void (*func)(void);

void foo(void)
{
}

#ifdef __SDCC_mcs51
func GetFunc(void) __naked
{
  __asm

    ; some assembler code
    mov dptr,#_foo
#ifdef __SDCC_MODEL_HUGE
    mov B,#_foo>>16
    ljmp __sdcc_banked_ret
#else
    ret
#endif
  __endasm;
}
#endif

void testCaller(void)
{
#ifdef __SDCC_mcs51
  GetFunc()();
#endif

  ASSERT (1);
}
