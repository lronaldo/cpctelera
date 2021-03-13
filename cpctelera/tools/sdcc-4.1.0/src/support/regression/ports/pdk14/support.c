void
_putchar(unsigned char c)
{
  c;
#ifndef __SDCC_STACK_AUTO
  __asm
    mov a, __putchar_PARM_1+0
    .db 0x00, 0xff
  __endasm;
#else
  __asm
    mov	a, sp
    add	a, #0xfc
    mov	p, a
    idxm	a, p
    .db 0x00, 0xff
  __endasm;
#endif
}

void
_initEmu(void)
{
}

void
_exitEmu(void)
{
  __asm
    stopsys
  __endasm;
}
