#include "gpsim_assert.h"
#include "picregs.h"

unsigned char failures=0;
//unsigned bit bit1;

typedef unsigned char byte;


byte uchar0;
const byte arr[] = { 1,2,8,9,0 };

void
done()
{
  ASSERT(MANGLE(failures) == 0);
  PASSED();
}

void  isr(void) __interrupt 1 __using 1
{

  if(arr[3]!=9)
    failures++;
  PORTB = 7;
}

void lcd1(const unsigned char *str)
{
  const char *p = "hello world!\r\n";

  if(!str)
    failures++;

 if(str && str[2] != 'r')
    failures++;

  if(arr[3]!=9)
    failures++;

  while (1) {
    if (0 == *p) break;
    PORTB = *p;
    p++;
  }

}

void main(void)
{
  lcd1("str");
  done();
}
