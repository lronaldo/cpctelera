/* Test CSE - make sure CSE does not use stale data

 */

#include <testfwk.h>

/* Need to locate a variable at an absolute address, but without */
/* the linker's help. Pick an address and address space that is  */
/* likely free based on cpu and compile model. If ABSADDR is     */
/* left undefined, the tests that needs it will be skipped.      */
#if defined(SDCC) || defined(__SDCC)
#  if  defined(SDCC_mcs51) || defined(__SDCC_mcs51)
#    if defined(SDCC_MODEL_LARGE) || defined(__SDCC_MODEL_LARGE)
#      define ABSADDRSPACE __xdata
#      define ABSADDR 0x8000
#    else
#      define ABSADDRSPACE __data
#      define ABSADDR 0x70
#    endif
#  endif
#  if defined(SDCC_ds390) || defined(__SDCC_ds390)
#    define ABSADDRSPACE __xdata
#    define ABSADDR 0x8000
#  endif
#  if defined(SDCC_hc08) || defined(__SDCC_s08)
#    define ABSADDRSPACE __xdata
#    define ABSADDR 0x7f00
#  endif
#  if defined(SDCC_z80) || defined(__SDCC_z80)
#    define ABSADDRSPACE __xdata
#    define ABSADDR 0x7f00
#  endif
#endif


typedef struct {
  unsigned char x1;
  unsigned char x2;
  unsigned char *p;
} packet_t;

unsigned char gx;
unsigned char *px;

packet_t gpacket;
#ifdef ABSADDR
# define apacket1 (*((ABSADDRSPACE packet_t *)ABSADDR))
ABSADDRSPACE packet_t __at ABSADDR apacket2;
#endif

unsigned char a1,a2,a3,a4,a5;

void
setgx(unsigned char x)
{
  gx = x;
}

unsigned char 
passthrough(unsigned char i)
{
  return i;
}

void
zeroAndKeepAddress(unsigned char *p)
{
  px = p;
  *p = 0;
}

void
keepAddressFromPacket(packet_t *pp)
{
  px = pp->p;
}

void
incIndirect(void)
{
  (*px)++;
}

packet_t *
getaddrgpacket(void)
{
  return &gpacket;
}

void
incx1(packet_t *pp)
{
  pp->x1++;
}

void
incx2(packet_t *pp)
{
  pp->x2++;
}

void
incgpx1(void)
{
  gpacket.x1++;
}

/* Be sneaky and define this later so that the addrtaken flag of gx is not  */
/* set until after the test functions are compiled. Not important now, but  */
/* this should remove the temptation to misuse reliance on addrtaken later. */
unsigned char * getaddrgx(void);


/* The ASSERT macro, since it branches based on a condition, will break up a */
/* extended basic block. So to test for CSE problems within a single EBBlock */
/* we will save intermediate results in global variables a1..a5 and do all   */
/* of the ASSERTs at the end of the function. For some tests it may make     */
/* more sense to use local variables, but I'm intentionally trying to keep   */
/* register pressure low. */

void
test_FuncCall1(void)
{
  unsigned char lx;
  
  gx = 1;
  setgx(2);
  /* Cannot CSE global variable assignment across function call */
  a1 = gx; /* ASSERT(gx == 2) */

  lx = gx;
  setgx(3);
  /* Cannot CSE global variable read across function call */
  a2 = gx; /* ASSERT(gx == 3) */
  a3 = lx; /* ASSERT(lx == 2) */ 
 
  lx = gx<<1;
  a4 = lx; /* ASSERT(lx == 6) */
  setgx(1);
  lx = gx<<1;
  /* Cannot CSE expression based on global variable across function call */
  a5 = lx; /* ASSERT(lx == 2) */

  ASSERT(a1 == 2);
  ASSERT(a2 == 3);
  ASSERT(a3 == 2);
  ASSERT(a4 == 6);
  ASSERT(a5 == 2);
}

void
test_FuncCall2(void)
{
  unsigned char lx,ly;

  lx = passthrough(2);
  ly = lx<<1;
  zeroAndKeepAddress(&lx);
  
  /* Cannot CSE local variable if a pointer to it was passed as parameter */
  a1 = lx; /* ASSERT(lx == 0) */
  
  incIndirect();
  /* Cannot CSE local variable across function call even if its address */
  /* was not a parameter if its address was taken at all */ 
  a2 = lx; /* ASSERT(lx == 1) */


  a3 = ly; /* ASSERT(ly == 4) */
  ly = lx<<1;
  /* Cannot CSE expression based on local variable whose address was */
  /* taken across function call */
  a4 = ly; /* ASSERT(ly == 2) */

  ASSERT(a1 == 0);
  ASSERT(a2 == 1);
  ASSERT(a3 == 4);
  ASSERT(a4 == 2);
}

void
test_FuncCall3(void)
{
  unsigned char lx,ly;

  lx = passthrough(2);
  ly = lx<<1;
  px = &lx;
  incIndirect();
  /* Cannot CSE local variable across function call if address was taken at all */ 
  a1 = lx; /* ASSERT(lx == 3) */

  a2 = ly; /* ASSERT(ly == 4) */
  ly = lx<<1;
  /* Cannot CSE expression based on local variable whose address was */
  /* taken across function call */
  a3 = ly; /* ASSERT(ly == 6) */

  ASSERT(a1 == 3);
  ASSERT(a2 == 4);
  ASSERT(a3 == 6);
}

void
test_FuncCall4(void)
{
  packet_t packet1,packet2;
  
  packet1.p = &packet2.x1;
  keepAddressFromPacket(&packet1);
  packet2.x1 = 1;
  packet2.x2 = 3;
  incIndirect();
  /* Cannot CSE members of local variables whose address was taken */
  /* across function call */
  a1 = packet2.x1; /* ASSERT(packet2.x1 == 2) */
  a2 = packet2.x2; /* ASSERT(packet2.x2 == 3) */
  
  ASSERT(a1 == 2);
  ASSERT(a2 == 3);
}

void
test_FuncCall5(void)
{
  packet_t *pp;
  unsigned char i;

  gpacket.x1 = 1;
  gpacket.x2 = 10;
  pp = getaddrgpacket();
  
  i = pp->x1;
  a1 = i; /* ASSERT(i == 1) */
  incx1(pp);
  i = pp->x1;
  a2 = i; /* ASSERT(i == 2) */
  i = pp->x2;
  a3 = i; /* ASSERT(i == 10) */
  incx2(pp);
  i = pp->x2;
  a4 = i; /* ASSERT(i == 11) */

  ASSERT(a1 == 1);
  ASSERT(a2 == 2);
  ASSERT(a3 == 10);
  ASSERT(a4 == 11);
}

void
test_FuncCall6(void)
{
  unsigned char lx;

  lx = passthrough(1);
  gpacket.x1 = lx;
  incgpx1();
  lx = gpacket.x1;
  ASSERT(lx == 2);
}

/* test_Struct1 through test_Struct4 are checking for cases in which */
/* operand keys are erroneously equal or unassigned, causing CSE to  */
/* use the wrong substitution. */
void
test_Struct1(void)
{
  packet_t packet;
  unsigned char one;
  unsigned char ten;
  
  one = passthrough(1);
  ten = passthrough(10);
  
  gx = 100;
  packet.x1 = one;
  packet.x2 = ten;
  setgx(packet.x2);
  /* CSE of "apacket.x2" allowed above , but make sure it was x2 not x1*/
  ASSERT(gx == 10);
}

void
test_Struct2(void)
{
  unsigned char one;
  unsigned char ten;
  
  one = passthrough(1);
  ten = passthrough(10);
  
  gx = 100;
  gpacket.x1 = one;
  gpacket.x2 = ten;
  setgx(gpacket.x2);
  /* CSE of "apacket.x2" allowed above , but make sure it was x2 not x1*/
  ASSERT(gx == 10);
}

void
test_Struct3(void)
{
#ifdef ABSADDR
  unsigned char one;
  unsigned char ten;
  
  one = passthrough(1);
  ten = passthrough(10);
  apacket1.x2 = ten;
  
  setgx(100);
  apacket1.x1 = one;
  setgx(apacket1.x2);
  /* CSE must not substitute x1 in place of x2 */
  ASSERT(gx == 10);
#endif
}

void
test_Struct4(void)
{
#ifdef ABSADDR
  unsigned char one;
  unsigned char ten;
  
  one = passthrough(1);
  ten = passthrough(10);
  apacket2.x2 = ten;
  
  setgx(100);
  apacket2.x1 = one;
  setgx(apacket2.x2);
  /* CSE must not substitute x1 in place of x2 */
  ASSERT(gx == 10);
#endif
}

void
test_Struct5(void)
{
/* ASSERT(a2 == 2) currently fails in this test (2012-02-29) */
#if 0
  packet_t  *pp1;
  packet_t  *pp2;
  unsigned char lx;
  
  pp1 = getaddrgpacket();
  pp2 = getaddrgpacket();

  /* Since the declarations of pp1 and pp2 did not use the restrict */
  /* qualifier, the compiler must assume they might point to the    */
  /* same object and so should not CSE pointer dereferences across  */
  /* other pointer dereferenced assingments of the same type. */
  gpacket.x1 = 0;
  pp1->x1 = 1;
  lx = pp2->x1;
  a1 = lx;
  
  pp1->x1 = 2;
  lx = pp2->x1;
  a2 = lx;
  
  pp2->x1 = 3;
  lx = pp1->x1;
  a3 = lx;

  ASSERT(a1 == 1);
  ASSERT(a2 == 2);
  ASSERT(a3 == 3);
#endif
}

void
test_Global1(void)
{
/* ASSERT(a2 == 2) currently fails in this test (2012-02-29) */
#if 0
  unsigned char lx;
  unsigned char *px;

  /* Since the declaration of px did not use the restrict */
  /* qualifier, the compiler must assume it might point   */
  /* to a global variable and so should not CSE any       */
  /* read of or expression using a global variable.       */
  gx = 0;
  px = getaddrgx();
  lx = gx<<1;
  a1 = lx;
  *px = 1;
  lx = gx<<1;
  a2 = lx;
  
  ASSERT(a1 == 0);
  ASSERT(a2 == 2);
#endif
}

/* Hide taking the address of gx by defining this function last */
unsigned char
*getaddrgx(void)
{
  return &gx;
}
