/* Bad mangaling of support names.
 */
#include <testfwk.h>

/* The original bug */
float z1(void)
{
  return 5;
}

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
float fun( void ) 
{ 
  unsigned long i; 
  float f; 
  i=5.5 * z1(); 
  f=i; 
  if (i & 1) 
    f += 1.0; 
  return f; 
}
#endif

/* Tests to check basic conversion */
void
testfs2long(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  volatile float f;
  volatile unsigned long ul;
  volatile long l;

  f = 5.0;
  ul = f;
  ASSERT(ul == 5);

  l = f;
  ASSERT(l == 5);

  f = -134;
  l = f;
  ASSERT(l == -134);

  l = 4567;
  f = l;
  ASSERT(f == 4567.0);

  l = -1539;
  f = l;
  ASSERT(f == -1539.0);

  ul = 9995;
  f = ul;
  ASSERT(f == 9995.0);
#endif
}
