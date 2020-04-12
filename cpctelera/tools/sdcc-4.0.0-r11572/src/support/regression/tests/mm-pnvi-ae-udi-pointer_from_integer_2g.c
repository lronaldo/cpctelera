// Derived from test case from the memory model study group of SC22 WG14
// For this test, behaviour is defined in the PNVI-ae-udi memory model.

/*
Original test copyright (c) 2012-2016 David Chisnall, Kayvan Memarian, and Peter Sewell.

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
above copyright notice and this permission notice appear in all
copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

// Adapted for SDCC by Philipp Klaus Krause in 2020.

#include <testfwk.h>

#pragma disable_warning 127

#include <stdint.h>

// Guess address of j.
#if defined (__SDCC_pdk14)
#define ADDRESS_PFI_2G 42
#elif defined (__SDCC_pdk14)
#define ADDRESS_PFI_2G 0x17fc
#elif defined (__SDCC_gbz80)
#define ADDRESS_PFI_2G 0xdff8
#else
#define ADDRESS_PFI_2G 0
#endif

void f() {
  uintptr_t i=ADDRESS_PFI_2G;
  int *p = (int*)i;
  *p=7;
}

void
testMM(void)
{
  int j=5;
  if ((uintptr_t)&j == ADDRESS_PFI_2G) {
    f();
    ASSERT (j == 7);
  }
}

