static unsigned char * volatile sif= (unsigned char *)0xbfff;

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
