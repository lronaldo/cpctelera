#include <80c51xa.h>

#define	BAUD_RATE 9600
#define	OSC 20000000L	/* Xtal frequency */

void putchar(char);
char getchar(void);
int puts(char *);

void exit_simulator(void);
