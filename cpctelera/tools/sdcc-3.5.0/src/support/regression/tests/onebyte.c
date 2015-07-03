/** test one byte mul/div/mod operations.

  attrL: volatile,
  attrR: volatile,
*/
#include <testfwk.h>

void
testMul (void)
{
  {attrL}    signed  char  cL;
  {attrL}  unsigned  char ucL;
  {attrR}    signed  char  cR;
  {attrR}  unsigned  char ucR;
  volatile   signed  char   r8 , r8b;
  volatile unsigned  char  ur8, ur8b;
  volatile           short r16, r16b;
#ifndef __SDCC_pic16
  cL = -127; cR =   -5; r16 =  cL * cR; r16b = cR *  cL; ASSERT (r16 ==  635); ASSERT (r16b ==  635);
  cL =  127; cR =   -5; r16 =  cL * cR; r16b = cR *  cL; ASSERT (r16 == -635); ASSERT (r16b == -635);
  cL = -127; cR =    5; r16 =  cL * cR; r16b = cR *  cL; ASSERT (r16 == -635); ASSERT (r16b == -635);
  cL =  127; cR =    5; r16 =  cL * cR; r16b = cR *  cL; ASSERT (r16 ==  635); ASSERT (r16b ==  635);

  cL = -128; cR =   -1; ur8  =  cL * cR; ur8b  = cR *  cL; ASSERT (ur8  ==  128); ASSERT (ur8b  ==  128);
  cL =  128; cR =   -1;  r8  =  cL * cR;  r8b  = cR *  cL; ASSERT ( r8  == -128); ASSERT ( r8b  == -128);
  cL = -128; cR =    1;  r8  =  cL * cR;  r8b  = cR *  cL; ASSERT ( r8  == -128); ASSERT ( r8b  == -128);
  cL =  128; cR =    1; ur8  =  cL * cR; ur8b  = cR *  cL; ASSERT (ur8  ==  128); ASSERT (ur8b  ==  128);

  ucL = 128; cR =   -5; r16 = ucL * cR; r16b = cR * ucL; ASSERT (r16 == -640); ASSERT (r16b == -640);
  ucL = 128; cR =    5; r16 = ucL * cR; r16b = cR * ucL; ASSERT (r16 ==  640); ASSERT (r16b ==  640);

  ucL = 127;  cR =  -1;  r8 = ucL * cR;   r8b  = cR * ucL; ASSERT ( r8 == -127); ASSERT ( r8b  == -127);
  ucL = 128;  cR =   1; ur8 = ucL * cR;  ur8b  = cR * ucL; ASSERT (ur8 ==  128); ASSERT (ur8b  ==  128);
  ucL = 128; ucR =   5; r16 = ucL * ucR; r16b = ucR * ucL; ASSERT (r16 ==  640); ASSERT (r16b ==  640);
  ucL = 128; ucR =   1; ur8 = ucL * ucR; ur8b = ucR * ucL; ASSERT (ur8 ==  128); ASSERT (ur8b ==  128);

  ucL =  254;  cR = -1; r16 = ucL *  cR; ASSERT (r16 == -254);
   cL = -128;  cR = -1; r16 =  cL *  cR; ASSERT (r16 ==  128);
#endif
}

void
testDiv (void)
{
  {attrL}    signed char  cL;
  {attrL}  unsigned char ucL;
  {attrR}    signed char  cR;
  {attrR}  unsigned char ucR;
  volatile   signed char  r8;
  volatile unsigned char ur8;
  volatile         short r16;

   cL = -128;  cR =   -2; r8  = cL / cR; ASSERT (r8  ==  64);
   cL = -128;  cR =   -2; r16 = cL / cR; ASSERT (r16 ==  64);

  ucL =  255; ucR =    3; r8 = ucL / ucR; ASSERT (r8 ==  85);
  ucL =  255; ucR =  255; r8 = ucL / ucR; ASSERT (r8 ==   1);
  ucL =    3; ucR =  255; r8 = ucL / ucR; ASSERT (r8 ==   0);

   cL =  127;  cR =    3; r8 =  cL /  cR; ASSERT (r8 ==  42);
   cL = -127;  cR =    3; r8 =  cL /  cR; ASSERT (r8 == -42);
   cL =  127;  cR =   -3; r8 =  cL /  cR; ASSERT (r8 == -42);
   cL = -127;  cR =   -3; r8 =  cL /  cR; ASSERT (r8 ==  42);

  ucL =  127;  cR =    3; r8 = ucL /  cR; ASSERT (r8 ==  42);
  ucL =  255;  cR =    3; r8 = ucL /  cR; ASSERT (r8 ==  85);
  ucL =  127;  cR =   -3; r8 = ucL /  cR; ASSERT (r8 == -42);
  ucL =  255;  cR =   -3; r8 = ucL /  cR; ASSERT (r8 == -85);

   cL =  127; ucR =    3; r8 =  cL / ucR; ASSERT (r8 ==  42);
   cL = -127; ucR =    3; r8 =  cL / ucR; ASSERT (r8 == -42);
   cL =  127; ucR =  128; r8 =  cL / ucR; ASSERT (r8 ==   0);
   cL = -127; ucR =  128; r8 =  cL / ucR; ASSERT (r8 ==   0);

   cL =  127;  cR =    1; r8  =  cL /  cR; ASSERT (r8  == 127);
   cL =  127;  cR =    1; r16 =  cL /  cR; ASSERT (r16 == 127);

  ucL = 251;   cR =    1; ur8 = ucL /  cR; ASSERT (ur8 == 251);
  ucL = 251;   cR =    1; r16 = ucL /  cR; ASSERT (r16 == 251);

  ucL = 253;   cR =   -3; r8  = ucL /  cR; ASSERT (r8  == -84);
  ucL = 253;   cR =   -3; r16 = ucL /  cR; ASSERT (r16 == -84);

  ucL =  254;  cR =   -1; r16 = ucL /  cR; ASSERT (r16 == -254);
   cL = -128;  cR =   -1; r16 =  cL /  cR; ASSERT (r16 ==  128);
}

void
testMod (void)
{
  {attrL}    signed char  cL;
  {attrL}  unsigned char ucL;
  {attrR}    signed char  cR;
  {attrR}  unsigned char ucR;
  volatile   signed char  r8;
  volatile unsigned char ur8;
  volatile         short r16;

  ucL =  128;  cR =    5; r16 = ucL %  cR; ASSERT (r16 ==  3);
  ucL =  128;  cR =   -5; r16 = ucL %  cR; ASSERT (r16 ==  3);
  ucL =  128; ucR =    5; r16 = ucL % ucR; ASSERT (r16 ==  3);

  ucL =  128; ucR =  255; ur8 = ucL % ucR; ASSERT (ur8 == 128);
  ucL =  128; ucR =  255; r16 = ucL % ucR; ASSERT (r16 == 128);

  ucL =  128;  cR =  127; r8  = ucL %  cR; ASSERT (r8  ==  1);

   cL =  127;  cR =    5; r16 =  cL %  cR; ASSERT (r16 ==  2);
   r16 = -1;
   cL =  127;  cR =   -5; r16 =  cL %  cR; ASSERT (r16 ==  2);
   r16 = -1;
   cL =  127; ucR =    5; r16 =  cL % ucR; ASSERT (r16 ==  2);

   cL = -128;  cR =    5; r16 =  cL %  cR; ASSERT (r16 == -3);
   r16 = 0;
   cL = -128;  cR =   -5; r16 =  cL %  cR; ASSERT (r16 == -3);
   r16 = 0;
   cL = -128; ucR =    5; r16 =  cL % ucR; ASSERT (r16 == -3);
}

void
testOr (void)
{
  {attrL}    signed char  cL;
  {attrL}  unsigned char ucL;
  {attrR}    signed char  cR;
  {attrR}  unsigned char ucR;
  volatile         short r16, r16b;

   cL = 0x00;  cR = 0x80; r16 =  cL |  cR; r16b =  cR |  cL; ASSERT (r16 == (short) 0xff80); ASSERT (r16b == (short) 0xff80);
  ucL = 0x80;  cR = 0x00; r16 = ucL |  cR; r16b =  cR | ucL; ASSERT (r16 ==           0x80); ASSERT (r16b ==           0x80);
  ucL = 0x80; ucR = 0x80; r16 = ucL | ucR; r16b = ucR | ucL; ASSERT (r16 ==           0x80); ASSERT (r16b ==           0x80);
}

void
testXor (void)
{
  {attrL}    signed char  cL;
  {attrL}  unsigned char ucL;
  {attrR}    signed char  cR;
  {attrR}  unsigned char ucR;
  volatile         short r16, r16b;

   cL = 0x80;  cR = 0x80; r16 =  cL ^  cR; r16b =  cR ^  cL; ASSERT (r16 ==              0); ASSERT (r16b ==              0);
   cL = 0x80;  cR = 0x00; r16 =  cL ^  cR; r16b =  cR ^  cL; ASSERT (r16 == (short) 0xff80); ASSERT (r16b == (short) 0xff80);

  ucL = 0x80;  cR = 0x80; r16 = ucL ^  cR; r16b =  cR ^ ucL; ASSERT (r16 == (short) 0xff00); ASSERT (r16b == (short) 0xff00);
  ucL = 0x80;  cR =    0; r16 = ucL ^  cR; r16b =  cR ^ ucL; ASSERT (r16 ==           0x80); ASSERT (r16b ==           0x80);
  ucL =    0;  cR = 0x80; r16 = ucL ^  cR; r16b =  cR ^ ucL; ASSERT (r16 == (short) 0xff80); ASSERT (r16b == (short) 0xff80);

  ucL = 0x80; ucR = 0x80; r16 = ucL ^ ucR; r16b = ucR ^ ucL; ASSERT (r16 ==      0); ASSERT (r16b ==      0);
  ucL =    0; ucR = 0x80; r16 = ucL ^ ucR; r16b = ucR ^ ucL; ASSERT (r16 ==   0x80); ASSERT (r16b ==   0x80);
}

void
testAnd (void)
{
  {attrL}    signed char  cL;
  {attrL}  unsigned char ucL;
  {attrR}    signed char  cR;
  {attrR}  unsigned char ucR;
  volatile         short r16, r16b;

   cL = 0x80;  cR = 0x80; r16 =  cL &  cR; r16b =  cR &  cL; ASSERT (r16 == (short) 0xff80); ASSERT (r16b == (short) 0xff80);
  ucL = 0x80;  cR = 0x80; r16 = ucL &  cR; r16b =  cR & ucL; ASSERT (r16 ==           0x80); ASSERT (r16b ==           0x80);
  ucL = 0x80; ucR = 0x80; r16 = ucL & ucR; r16b = ucR & ucL; ASSERT (r16 ==           0x80); ASSERT (r16b ==           0x80);
}

void
testComplement (void)
{
  {attrR}    signed char  c;
  {attrR}  unsigned char uc;
  volatile         short r16;

   c = 0x00;  r16 = ~ c; ASSERT (r16    == (short) 0xffff); ASSERT (~ c < 0);
  uc = 0x00;  r16 = ~uc; ASSERT (r16    == (short) 0xffff); ASSERT (~uc < 0);
  ASSERT (~ (signed char)   0x00 == (short) 0xffff); ASSERT (~ (signed char)   0x00 < 0);
  ASSERT (~ (unsigned char) 0x00 == (short) 0xffff); ASSERT (~ (unsigned char) 0x00 < 0);
   c = 0x80;  r16 = ~ c; ASSERT (r16 == (short) 0x007f); ASSERT (~ c > 0);
  uc = 0x80;  r16 = ~uc; ASSERT (r16 == (short) 0xff7f); ASSERT (~uc < 0);
  ASSERT (~ (signed char)   0x80 == (short) 0x007f); ASSERT (~ (signed char)   0x80 > 0);
  ASSERT (~ (unsigned char) 0x80 == (short) 0xff7f); ASSERT (~ (unsigned char) 0x80 < 0);

  ASSERT (~ 1   < 0);
  ASSERT (~ 1u  > 0);
  ASSERT (~ 1l  < 0);
  ASSERT (~ 1ul > 0);
}

void
testCompare (void)
{
  {attrL}   signed char  c;
  {attrR} unsigned char uc;

   c = 0x80; /* -128 */
  uc = 0x80; /* +128 */

  ASSERT (!(c == uc));
  ASSERT (  c != uc );
  ASSERT (  c <  uc );
  ASSERT (  c <= uc );
  ASSERT (!(c >  uc));
  ASSERT (!(c >= uc));
}

void
testUMinus (void)
{
    signed char  {attrL} sc;
  unsigned char  {attrL} uc;
    signed int   {attrL} si;
  unsigned int   {attrL} ui;
    signed long  {attrL} sl;
  unsigned long  {attrL} ul;

  ASSERT (-(53l ) < 0);
  ASSERT (-(53ul) > 0);
  ul = 53;
  ASSERT (-ul > 0);
  sl = 53;
  ASSERT (-sl < 0);

  ASSERT (-(53  ) < 0);
  ASSERT (-(53u ) > 0);
  ui = 53;
  ASSERT (-ui > 0);
  si = 53;
  ASSERT (-si < 0);

  ASSERT (-( 250 ) == -250);
  uc = 250;
  ASSERT (-uc == -250);

  ASSERT (-(-128 ) ==  128);
  sc = -128;
  ASSERT (-sc == 128);
}

void
testBug1571231 (void)
{
  unsigned char  {attrL} uc;

  /* bug-1571231 */
  uc = 0x80;
  if (uc + 0x80)
    ASSERT (1);
  else
    ASSERT (0);
}
