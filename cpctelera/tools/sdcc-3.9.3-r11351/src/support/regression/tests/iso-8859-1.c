/** tests for support for iso-8859-1. Extracted from string.c
*/
#include <testfwk.h>
#include <string.h>

/* SDCC has limited support for source encodings other than utf-8. This is a small test for what is there. */

/** tests for multibyte character sets
 * related to bug #3506236
*/
#ifndef PORT_HOST
static void
do_multibyte (void)
{
  const char *str = "ÔÂ";

  ASSERT (str[0] == '\xd4');
  ASSERT (str[1] == '\xc2');
}

// Test character constants - bug #2812.
void lcdWriteData (unsigned char LcdData)
{
  const unsigned char output[] = {'a', 0xe1, 'u', 0xf5, 0};

  static int i;

  ASSERT (LcdData == output[i++]);
}

void lcdWriteText (char *pcText)
{
  unsigned char i = 0;

  while (pcText[i] != '\0') {
    switch (pcText[i]) {
      case 'ä' : lcdWriteData (0xE1);
        break;
      case 'ü' : lcdWriteData (0xF5);
        break;
      case 'ö' : lcdWriteData (0xEF);
        break;
      case 'ß' : lcdWriteData (0xE2);
        break;
      default : lcdWriteData (pcText[i]);
        break;
    }
    i++;
  }
}
#endif

void testStr (void)
{
#ifndef PORT_HOST
  do_multibyte ();
#endif
}

void testCharconst (void)
{
#ifndef PORT_HOST
  lcdWriteText ("aäuü");
#endif
}


