/* Test conditional operator with pointer operands
 */
/* In addition to the usual promotion rules that apply to the operands */
/* to all operators, the conditional operator has some special rules   */
/* that apply to pointer types (as well as arrays, since an array      */
/* reference is interchangable with a pointer to the first element):   */
/*   1. If the operands are a pointer and NULL, the resultant type     */
/*      is that of the non-NULL pointer. NULL can be defined as either */
/*      0 or (void *)0.                                                */
/*   2. If the operands are a non-void pointer and a void (non-NULL)   */
/*      pointer, the resultant type is that of the void pointer.       */
#include <testfwk.h>

int testarray[5] = {3,2,7,6,8};

unsigned char cond;

void
setcond(unsigned char state)
{
  cond = state;
}

int
deref1(int *ip)
{
  return *ip;
}

int
deref2(char *cp)
{
  int *ip = (void *)cp;
  return *ip;
}


void
testCondOpPtrTypes1(void)
{
  int *ip1;
  int *ip2;
  void *vp1;
  void *vp2;
  char *cp;

  
  ip1 = &(testarray[2]);

  setcond(1);
  
  ip2 = cond ? testarray : ip1;    /* int[], int *  ==> int *  */
  ASSERT (*ip2 == 3);
  ip2 = cond ? ip1 : testarray;    /* int *, int[]  ==> int *  */
  ASSERT (*ip2 == 7);

  ip2 = cond ? testarray : 0;      /* int[], 0      ==> int *  */
  ASSERT (*ip2 == 3);
  ip2 = cond ? ip1 : 0;            /* int *, 0      ==> int *  */
  ASSERT (*ip2 == 7);

  ip2 = cond ? testarray : (void *)0;  /* int[], NULL   ==> int *  */
  ASSERT (*ip2 == 3);
  ip2 = cond ? ip1 : (void *)0;        /* int *, NULL   ==> int *  */
  ASSERT (*ip2 == 7);

  vp1 = &(testarray[1]);
  ip2 = testarray;
  vp2 = cond ? testarray : vp1;        /* int[], void * ==> void * */
  ASSERT (vp2 == ip2);
  vp2 = cond ? ip1 : vp1;              /* int *, void * ==> void * */
  ASSERT (vp2 == ip1);
  cp = cond ? testarray : vp1;         /* int[], void * ==> void * */
  ASSERT ((int *)cp == ip2);
  cp = cond ? ip1 : vp1;               /* int *, void * ==> void * */
  ASSERT ((int *)cp == ip1);

  ASSERT (deref1 (cond ? testarray : ip1) == 3);
  ASSERT (deref1 (cond ? ip1 : testarray) == 7);
  ASSERT (deref1 (cond ? testarray : 0) == 3);
  ASSERT (deref1 (cond ? ip1 : 0) == 7);
  ASSERT (deref1 (cond ? testarray : (void *)0) == 3);
  ASSERT (deref1 (cond ? ip1 : (void *)0) == 7);

  /* These resolve to type void * and so are compatible with */
  /* the char * parameter */
  ASSERT (deref2 (cond ? testarray : vp1) == 3);
  ASSERT (deref2 (cond ? ip1 : vp1) == 7);
}

void
testCondOpPtrTypes2(void)
{
  int *ip1;
  int *ip2;
  void *vp1;
  void *vp2;
  char *cp;
  
  ip1 = &(testarray[2]);

  setcond(0);
  
  ip2 = cond ? testarray : ip1;    /* int[], int *  ==> int *  */
  ASSERT (*ip2 == 7);
  ip2 = cond ? ip1 : testarray;    /* int *, int[]  ==> int *  */
  ASSERT (*ip2 == 3);

  ip2 = cond ? 0 : testarray;      /* 0, int[]      ==> int *  */
  ASSERT (*ip2 == 3);
  ip2 = cond ? 0: ip1;             /* 0, int *      ==> int *  */
  ASSERT (*ip2 == 7);

  ip2 = cond ? (void *)0 : testarray;  /* NULL, int[]   ==> int *  */
  ASSERT (*ip2 == 3);
  ip2 = cond ? (void *)0 : ip1;        /* NULL, int *   ==> int *  */
  ASSERT (*ip2 == 7);

  vp1 = &(testarray[1]);
  ip2 = testarray;
  vp2 = cond ? vp1 : testarray;        /* void *, int[] ==> void * */
  ASSERT (vp2 == ip2);
  vp2 = cond ? vp1 : ip1;              /* void *, int * ==> void * */
  ASSERT (vp2 == ip1);
  cp = cond ? vp1 : testarray;         /* void *, int[] ==> void * */
  ASSERT ((int *)cp == ip2);
  cp = cond ? vp1 : ip1;               /* void *, int * ==> void * */
  ASSERT ((int *)cp == ip1);

  ASSERT (deref1 (cond ? testarray : ip1) == 7);
  ASSERT (deref1 (cond ? ip1 : testarray) == 3);
  ASSERT (deref1 (cond ? 0 : testarray) == 3);
  ASSERT (deref1 (cond ? 0 : ip1) == 7);
  ASSERT (deref1 (cond ? (void *)0 : testarray) == 3);
  ASSERT (deref1 (cond ? (void *)0 : ip1) == 7);

  /* These resolve to type void * and so are compatible with */
  /* the char * parameter */
  ASSERT (deref2 (cond ? vp1 : testarray) == 3);
  ASSERT (deref2 (cond ? vp1 : ip1) == 7);
}

int
foo0(int a, int b, int c)
{
  return a > 10 ? ++b, ++c : b + c;
}

int
foo1(int a, int b, int c, int d)
{
  return a > 20 ? ++b, ++c, ++d + b + c : b < 100 ? c + ++d : c - --d;
}

void
testBug2412(void)
{
  ASSERT (foo0 (2, 4, 5) == 9);
  ASSERT (foo0 (72, 84, 5) == 6);
  ASSERT (foo0 (7, 84, 75) == 159);

  ASSERT (foo1 (110, 12, 13, 15) == 43);
  ASSERT (foo1 (18, 12, 13, 15) == 29);
  ASSERT (foo1 (12, 129, 13, 13) == 1);
}
