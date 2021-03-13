static unsigned char * volatile sif;

void
_putchar(unsigned char c)
{
  *sif= 'p';
  *sif= c;
  return;
  c;
  __asm
    .db 0x9e, 0xed
  __endasm;
}

void
_initEmu(void)
{
  sif= (unsigned char *)0x7f;
}

void
_exitEmu(void)
{
  *sif= 's';
  return;
  __asm
    .db 0x9e, 0xec
  __endasm;
}
