/*
  bug-2349.c
*/

#include <testfwk.h>

#pragma disable_warning 85

long subchar(void *pa, void *pb)
{
  return *((char *) pa) - *((char *) pb);
}

long subint(void *pa, void *pb)
{
  return *((int *) pa) - *((int *) pb);
}

long sublong(void *pa, void *pb)
{
  return *((long *) pa) - *((long *) pb);
}

long addlong(void *pa, void *pb)
{
  return *((long *) pa) + *((long *) pb);
}

#define NULL ((void *) 0)

#if defined (__SDCC_MODEL_MEDIUM)
typedef long (*subfunc_t)(void *pa, void * __pdata pb);
#elif defined (__SDCC_MODEL_LARGE)
typedef long (*subfunc_t)(void *pa, void * __xdata pb);
#elif defined (__SDCC_MODEL_HUGE)
typedef long (*subfunc_t)(void *pa, void * __xdata pb);
#else
typedef long (*subfunc_t)(void *pa, void *pb);
#endif

subfunc_t dosub(subfunc_t f)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  if (f == subchar)
    return subint;
  else if (f == subint)
    return sublong;
  else if (f == sublong)
    return subchar;
  else if (f == NULL)
    return addlong;
  else
    return NULL;
#endif
}

void testBug(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15)
  ASSERT (dosub(subchar) == subint);
  ASSERT (dosub(subint) == sublong);
  ASSERT (dosub(sublong) == subchar);
  ASSERT (dosub(addlong) == NULL);
  ASSERT (dosub(NULL) == addlong);
#endif
}
