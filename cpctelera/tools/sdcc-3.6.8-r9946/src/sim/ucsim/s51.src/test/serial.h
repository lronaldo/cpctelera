#ifndef SERIAL_HEADER
#define SERIAL_HEADER


#define SERIAL_BUFFER_SIZE 8

#if defined __SDCC || defined SDCC
extern void serial_isr(void) __interrupt(4);
#elif defined __C51__
extern void serial_isr(void);
#else /* IAR4 */
interrupt void SCON_int(void);
#endif

extern void serial_init(long int br);
extern void beallitas(void);

extern unsigned char serial_send(unsigned char c);
extern unsigned char serial_received(void);
extern unsigned char serial_receive(void);

#define kikuld(c) serial_send(c)
#define vetel_volt() serial_received()
#define vett() serial_receive()

extern void serial_dummy(void);


#endif

/* End of serial.h */
