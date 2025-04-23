volatile static unsigned char *sif;

void
_putchar(unsigned char c)
{
  *sif= 'p';
  *sif= c;
  return;
  c;
  __asm
    ld	a, (0x03, sp)
    .db 0x71, 0xed
  __endasm;
}

void
_initEmu(void)
{
  sif= (unsigned char *)0x7fff;
}

void
_exitEmu(void)
{
  *sif= 's';
  return;
  __asm
    .db 0x71, 0xec
  __endasm;
}
