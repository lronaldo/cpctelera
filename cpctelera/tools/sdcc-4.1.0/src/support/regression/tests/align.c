/** Tests covering alignment operators.

    sign: signed, unsigned
    type: char, int, long
*/

#include <testfwk.h>

#include <stddef.h>

#if defined (__SDCC) || __STDC_VERSION__ >= 201112L
#include <stdalign.h>
char alignas (0) alignas({sign} {type}) a;
char alignas (int) alignas({sign} {type}) alignas(long) b;
char alignas ({sign} {type}) alignas(0) c;
#endif

void
testAlignof(void)
{
#if defined (__SDCC) || __STDC_VERSION__ >= 201112L
  ASSERT(alignof(char)  <= alignof({sign} {type}));
  /*ASSERT(alignof({sign} {type})  <= alignof(max_align_t)); #pragma std_c11 support incomplete  - maxalign_t */

  /* sdcc currently only architectures that do not have alignment restrictions. */
  ASSERT(alignof({sign} {type})  == 1);
  /*ASSERT(alignof(max_align_t)  == 1); #pragma std_c11 support incomplete - maxalign_t */
#endif
}

