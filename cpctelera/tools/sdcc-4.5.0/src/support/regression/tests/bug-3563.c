/* bug-3563.c
   A parameter passing issue in the mcs51 port when using --stack-auto.
 */

// According to bug reporter, his reproducing sample code, from which this regression test is derived, was derived from TinyUSb. TinyUSB is under MIT license.

#include <testfwk.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
bool check_now;

void my_putchar(char c) {
  (void)c;
}

void my_putchar_hex(uint8_t c) {
  if (check_now)
    ASSERT (!c);
}

void my_puts(char const * s) {
  check_now = !strcmp (s, "rd_idx=");
}

typedef struct tu_fifo_t {
  uint8_t* buffer          ; // buffer pointer
  uint16_t depth           ; // max items

  struct {
    uint16_t item_size : 15; // size of each item
    bool overwritable  : 1 ; // ovwerwritable when full
  };

  volatile uint16_t wr_idx ; // write index
  volatile uint16_t rd_idx ; // read index
} tu_fifo_t;

static inline
uint16_t idx2ptr(uint16_t depth, uint16_t idx)
{
  // Only run at most 3 times since index is limit in the range of [0..2*depth)
  while ( idx >= depth ) idx -= depth;
  return idx;
}

static inline    // WORKAROUND: comment out this line
void _ff_pull(tu_fifo_t* f, void * app_buf, uint16_t rel)
{
  // WORKAROUND: move the puts("rel=") line here

  memcpy(app_buf, f->buffer + (rel * f->item_size), f->item_size);

  my_puts("rel="); my_putchar_hex(rel >> 8); my_putchar_hex(rel & 0xff); my_putchar('\n');
  my_puts("app_buf="); my_putchar_hex((uintptr_t)app_buf >> 8); my_putchar_hex((uintptr_t)app_buf & 0xff); my_putchar('\n');
}

static bool _tu_fifo_peek(tu_fifo_t* f, void * p_buffer, uint16_t wr_idx, uint16_t rd_idx)
{
  (void)wr_idx;

  uint16_t rd_ptr = idx2ptr(f->depth, rd_idx);    // becomes rel inside _ff_pull

  // WORKAROUND: move rd_idx line here

  // Peek data
  _ff_pull(f, p_buffer, rd_ptr);

  my_puts("rd_idx="); my_putchar_hex(rd_idx >> 8); my_putchar_hex(rd_idx & 0xff); my_putchar('\n');
  my_puts("p_buffer="); my_putchar_hex((uintptr_t)p_buffer >> 8); my_putchar_hex((uintptr_t)p_buffer & 0xff); my_putchar('\n');

  return true;
}

uint8_t app_buf[2];

int m(void) {
    uint8_t buf[32];

    tu_fifo_t fifo = {
        .buffer = buf,
        .depth = 16,
        .item_size = 2,
        .wr_idx = 1,
        .rd_idx = 0
    };

    // Here we pass wr_idx = 1, rd_idx=0 but rd_idx will be reported as 0xe21d if you print
    // it at any point after the memcpy inside _ff_pull. If you print it before that point,
    // or if you remove the "inline" on _ff_pull, the correct value of 0 is printed.

    _tu_fifo_peek(&fifo, app_buf, 1, 0);

    return 0;
}
#endif

void
testBug (void)
{
#ifndef __SDCC_mcs51 // This bug #3563 is not yet fixed.
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
  m ();
#endif
#endif
}

