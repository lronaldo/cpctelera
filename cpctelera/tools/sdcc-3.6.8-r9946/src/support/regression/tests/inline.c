/** inline tests
*/

#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

/*--------------
    bug 2450
*/

static inline int bug2450isdigit (int c)
{
  return ((unsigned char)c >= (char)'0' && (unsigned char)c <= (char)'9');
}

/* Extracted from atof(). Using an inlined function in an expression */
/* in a for-statment would leave some of the intermediate variables  */
/* created for the inlining unallocated and thus missing an output   */
/* storage class, which would then cause a fatal error for the code  */
/* generator. */
void bug2450(const char *s)
{
    for (; bug2450isdigit(*s););
}


/*--------------
    bug 1717305
*/

static inline int
f (const int a)
{
  return (a + 3);
}

int g (int b)
{
  return (f (b));
}

void
bug_1717305 (void)
{
  int x = 0;
  ASSERT (g (x) == 3);
}

/*--------------
    bug 1767885
*/

static inline int
f1 (int x)
{
  int b = 6;
  return x + b;
}

static inline int
f2 (int x)
{
  int y = f1 (x);
  return y;
}

static inline int
f3 (int x)
{
  int y;
  y = f1 (x);
  return y;
}

static inline int
g_for (int b)
{
  int a = 10 + b * 2;
  for (; b > 0; b--)
    {
      if (a > 1000)
        return a;
    }
  return a;
}

static inline int
f_for (int c)
{
  int i;
  for (i = c; i > 0; i--)
    c += i * g_for (i);
  return c;
}

int gi = 6;
int g_a;
int g_y;

void
bug_1767885 (void)
{
  gi = f2 (gi);
  ASSERT (gi == 12);

  gi = f3 (gi);
  ASSERT (gi == 18);

  g_y = f_for (g_a);
  g_y = g_for (g_y);
}

/*--------------
    bug 1864577
*/
typedef unsigned char uint8_t;

typedef uint8_t error_t;

enum __nesc_unnamed4247
  {
    SUCCESS = 0,
    FAIL = 1,
    ESIZE = 2,
    ECANCEL = 3,
    EOFF = 4,
    EBUSY = 5,
    EINVAL = 6,
    ERETRY = 7,
    ERESERVE = 8,
    EALREADY = 9
  };

static inline error_t
PlatformP__LedsInit__default__init (void)
{
  return SUCCESS;
}

static error_t
PlatformP__LedsInit__init (void);

static inline error_t
PlatformP__LedsInit__init (void)
{
  unsigned char result;

  result = PlatformP__LedsInit__default__init ();

  return result;
}

void
bug_1864577 (void)
{
  ASSERT (PlatformP__LedsInit__init () == SUCCESS);
}

/* inline definition seems to be broken on NetBSD GCC 4.1.3 and Mac OS X GCC 4.0.1
   it gets external linkage where it should not */
#if (defined(__NetBSD__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 1) && (__GNUC_PATCHLEVEL__ == 3))
#define SKIP_EXTERNAL
#endif
#if (defined(__APPLE__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 0) && (__GNUC_PATCHLEVEL__ == 1))
#define SKIP_EXTERNAL
#endif
#if (defined(__OpenBSD__) && (__GNUC__ == 4) && (__GNUC_MINOR__ == 2) && (__GNUC_PATCHLEVEL__ == 1))
#define SKIP_EXTERNAL
#endif

#ifdef SKIP_EXTERNAL
#warning inline definition skipped
#else
/*--------------
    inline definition with external linkage
    the corresponding external definition is in fwk/lib/extern1.c
*/
inline char inlined_function (void)
{
  return 1;
}

/*  function pointer defined in fwk/lib/extern2.c initialized to the
    externally defined inlined_function */
extern char (*inlined_function_pointer) (void);

/*--------------
    bug 2591
    inline definition with parameters not in registers
	these parameters should not be allocated here
    the corresponding external definition is in fwk/lib/extern1.c
*/
inline long bug_2591 (long a, long b, long c)
{
  return a | b | c;
}
#endif

/*--------------
    bug 3564755
*/

static char a_3564755;
static char b_3564755;

#if defined(__SDCC_mcs51) || defined(__SDCC_ds390)
#define BIT_T __bit
#else
#define BIT_T char
#endif

static inline BIT_T
condition_func()
{
  return (a_3564755 == b_3564755);
}

void
bug_3564755 (void)
{
  a_3564755 = 1;
  b_3564755 = 250;

  while (!condition_func()) /* inlined function returning bit caused segfault */
    {
      b_3564755 += 1;
    }
  ASSERT(a_3564755 == b_3564755);
}

/*--------------*/

/*--------------
    bug 2295
*/

void
bug_2295 (void)
{
  char x = 0, y = 0, z = 0;
  for (x = inlined_function(); inlined_function() - z; y += inlined_function())
    {
      z += inlined_function();
    }
  ASSERT (x == 1);
  ASSERT (y == 1);
  ASSERT (z == 1);
}

/*--------------*/

void
testInline (void)
{
#ifndef SKIP_EXTERNAL
  char x = inlined_function(); /* can use the inlined or the external implementation */
  ASSERT (x == 1 || x == 2);
  ASSERT (inlined_function_pointer() == 2); /* must use the external one */
#endif
  bug_1717305 ();
  bug_1767885 ();
  bug_1864577 ();
  bug_3564755 ();
#ifndef SKIP_EXTERNAL
  bug_2295 ();
#endif
}

