static unsigned char * volatile sif= (unsigned char *)0x00ff;

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
  unsigned int i;
  unsigned int j;

  prints("Start.\n");
  for (j= 0; j<200; j++)
    {
      for (i= 1000; i; i--)
	putchar('a');
    }

  prints("Done.\n");
  *sif= 's';
  for (;;)
    ;
}
