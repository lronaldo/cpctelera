/* bug-2807.c
   (s)printf() could not handle %s arguments longer than 127 bytes.
 */

#include <testfwk.h>

#include <stdio.h>

#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Array too big to fit in memory.
char buffer[260];
const char *string = "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "0123456789abcdef"
                     "t"; // 257 bytes excluding terminating 0.
#endif

void testBug(void)
{
#if !(defined(__SDCC_mcs51) && (defined(__SDCC_MODEL_SMALL) || defined(__SDCC_MODEL_MEDIUM))) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Array too big to fit in memory.
  sprintf (buffer, "X%sX", string);

  ASSERT (buffer[0] == 'X');
  ASSERT (buffer[1] == '0');
  ASSERT (buffer[256] == 'f');
  ASSERT (buffer[257] == 't');
  ASSERT (buffer[258] == 'X');
  ASSERT (buffer[259] == 0);
#endif
}

