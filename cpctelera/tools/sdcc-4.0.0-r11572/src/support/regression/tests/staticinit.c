/** Tests that the static initialiser code works.
    As the init code is now clever we have to be careful.

    type: char, int, long
*/

#include <testfwk.h>

#if !defined(__SDCC_pdk14) // Lack of memory
/*--------------------------------------------------
   regression test for #1864582:
   multiple definition of char cons w. --model-large
   compile-time test only */
const char *c = (const char *) "Booting";
/*------------------------------------------------*/

static {type} smallDense[] = {
  1, 2, 3, 4, 5, 6
};
#endif

static void
testSmallDense (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
  ASSERT (smallDense[0] == 1);
  ASSERT (smallDense[1] == 2);
  ASSERT (smallDense[2] == 3);
  ASSERT (smallDense[3] == 4);
  ASSERT (smallDense[4] == 5);
  ASSERT (smallDense[5] == 6);
#endif
}

#ifdef __SDCC_mcs51
__idata
#elif defined(__SDCC_pdk14)
const
#endif
static {type} smallSparse[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1
};

static void
testSmallSparse (void)
{
#if !defined(__SDCC_pdk14) // Lack of memory
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  ASSERT (smallSparse[0] == 1);
  ASSERT (smallSparse[1] == 1);
  ASSERT (smallSparse[2] == 1);
  ASSERT (smallSparse[3] == 1);
  ASSERT (smallSparse[4] == 1);
  ASSERT (smallSparse[5] == 1);
  ASSERT (smallSparse[6] == 1);
  ASSERT (smallSparse[7] == 1);
  ASSERT (smallSparse[8] == 1);
#endif
#endif
}

#ifdef __SDCC_mcs51
__idata
#elif defined(__SDCC_pdk14) || defined(__SDCC_pdk15)
const
#endif
static {type} smallSparseZero[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0
};

static {type} smallSparseZeroTail[] = {
  1, 2, 3
};

static void
testSmallSparseZero (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  ASSERT (smallSparseZero[0] == 0);
  ASSERT (smallSparseZero[1] == 0);
  ASSERT (smallSparseZero[2] == 0);
  ASSERT (smallSparseZero[3] == 0);
  ASSERT (smallSparseZero[4] == 0);
  ASSERT (smallSparseZero[5] == 0);
  ASSERT (smallSparseZero[6] == 0);
  ASSERT (smallSparseZero[7] == 0);
  ASSERT (smallSparseZero[8] == 0);
  // Make the compiler happy
  ASSERT (smallSparseZeroTail[0] == 1);
#endif
}

#ifdef __SDCC_mcs51
__xdata
#elif __SDCC_pic16
__code
#elif defined(__SDCC_pdk14) || defined(__SDCC_pdk15)
const
#endif
static {type} largeMixed[] = {
  1, 2, 3, 4, 5, 6, 7,	/* 0-6 */
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,	/* 8*12 = 96+7 = -102 */
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  3, 4, 5, 6, 3, 4, 5, 6,	/* 8*17 = 136+7 */
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6,
  3, 4, 5, 6, 3, 4, 5, 6
};

static void
testLargeMixed (void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  ASSERT (largeMixed[0] == 1);
  ASSERT (largeMixed[1] == 2);
  ASSERT (largeMixed[7] == 1);
  ASSERT (largeMixed[102] == 1);
  ASSERT (largeMixed[143] == 3);
  ASSERT (largeMixed[143+8] == 3);
  ASSERT (largeMixed[143+16] == 3);
  ASSERT (largeMixed[143+1] == 4);
  ASSERT (largeMixed[143+8+1] == 4);
  ASSERT (largeMixed[143+16+1] == 4);
#endif
}
