
unsigned char __xdata * volatile sif;

void
main(void)
{
  volatile unsigned int i, x= 0, j;
  __xdata char *p= (__xdata char *)0;

  sif= (unsigned char __xdata *)0xffff;
  for (j=0; j<10; j++)
    for (i= 0; i<0xfff0; i++)
      {
	char c= p[i];
	p[i]= i&0xff;
	if (p[i] != i&0xff)
	  x++;
	p[i]= c;
      }
  * (char __idata *) 0 = * (char __xdata *) 0xfffe;
  *sif= 's';
}
