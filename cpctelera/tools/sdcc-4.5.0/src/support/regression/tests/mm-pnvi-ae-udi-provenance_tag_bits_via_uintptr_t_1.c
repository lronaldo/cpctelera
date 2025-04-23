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

int x=1;

void
testMM(void)
{
  int *p = &x;
  // cast &x to an integer 
  uintptr_t i = (uintptr_t) p;
  // check the bottom two bits of an int* are not used
  if (_Alignof(int) >= 4 && (i & 3u) == 0u) {
    // construct an integer like &x with low-order bit set
    i = i | 1u;  
    // cast back to a pointer
    int *q = (int *) i; // does this have defined behaviour?
    // cast to integer and mask out the low-order two bits
    uintptr_t j = ((uintptr_t)q) & ~((uintptr_t)3u);  
    // cast back to a pointer
    int *r = (int *) j; 
    // are r and p now equivalent?  
    *r = 11;           //  does this have defined behaviour? 
    _Bool b = (r==p);  //  is this true?
    if (b)
      ASSERT (*r == x);
  }
}

