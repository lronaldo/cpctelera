/** Absolute addressing tests.

    mem: __code, __xdata
*/
#include <testfwk.h>
#include <stddef.h>

#ifdef __SDCC_pic16
# define ADDRESS(x) (0x02 ## x)
#elif defined(__SDCC_pic14)
# define ADDRESS(x) (0x01 ## x)
#elif defined(__SDCC_stm8)
# define ADDRESS(x) (0x10 ## x)
#else
# define ADDRESS(x) (0xCA ## x)
#endif

typedef struct
{
  int a, b;
} TestStruct;

{mem} volatile __at(ADDRESS(BC)) TestStruct TestVar = {0,0};
{mem} __at(ADDRESS(B7)) char u;
{mem} __at(ADDRESS(B7)) char x = 'x';
{mem} __at(ADDRESS(B9)) char y = 'y';
{mem} __at(ADDRESS(B0)) int  k = 0x1234;

char * {mem} __at(ADDRESS(A0)) pc = NULL;

typedef char *Pointer;
{mem} Pointer __at(ADDRESS(A4)) p = NULL;

typedef char Array[2];
{mem} Array __at(ADDRESS(A8)) a = {0};

typedef void (* const Function)(void);
{mem} Function __at(ADDRESS(AC)) f = NULL;

char z = 'z';

void
testAbsolute(void)
{
#if !defined(PORT_HOST)
  static {mem} __at(ADDRESS(B6)) char s = 's';
  char {mem} *pC = (char {mem} *)(ADDRESS(B0));
  int  {mem} *pI = (int  {mem} *)(ADDRESS(B0));

  ASSERT(u == 'x');
  ASSERT(pC[7] == 'x');
  ASSERT(pC[9] == 'y');
  ASSERT(pC[6] == 's');
  ASSERT(pI[0] == 0x1234);

  ASSERT((unsigned int)&pc==ADDRESS(A0));
  ASSERT((unsigned int)&p==ADDRESS(A4));
  ASSERT((unsigned int)&a==ADDRESS(A8));
  ASSERT((unsigned int)&f==ADDRESS(AC));

  //bug 2941749
  ASSERT((*((TestStruct *)&k)).a == 0x1234);
#endif
}

#if defined(__SDCC_mcs51) || defined(__SDCC_ds390)
volatile __data __at(0x20) unsigned char Byte0 = 0x00;
volatile __data __at(0x22) unsigned char Byte1 = 0x00;
volatile __bit Bit0, Bit1, Bit2, Bit3, Bit4, Bit5, Bit6, Bit7, Bit8;
#endif

void
testAbsBdata(void)
{
#if defined(__SDCC_mcs51) || defined(__SDCC_ds390)
  Bit0 = 1;
  ASSERT(Byte0 == 0x00);
  Byte0 = 0xFF;
  Bit0 = 0;
  ASSERT(Byte0 == 0xFF);
#endif
}

