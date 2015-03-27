/* Bad mangaling of support names.
 */
#include <testfwk.h>

/* The original bug */
float z1(void)
{
  return 5;
}

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

/* Tests to check basic conversion */
void
testfs2long(void)
{
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
}
