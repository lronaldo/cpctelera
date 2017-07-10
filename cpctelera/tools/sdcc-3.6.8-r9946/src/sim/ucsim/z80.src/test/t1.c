static unsigned char * volatile sif= (unsigned char *)0xffff;

volatile unsigned char x;

void
putchar(unsigned char c)
{
  *sif= 'p';
  *sif= c;
}

void
prints(char *s)
{
  while (*s)
    putchar(*s++);
}

void
main(void)
{
  unsigned char i;
  unsigned int j;

  prints("Start.\n");
  for (j= 0; j<41000; j++)
    {
      x= j;
      i= j;
      putchar('a');
    }

  prints("Done.\n");
  *sif= 's';
  for (;;)
    ;
}
