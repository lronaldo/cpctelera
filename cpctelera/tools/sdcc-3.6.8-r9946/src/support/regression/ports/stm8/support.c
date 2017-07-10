void
_putchar(unsigned char c)
{
  c;
  __asm
    ld	a, (0x03, sp)
    .db 0x71, 0xed
  __endasm;
}

void
_initEmu(void)
{
}

void
_exitEmu(void)
{
  __asm
    .db 0x71, 0xec
  __endasm;
}
