#include <iostream>
#include "Itoa.h"
using namespace std;
#define NUMBER_OF_DIGITS 16   /* space for NUMBER_OF_DIGITS + '\0' */

char* uitoa(unsigned int value, char* string, int radix)
{
unsigned char index, i;

  index = NUMBER_OF_DIGITS;
  i = 0;

  do {
    string[--index] = '0' + (value % radix);
    if ( string[index] > '9') string[index] += 'A' - ':';   /* continue with A, B,.. */
    value /= radix;
  } while (value != 0);

  do {
    string[i++] = string[index++];
  } while ( index < NUMBER_OF_DIGITS );

  string[i] = 0; /* string terminator */
  return string;
}

char* itoa(int value, char* string, int radix)
{
  if (value < 0 && radix == 10) {
    *string++ = '-';
    return uitoa(-value, string, radix);
  }
  else {
    return uitoa(value, string, radix);
  }
}

