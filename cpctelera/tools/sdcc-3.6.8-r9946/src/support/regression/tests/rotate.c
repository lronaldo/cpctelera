/** Tests covering rotate operations
    size: 8,16,32
    msb: 0,1
*/
#include <testfwk.h>
#ifdef __sun__
#include <inttypes.h>
#else
#include <stdint.h>
#endif

#define SIZE ({size})
#define MSB ({msb})

#if SIZE == 8
#  define TYPE uint8_t
#  if MSB
#    define TEST_VECT 0xa4
#  else
#    define TEST_VECT 0x53
#  endif
#endif

#if SIZE == 16
#  define TYPE uint16_t
#  if MSB
#    define TEST_VECT 0xa8ce
#  else
#    define TEST_VECT 0x5357
#  endif
#endif

#if SIZE == 32
#  define TYPE uint32_t
#  if MSB
#    define TEST_VECT 0xa8c5a5c6
#  else
#    define TEST_VECT 0x55357553
#  endif
#endif

TYPE rol1(TYPE s){ return (s<<1) | (s>>(SIZE-1)); }
TYPE rol2(TYPE s){ return (s<<2) | (s>>(SIZE-2)); }
TYPE rol3(TYPE s){ return (s<<3) | (s>>(SIZE-3)); }
TYPE rol4(TYPE s){ return (s<<4) | (s>>(SIZE-4)); }
TYPE rol5(TYPE s){ return (s<<5) | (s>>(SIZE-5)); }
TYPE rol6(TYPE s){ return (s<<6) | (s>>(SIZE-6)); }
TYPE rol7(TYPE s){ return (s<<7) | (s>>(SIZE-7)); }

#if SIZE >=16
TYPE rol8 (TYPE s){ return (s<<8 ) | (s>>(SIZE-8 )); }
TYPE rol9 (TYPE s){ return (s<<9 ) | (s>>(SIZE-9 )); }
TYPE rol10(TYPE s){ return (s<<10) | (s>>(SIZE-10)); }
TYPE rol11(TYPE s){ return (s<<11) | (s>>(SIZE-11)); }
TYPE rol12(TYPE s){ return (s<<12) | (s>>(SIZE-12)); }
TYPE rol13(TYPE s){ return (s<<13) | (s>>(SIZE-13)); }
TYPE rol14(TYPE s){ return (s<<14) | (s>>(SIZE-14)); }
TYPE rol15(TYPE s){ return (s<<15) | (s>>(SIZE-15)); }
#endif

#if SIZE >=32
TYPE rol16(TYPE s){ return (s<<16) | (s>>(SIZE-16)); }
TYPE rol17(TYPE s){ return (s<<17) | (s>>(SIZE-17)); }

TYPE rol23(TYPE s){ return (s<<23) | (s>>(SIZE-23)); }
TYPE rol24(TYPE s){ return (s<<24) | (s>>(SIZE-24)); }
TYPE rol25(TYPE s){ return (s<<25) | (s>>(SIZE-25)); }

TYPE rol30(TYPE s){ return (s<<30) | (s>>(SIZE-30)); }
TYPE rol31(TYPE s){ return (s<<31) | (s>>(SIZE-31)); }
#endif

static void
testRol(void)
{
    volatile TYPE t = TEST_VECT;
    TYPE u;

    u = t;

    ASSERT( rol1(u) == (TYPE)((TEST_VECT<<1) | (TEST_VECT>>(SIZE-1))) );
    ASSERT( rol2(u) == (TYPE)((TEST_VECT<<2) | (TEST_VECT>>(SIZE-2))) );
    ASSERT( rol3(u) == (TYPE)((TEST_VECT<<3) | (TEST_VECT>>(SIZE-3))) );
    ASSERT( rol4(u) == (TYPE)((TEST_VECT<<4) | (TEST_VECT>>(SIZE-4))) );
    ASSERT( rol5(u) == (TYPE)((TEST_VECT<<5) | (TEST_VECT>>(SIZE-5))) );
    ASSERT( rol6(u) == (TYPE)((TEST_VECT<<6) | (TEST_VECT>>(SIZE-6))) );
    ASSERT( rol7(u) == (TYPE)((TEST_VECT<<7) | (TEST_VECT>>(SIZE-7))) );

#if SIZE >=16
    ASSERT( rol8 (u) == (TYPE)((TEST_VECT<<8 ) | (TEST_VECT>>(SIZE-8 ))) );
    ASSERT( rol9 (u) == (TYPE)((TEST_VECT<<9 ) | (TEST_VECT>>(SIZE-9 ))) );
    ASSERT( rol10(u) == (TYPE)((TEST_VECT<<10) | (TEST_VECT>>(SIZE-10))) );
    ASSERT( rol11(u) == (TYPE)((TEST_VECT<<11) | (TEST_VECT>>(SIZE-11))) );
    ASSERT( rol12(u) == (TYPE)((TEST_VECT<<12) | (TEST_VECT>>(SIZE-12))) );
    ASSERT( rol13(u) == (TYPE)((TEST_VECT<<13) | (TEST_VECT>>(SIZE-13))) );
    ASSERT( rol14(u) == (TYPE)((TEST_VECT<<14) | (TEST_VECT>>(SIZE-14))) );
    ASSERT( rol15(u) == (TYPE)((TEST_VECT<<15) | (TEST_VECT>>(SIZE-15))) );
#endif

#if SIZE >=32
    ASSERT( rol16(u) == (TYPE)((TEST_VECT<<16) | (TEST_VECT>>(SIZE-16))) );
    ASSERT( rol17(u) == (TYPE)((TEST_VECT<<17) | (TEST_VECT>>(SIZE-17))) );

    ASSERT( rol23(u) == (TYPE)((TEST_VECT<<23) | (TEST_VECT>>(SIZE-23))) );
    ASSERT( rol24(u) == (TYPE)((TEST_VECT<<24) | (TEST_VECT>>(SIZE-24))) );
    ASSERT( rol25(u) == (TYPE)((TEST_VECT<<25) | (TEST_VECT>>(SIZE-25))) );

    ASSERT( rol30(u) == (TYPE)((TEST_VECT<<30) | (TEST_VECT>>(SIZE-30))) );
    ASSERT( rol31(u) == (TYPE)((TEST_VECT<<31) | (TEST_VECT>>(SIZE-31))) );
#endif
}
