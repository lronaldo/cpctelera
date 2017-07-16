/* Test CSE with |&^

    type: bool, char, short, long
 */
#include <testfwk.h>

#ifdef __SDCC
#pragma std_sdcc99
#endif

#include <stdbool.h>

/* This is not only a regression test, the focus of this test
   is more on the generated code (volatile!). */

#define TYPE_{type}

#if defined(TYPE_bool)
#  define UNSIGNED
#else
#  define UNSIGNED unsigned
#endif

#if defined(TYPE_bool)
#  define _data
#else
#  define _data __idata
#endif

#if defined(TYPE_bool) && defined(__bool_true_false_are_defined)
#  define MASK 1
#elif defined(TYPE_bool) && !defined(__bool_true_false_are_defined)
#  if defined(PORT_HOST)
#    define MASK 0xffffffff
#  else
#    define MASK 0xffff
#  endif
#  define bool int
#elif defined(TYPE_char)
#  define MASK 0xff
#elif defined(TYPE_short)
#  define MASK 0xffff
#elif defined(TYPE_long)
#  define MASK 0xffffffff
#else
#  warning Unknown type
#endif

/* the variable 'mask' is only defined to see if MASK is correctly set up */
const unsigned long mask = MASK;

      volatile          {type}  v;
      volatile UNSIGNED {type} uv;
/* an array would be nicer, but an array of bits isn't possible */
_data                   {type}  a0 , a1 , a2 , a3 , a4 , a5 , a6 , a7 , a8 , a9 ,
                                a10, a11, a12, a13, a14, a15, a16, a17, a18, a19,
                                a20;
_data          UNSIGNED {type} ua0, ua1, ua2, ua3, ua4, ua5, ua6;
_data                   {type}  b;
_data volatile UNSIGNED {type} ub = 0xbe;

void
testcse(void)
{
   b = 0xeb;
  ub = 0xbe;
   v = 0x33;
  uv = 0x7b;

   a0   =    0 & b;
   a1   =    0 & v;

   a2   = MASK &  b;
   a3   = MASK &  v;
  ua0   = MASK & ub;
  ua1   = MASK & uv;

   a4   =    b & b;
   a5   =    v & v;

   a6  &= 0;
   v   &= 0;
   a7  &= MASK;
   v   &= MASK;
  ua2  &= MASK;
  uv   &= MASK;


   a8   =    0 | b;
   a9   =    0 | v;

   a10  = MASK |  b;
   a11  = MASK |  v;
  ua3   = MASK | ub;
  ua4   = MASK | uv;

   a12  =   b | b;
   a13  =   v | v;

   a14 |= 0;
   v   |= 0;
   a15 |= MASK;
   v   |= MASK;
  ua5  |= MASK;
  uv   |= MASK;


   a16  = 0 ^ b;
   a17  = 0 ^ v;

   a18  = b ^ b;
   a19  = v ^ v;

   a20 ^= 0;
   v   ^= 0;

   ASSERT( a0  ==  0);
   ASSERT( a1  ==  0);
   ASSERT( a2  ==  b);
#if defined(TYPE_bool) && defined(__bool_true_false_are_defined)
   ASSERT( a3  == 1);
   ASSERT(ua1  == 1);
   ASSERT( a5  == 1);
   ASSERT(  b  == 1);
   ASSERT( ub  == 1);
   ASSERT(ua0  == 1);
   ASSERT( a4  == 1);
#else
   ASSERT( a3  == ({type}) 0x33);
   ASSERT(ua1  == ({type}) 0x7b);
   ASSERT( a5  == ({type}) 0x33);
#endif
   ASSERT(ua0  == ub);
   ASSERT( a4  ==  b);
   ASSERT( a6  ==  0);
   // ASSERT( a7 == );
   // ASSERT(ua2 == );
   ASSERT( a8  ==  b);
   ASSERT( a9  ==  0);
   ASSERT( a10 == ({type}) MASK);
   ASSERT( a11	== ({type}) MASK);
   ASSERT(ua3  == MASK);
   ASSERT(ua4  == MASK);
   ASSERT( a12 ==  b);
   ASSERT( a13 ==  0);
   // ASSERT( a14 == );
   ASSERT( a15 == ({type}) MASK);
   ASSERT(ua5 == MASK);
   ASSERT( a16 == b);
   ASSERT( a17 == ({type}) MASK);
   ASSERT( a18 == 0);
   ASSERT( a19 == 0);
   // ASSERT( a20 == );
   ASSERT( v   == ({type}) MASK);
   ASSERT(uv   == MASK);
}

