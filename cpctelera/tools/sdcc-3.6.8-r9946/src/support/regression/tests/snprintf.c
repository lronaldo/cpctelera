/** tests for snprintf
  type: INT, LONG, STRING, FLOAT
*/
#include <testfwk.h>
#include <string.h>
#include <stdio.h>

#define {type} 1

#if 0
# define DEBUG(x) x
#else
# define DEBUG(x)
#endif

#define CHECK_B 0

#if defined(INT)

struct
{
  int arg;
  char *fmt;
  char *result;
} static const cases[] = {
  // arg, fmt,    result
  {0xab, "%04x", "00ab"},
  {0xffff, "0x%02X", "0xFFFF"},
  {0xffffu, "%u", "65535"},
  {1234, "%+6d", " +1234"},
  {12345, "% d", " 12345"},
  {-1234, "%d", "-1234"},
  {32767, "%8d", "   32767"},
  {1, "%%%d", "%1"},
  {1001, "%08i", "00001001"},
  {101, "%-+8d", "+101    "},
  {01234, "%o(oct)", "1234(oct)"},

  // do we want to check these:
#if defined(__SDCC) && !defined(__SDCC_z80) && CHECK_B
  {0x4142, "%bc %bx", "\x41 42"},       /* non-std: print as bytes! */
  {0xfffe, "0x%02bX%02bx", "0xFFfe"},   /* non-std: print as bytes! */
#elif defined(__SDCC) && defined(__SDCC_z80) && CHECK_B
  {0x4142, "%bc %bx", "\x42 41"},       /* non-std: print as bytes! */
  {0xfffe, "0x%02bX%02bx", "0xFEff"},   /* non-std: print as bytes! */
#endif
};

#elif defined(LONG)

struct
{
  long arg;
  char *fmt;
  char *result;
} static const cases[] = {
  // arg, fmt,    result
  {0x12345678, "0x%lx", "0x12345678"},
  {0x7fffFFFF, "%10lx", "  7fffffff"},
  {0x789abcde, "0x%-10lX", "0x789ABCDE  "},
  {0x1000a, "0x%02lX", "0x1000A"},
  {0x80000000, "%lu", "2147483648"},
  {-2147483648, "%li", "-2147483648"},
  {-1234, "%+6ld", " -1234"},
  {012345670123, "%lo(oct)", "12345670123(oct)"},
  {0xffffFFFF, "%lo(oct)", "37777777777(oct)"},

  // do we want to check these:
#if defined(SDCC) && !defined(SDCC_z80) && CHECK_B
  {0xfedcba98, "0x%bX%bX%bx%bx", "0xFEDCba98"}, /* non-std: print as bytes! */
#elif defined(SDCC) && defined(SDCC_z80) && CHECK_B
  {0xfedcba98, "0x%bX%bX%bx%bx", "0x98BAdcfe"}, /* non-std: print as bytes! */
#endif
};

#elif defined(STRING)

struct
{
  char *arg;
  char *fmt;
  char *result;
} static const cases[] = {
  // arg, fmt,    result
  {"abcd", "%s", "abcd"},
  {"abcd", "%3s", "abcd"},
  {"abcd", "%5s", " abcd"},
  {"abcd", "%-5s", "abcd "},
  {"abcd", "%.2s", "ab"},
  {"XYZ\\", "%s", "XYZ\x5c"},
  {"ab\x1b\x7f", "%s", "ab\x1b\x7f"},
  {"ab\tcd\n", "%s", "ab\tcd\n"},
};

#elif defined(FLOAT)

struct
{
  float arg;
  char *fmt;
  char *result;
} static const cases[] = {
  // arg, fmt,    result
  // ... there should be more ...
#if defined(__SDCC) && !defined(__SDCC_ds390) && !(defined(__SDCC_mcs51) && (defined(__SDCC_USE_XSTACK) || defined(__SDCC_MODEL_HUGE)))
  {1.0, "%f", "<NO FLOAT>"},
#else
  {1.0, "%f", "1.000000"},
  {1.96, "%3.1f", "2.0"},
#endif
};

#endif

void
test_snprintf (void)
{
#ifndef __SDCC_pic16
  unsigned char buf[32];
  unsigned char i;

  memset (buf, 0xfe, sizeof buf);       /* cookies all over */

  for (i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
      sprintf (buf, cases[i].fmt, cases[i].arg);
      DEBUG (__prints ("Test"));
      DEBUG (__printn (i));
      DEBUG (__prints (" should be: '"));
      DEBUG (__prints (cases[i].result));
      DEBUG (__prints ("' is: '"));
      DEBUG (__prints (buf));
      DEBUG (__prints ("'\n"));
      ASSERT (!strcmp (buf, cases[i].result));
    }

  ASSERT (buf[sizeof buf - 10] == 0xfe);        /* check for cookie */
#endif
}

