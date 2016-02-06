/* bad shift right 12 */

#include <testfwk.h>

typedef unsigned int u16_t;

struct myhdr { u16_t x; } h, *p;

#define NTOHS(n) (((((u16_t)(n) & 0xff)) << 8) | (((u16_t)(n) & 0xff00) >> 8))
#define IPH_V(hdr) ((u16_t)NTOHS((hdr)->x) >> 12)

void testBug(void) {
  p = &h;
  p->x = 0x45;
  ASSERT(IPH_V(p)==4);
}
