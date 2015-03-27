/*
   20031204-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

/* PR optimization/13260 */

#include <string.h>
#include <stdint.h>

typedef uint32_t u32;

u32 in_aton(const char* x)
{
  return 0x0a0b0c0d;
}

u32 root_nfs_parse_addr(char *name)
{
 u32 addr;
 int octets = 0;
 char *cp, *cq;

 cp = cq = name;
 while (octets < 4) {
  while (*cp >= '0' && *cp <= '9')
   cp++;
  if (cp == cq || cp - cq > 3)
   break;
  if (*cp == '.' || octets == 3)
   octets++;
  if (octets < 4)
   cp++;
  cq = cp;
 }

 if (octets == 4 && (*cp == ':' || *cp == '\0')) {
  if (*cp == ':')
   *cp++ = '\0';
  addr = in_aton(name);
  strcpy(name, cp);
 } else
  addr = (-1);

 return addr;
}

void
testTortureExecute (void)
{
  static char addr[] = "10.11.12.13:/hello";
  u32 result = root_nfs_parse_addr(addr);
  if (result != 0x0a0b0c0d) { ASSERT(0); }
  return;
}

