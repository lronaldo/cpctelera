/*
   bug-2559.c
 */

#include <testfwk.h>

#include <stdint.h>

#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Bug #2874
static inline uint16_t shl_u16(uint16_t a, uint16_t b) {
  uint16_t r = a << b;
  return r;
}
static inline uint16_t ashr_u16(int16_t a, int16_t b) {
  uint16_t r = a >> b;
  return r;
}
static inline uint16_t or_u16(uint16_t a, uint16_t b) {
  uint16_t r = a | b;
  return r;
}
#endif

void testSwap_4(void) {
#if !defined( __SDCC_pdk14) && !defined( __SDCC_pdk15) // Bug #2874
  uint8_t tt;
  volatile uint8_t llvm_cbe_tmp__1;
  uint8_t llvm_cbe_tmp__2;
  uint8_t llvm_cbe_tmp__3;

  llvm_cbe_tmp__1 = 18;
  tt = llvm_cbe_tmp__1;
  llvm_cbe_tmp__2 = tt;
  llvm_cbe_tmp__3 = tt;
  tt = (((uint8_t)(or_u16((((uint16_t)(uint8_t)(((uint8_t)(shl_u16((((uint16_t)(uint8_t)llvm_cbe_tmp__2)), 4)))))), (((uint16_t)(uint8_t)(((uint8_t)(ashr_u16((((uint16_t)(uint8_t)llvm_cbe_tmp__3)), 4))))))))));

  ASSERT(tt == 33);
#endif
}

