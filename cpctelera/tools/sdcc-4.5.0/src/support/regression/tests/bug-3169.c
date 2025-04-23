/*
   bug-3169.c
   A bug in z80 pointer write code generation handling of non-dead register pair de when using --reserve-regs-iy.
 */

#include <testfwk.h>

#pragma disable_warning 283

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned char bool;

typedef struct _Struct {
  uint8_t abyte1;
  uint8_t abyte2;
  uint16_t aint;
} Struct;

Struct storage;

uint16_t srcInt;
bool flag;

extern void fn1(const Struct*) __z88dk_fastcall;

void f() {

  storage.abyte1 = 1;
  storage.abyte2 = 10;

  if (flag) {
    storage.aint = srcInt;
    fn1(&storage);

    return;
  }

  storage.aint = 1;
  fn1(&storage);
}

void fn1(const Struct *sp) __z88dk_fastcall
{
  ASSERT(sp == &storage);
}

void
testBug(void)
{
  flag = 1;
  f();
}

