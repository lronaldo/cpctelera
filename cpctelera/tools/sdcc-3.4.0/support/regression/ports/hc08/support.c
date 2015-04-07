
void
_putchar(unsigned char c)
{
  c;
  __asm
    .db 0x9e, 0xed
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
    .db 0x9e, 0xec
  __endasm;
}
