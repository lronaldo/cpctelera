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

#include <stdint.h>

int x=1;

void
testMM(void)
{
  int *p=&x, *q=&x;
  // read low-order (little endian) representation byte of p 
  unsigned char i = *(unsigned char*)&p;
  // check the bottom two bits of an int* are not used#
  if (_Alignof(int) >= 4 && (i & 3u) == 0u) {
    // set the low-order bit of the byte
    i = i | 1u;  
    // write the representation byte back
    *(unsigned char*)&p = i; 
    // [p might be passed around or copied here]
    // clear the low-order bits again
    *(unsigned char*)&p = (*(unsigned char*)&p) & ~((unsigned char)3u);
    // are p and q now equivalent?  
    *p = 11;          // does this have defined behaviour? 
    _Bool b = (p==q); // is this true?
    if (b)
      ASSERT (x == *p);
  }
}

