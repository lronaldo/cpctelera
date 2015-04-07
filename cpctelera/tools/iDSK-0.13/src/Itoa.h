#ifndef __ITOA_H__
#define __ITOA_H__



/* macros min and max definitions */

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y) (X > Y ? X : Y)

char* uitoa(unsigned int value, char* string, int radix);
char* itoa(int value, char* string, int radix);


#endif
