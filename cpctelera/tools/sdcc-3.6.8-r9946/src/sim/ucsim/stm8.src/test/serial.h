#ifndef SERIAL_HEADER
#define SERIAL_HEADER

#include "stm8.h"

extern void isr_rx(void) __interrupt(USART_RX_IRQ);

extern char serial_received();

extern void print_cx1(char c);
extern void print_cx2(char c);
extern void print_ix4(int i);

#endif
