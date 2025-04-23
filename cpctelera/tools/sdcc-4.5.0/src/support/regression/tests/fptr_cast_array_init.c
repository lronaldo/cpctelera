/** Test type cast with function pointers in array initialization.
 */

#include <testfwk.h>

#ifdef __SDCC
/* don't spam the output with "pointer types incompatible" warnings */
#pragma disable_warning 244
#endif

void a(void) {}

typedef void (*fp)(void);

void testFptrCastOld(void)
{
  /* old functionality */
  fp tab1[2] = {a, 0};
  ASSERT(tab1[0] == a);
  fp tab2[2] = {&a, 0};
  ASSERT(tab2[0] == a);
  fp tab3[2] = {(fp)a, 0};
  ASSERT(tab3[0] == a);
  fp tab4[2] = {(fp)&a, 0};
  ASSERT(tab4[0] == a);
  /* sizeof(void *) < sizeof(fp) implies undefined behavior,
   * i.e. the result of the comparison does not matter */
  void * tab5[2] = {a, 0};
  ASSERT(sizeof(void *) < sizeof(fp) || ((fp)tab5[0]) == a);
  void * tab6[2] = {&a, 0};
  ASSERT(sizeof(void *) < sizeof(fp) || ((fp)tab6[0]) == a);
}

void testFptrCastNew(void)
{
  /* new functionality */
  void * tab1[2] = {(void*)a, 0};
  ASSERT(sizeof(void *) < sizeof(fp) || ((fp)tab1[0]) == a);
  void * tab2[2] = {(void*)&a, 0};
  ASSERT(sizeof(void *) < sizeof(fp) || ((fp)tab2[0]) == a);
  char * tab3[2] = {(char*)a, 0};
  ASSERT(sizeof(char *) < sizeof(fp) || ((fp)tab3[0]) == a);
  char * tab4[2] = {(char*)&a, 0};
  ASSERT(sizeof(char *) < sizeof(fp) || ((fp)tab4[0]) == a);
}
