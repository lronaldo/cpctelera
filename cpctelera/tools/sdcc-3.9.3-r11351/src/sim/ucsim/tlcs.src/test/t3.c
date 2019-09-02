const unsigned char a[2]= { 0x11, 0x22 };

void __fail(char *s)
{
}

#define ASSERT(_a)  ((_a) ? (void)0 : __fail ("Assertion failed", #_a, __FILE__, __LINE__))

volatile unsigned char idx;

void main(void)
{
  volatile unsigned char c;
  idx= 1;
  (a[idx]==0x22)?(void)0:__fail("s");
  while (1) ;
}
