/*
   bug-2452.c
*/

#include <stdbool.h>
#include <testfwk.h>

typedef struct
{
  unsigned char  byte[5];
} value_t;

static void
output_digit (unsigned char n)
{
  ASSERT(n == 9);
}

static void
calculate_digit (value_t *value)
{
  value->byte[4] = 9;
}

int
xprint_format (const char *format)
{
  value_t value;
  bool lsd;
  int i;

  unsigned char length;

  while( *format++ )
  {
    unsigned char store[6];
    unsigned char *pstore = &store[5];

    format++;


    lsd = 1;
    i = 1;
    do {
      value.byte[4] = 0;

      calculate_digit(&value);

      if (!lsd)
      {
        *pstore = (value.byte[4] << 4) | (value.byte[4] >> 4) | *pstore;
        pstore--;
      }
      else
      {
        *pstore = value.byte[4];
      }

      lsd = !lsd;
    } while( i-- );

    length = 2;

    ASSERT(lsd);

    while( length-- )
    {
      lsd = !lsd;
      if (!lsd)
      {
        pstore++;
        value.byte[4] = *pstore >> 4;
      }
      else
      {
        value.byte[4] = *pstore & 0x0F;
      }

      output_digit( value.byte[4]);
    }
  }

  return 0;
}

void
test_xprintf (void)
{
  xprint_format ("%d");
}

