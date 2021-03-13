static unsigned char * volatile sif= (unsigned char *)0x7fff;

void
_putchar(unsigned char c)
{
  *sif= 'p';
  *sif= c;
}

void
_initEmu(void)
{
}

void
_exitEmu(void)
{
  *sif= 's';
}
