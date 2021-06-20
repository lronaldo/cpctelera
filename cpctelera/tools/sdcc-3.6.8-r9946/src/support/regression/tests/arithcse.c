/* Test arithmetic CSE with /+-*

    type: char, short, long
    attr: volatile,
 */

#include <testfwk.h>

void
test_arithCse(void)
{
  {attr} {type} res;
  {attr} {type} i = 10;

  /* addition with 0 */
  res = i + 0;
  ASSERT (i == 10);

  res = 0 + i;
  ASSERT (res == 10);

  /* multiplication with 1 */
  res = 1 * i;
  ASSERT (res == 10);

  res = i * 1;
  ASSERT (res == 10);

  /* multiplication with 0 */
  res = 0 * i;
  ASSERT (res == 0);

  res = i * 0;
  ASSERT (res == 0);

  /* multiplication with -1 */
  res = -1 * i;
  ASSERT (res == ({type})-i);

  res = i * -1;
  ASSERT (res == ({type})-i);

  /* division by 1 */
  res = i / 1;
  ASSERT (res == i);

  /* division by -1 */
  res = i / -1;
  ASSERT (res == ({type})-i);
}
