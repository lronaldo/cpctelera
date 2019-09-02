static unsigned char * volatile sif= (unsigned char *)0xffff;

void main(void)
{
  unsigned int i, j;

  for (i=0; i<0x2233; i++)
    j= i;
  *sif= 's';
  for (;;)
    ;
}
