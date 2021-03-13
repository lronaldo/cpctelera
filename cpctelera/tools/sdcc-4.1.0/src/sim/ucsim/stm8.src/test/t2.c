volatile unsigned char *sif= (unsigned char *)0x7fff;

volatile char c;

int f(int i)
{
  c= i;
  return i;
}

void main(void)
{
  f('H'*256 + 'L');
  *sif= 'p';
  *sif= c;
  *sif= 's';
}
