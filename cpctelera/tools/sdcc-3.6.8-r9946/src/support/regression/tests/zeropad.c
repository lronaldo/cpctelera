/** Zeropad tests.

    storage: auto, __idata, __pdata, __xdata, __code,
*/
#ifndef STORAGE
#define STORAGE_{storage}
#define STORAGE {storage}
#endif

#if defined (__GNUC__) && defined (__alpha__) && (__GNUC__ < 3)
  /* since g fails on GCC 2.95.4 on alpha... */
  #define FLEXARRAY 0
  #define TEST_G    0
#elif defined (STORAGE_auto)
  /* only static flexible arrays are allowed */
  #define FLEXARRAY 0
  #define TEST_G    1
#else
  #define FLEXARRAY 1
  #define TEST_G    1
#endif

#include <testfwk.h>
#include <stddef.h>

#if defined (STORAGE_auto)
  void Zeropad(void)
  {
#else
  extern char STORAGE bug1470790[3];
         char STORAGE bug1470790[] = {1, };

#endif //STORAGE_auto

const char *string1 = "\x00\x01";
const char string2[] = "\x00\x01";

#ifndef PORT_HOST
#pragma disable_warning 147 //no warning about excess initializers (W_EXCESS_INITIALIZERS)
#pragma disable_warning  85 //no warning about unreferenced variables (W_NO_REFERENCE)
//array will be truncated but warning will be suppressed
//if truncation is incorrect, other ASSERTs will fail with high probability
char STORAGE trunc[2] = {'a', 'b', 'c'};
#endif

char STORAGE array[5] = {'a', 'b', 'c'};

#if TEST_G
  struct w {
    char a;
    int  b;
  } STORAGE g[3] = {
    {'x', 1},
    {'y'},
    {'z', 3}
  };
#endif

struct x {
  short a;
  char  b[10];
};

struct x STORAGE teststruct[5] = {
  { 10, {  1, 2, 3, 4, 5} },
  { 20, { 11 } },
  { 30, {  6, 7, 8} }
};

#if FLEXARRAY
  struct y {
    short a;
    char  b[];
  };

struct y STORAGE incompletestruct = {
  10, {1, 2, 3, 4, 5}
};
#endif

struct z {
  short a;
  void (*fp)(void);
};

//see bug 2881971
struct z STORAGE funcptrstruct = {
  10
};

#if !defined (STORAGE_auto)
void
Zeropad (void)
{
  ASSERT (bug1470790[0] == 1);
  ASSERT (bug1470790[1] == 0);
#endif //STORAGE_auto

  ASSERT (string1[0] == '\x00');
  ASSERT (string1[1] == '\x01');
  ASSERT (string2[0] == '\x00');
  ASSERT (string2[1] == '\x01');

  ASSERT (array[2] == 'c');
  ASSERT (array[4] == 0);

#if TEST_G
  ASSERT (g[1].a == 'y');
  ASSERT (g[1].b == 0);
  ASSERT (g[2].a == 'z');
  ASSERT (g[2].b == 3);
#endif

  ASSERT (teststruct[0].b[1] ==  2);
  ASSERT (teststruct[0].b[5] ==  0);
  ASSERT (teststruct[1].b[0] == 11);
  ASSERT (teststruct[4].b[9] ==  0);

  ASSERT (sizeof(teststruct[2].a) ==  2);
  ASSERT (sizeof(teststruct[1].b) == 10);
  ASSERT (sizeof(teststruct[1])   == 12);
  ASSERT (sizeof(teststruct)      == 60);

#if FLEXARRAY
  ASSERT (incompletestruct.a    == 10);
  ASSERT (incompletestruct.b[0] ==  1);
  ASSERT (incompletestruct.b[4] ==  5);

  ASSERT (sizeof (incompletestruct) == sizeof (struct y));
  ASSERT (sizeof (incompletestruct) == offsetof (struct y, b));
  ASSERT (sizeof (incompletestruct) == offsetof (struct x, b));
#endif

#if defined (STORAGE_auto)
  array[4] = 1;
#if TEST_G
  g[1].b = 1;
#endif
  teststruct[0].b[5] = 1;
  teststruct[4].b[9] = 1;
#endif //STORAGE_auto
}

void
testZeropad (void)
{
  Zeropad ();

#if defined (STORAGE_auto)
  Zeropad (); //test reinitialization
#endif //STORAGE_auto
}
