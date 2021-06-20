/*
   bug-2554.c
   Triggered a stack corruption bug in STM8 code generation for !
*/

#include <testfwk.h>

#include <stdint.h>

uint16_t numTests;

void fail(void)
{
}

static inline uint16_t llvm_add_u16(uint16_t a, uint16_t b) {
  uint16_t r = a + b;
  return r;
}

static inline uint16_t llvm_or_u16(uint16_t a, uint16_t b) {
  uint16_t r = a | b;
  return r;
}

void testOr(void) {
  uint8_t llvm_cbe_res;
  uint16_t llvm_cbe_a;
  uint16_t llvm_cbe_tmp__88;
  uint8_t llvm_cbe_tmp__89;
  uint16_t llvm_cbe_tmp__90;
  uint16_t llvm_cbe_tmp__91;
  uint8_t llvm_cbe_tmp__92;
  uint16_t llvm_cbe_tmp__93;
  uint16_t llvm_cbe_tmp__94;
  uint8_t llvm_cbe_tmp__95;
  uint16_t llvm_cbe_tmp__96;
  uint16_t llvm_cbe_tmp__97;
  uint8_t llvm_cbe_tmp__98;

llvm_cbe_if_2e_end18:
  llvm_cbe_tmp__88 = numTests;
  numTests = (llvm_add_u16(llvm_cbe_tmp__88, 1));
  llvm_cbe_tmp__89 = llvm_cbe_res;
  if (((((((uint16_t)(uint8_t)llvm_cbe_tmp__89)) == ((uint16_t)1))&1))) {
    goto llvm_cbe_cond_2e_end25;
  }

llvm_cbe_cond_2e_end25:
  llvm_cbe_tmp__90 = *((volatile uint16_t*)&llvm_cbe_a);
  if (((((llvm_or_u16(llvm_cbe_tmp__90, 17185u)) != ((uint16_t)0))&1))) {
    goto llvm_cbe_if_2e_else29;
  }

  llvm_cbe_res = 1;
  goto llvm_cbe_if_2e_end30;

llvm_cbe_if_2e_else29:
  llvm_cbe_res = 0;

llvm_cbe_if_2e_end30:
  llvm_cbe_tmp__91 = numTests;
  numTests = (llvm_add_u16(llvm_cbe_tmp__91, 1));
  llvm_cbe_tmp__92 = llvm_cbe_res;
  if (((((((uint16_t)(uint8_t)llvm_cbe_tmp__92)) == ((uint16_t)0))&1))) {
    goto llvm_cbe_cond_2e_end37;
  }

llvm_cbe_cond_2e_end37:
  llvm_cbe_res = 1;
  llvm_cbe_tmp__93 = *((volatile uint16_t*)&llvm_cbe_a);
  if ((((llvm_cbe_tmp__93 != ((uint16_t)0))&1))) {
    goto llvm_cbe_if_2e_end41;
  }

  llvm_cbe_res = 0;

llvm_cbe_if_2e_end41:
  llvm_cbe_tmp__94 = numTests;
  numTests = (llvm_add_u16(llvm_cbe_tmp__94, 1));
  llvm_cbe_tmp__95 = llvm_cbe_res;
  if (((((((uint16_t)(uint8_t)llvm_cbe_tmp__95)) == ((uint16_t)1))&1))) {
    goto llvm_cbe_cond_2e_end48;
  }

llvm_cbe_cond_2e_end48:
  llvm_cbe_res = 1;
  llvm_cbe_tmp__96 = *((volatile uint16_t*)&llvm_cbe_a);
  if (!(((llvm_cbe_tmp__96 != ((uint16_t)0))&1))) {
    goto llvm_cbe_if_2e_end52;
  }

  llvm_cbe_res = 0;

llvm_cbe_if_2e_end52:
  llvm_cbe_tmp__97 = numTests;
  numTests = (llvm_add_u16(llvm_cbe_tmp__97, 1));
  llvm_cbe_tmp__98 = llvm_cbe_res;
  if (((((((uint16_t)(uint8_t)llvm_cbe_tmp__98)) == ((uint16_t)0))&1))) {
    goto llvm_cbe_cond_2e_end59;
  }

  fail();

llvm_cbe_cond_2e_end59:
  return;
}

void testBug(void)
{
}

