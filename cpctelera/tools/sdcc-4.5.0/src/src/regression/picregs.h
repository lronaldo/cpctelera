#ifndef PICREGS_H
#define PICREGS_H 1

#if defined(__SDCC_pic16)
#  include <pic18fregs.h>
#elif defined(__SDCC_pic14)
#  include <pic14regs.h>
#else
#  error "Unknown PIC architecture (__SDCC_pic14 and __SDCC_pic16 undefined)"
#endif

#endif

