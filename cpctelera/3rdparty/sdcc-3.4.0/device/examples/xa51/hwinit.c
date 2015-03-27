#include "hw.h"

bit b3;
code c2=1;
data d3, d4=4;
xdata x3, x4=5;

#define	DIVIDER	(OSC/(64L*BAUD_RATE))

void external_startup(void) {
  _asm
    mov.b	_WDCON,#0	;shut down the watchdog
    mov.b	_WFEED1,#0a5h
    mov.b	_WFEED2,#05ah
;   mov.b	_BCR,#1	;BCR: 8 data, 16 address
;   mov.b	_SCR,#1	;SCR: page zero mode
  _endasm;

  // init serial io
  TL1 = RTL1 = -DIVIDER;
  TH1 = RTH1 = -DIVIDER >> 8;
  TR1 = 1;		/* enable timer 1 */
  
  S0CON = 0x52;		/* mode 1, receiver enable */
  IPA4 |= 0x6;		/* maximum priority */
  ERI0=1;		/* enable receiver interupts */
  TI0==1;               /* transmitter empty */
  RI0=0;                /* receiver empty */
  
  //PSWH &= 0xf0;	/* start interupt system */
}

#define SIMULATOR 1

#ifdef SIMULATOR

void putchar(char c) {
  c; // hush the compiler
  _asm
    mov.b r0l, [r7+2]
    trap #0EH;
  _endasm;
}

char getchar() {
  return 0;
}

void exit_simulator(void) {
  _asm
    trap #0FH;
  _endasm;
}

#else

void putchar(char c) {
  while(!TI0) 
    ;
  S0BUF = c;
  TI0 = 0;
}

char getchar(void) {
  char	c;
  
  while (!RI0) 
    ;
  c=S0BUF;
  RI0=0;
  return c;
}

int kbhit(void) {
  return RI0;
}

#endif

int puts(char *string) {
  int chars=0;
  while (*string) {
    putchar (*string++);
    chars++;
  }
  putchar ('\n');
  return chars+1;
}
