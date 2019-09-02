/* strcmpi.c */

/*
 * Compare two strings ignoring case.
 *
 * Taken from GLIBC 2.2.5. Original code is copyrighted "Free
 * Software Foundation" and published under the GNU Lesser General
 * Public License.
 * 
 */

#include <ctype.h>
#include <stddef.h>

int as_strcmpi (const char *s1, const char *s2)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      c1 = tolower (*p1++);
      c2 = tolower (*p2++);
      if (c1 == '\0')
        break;
    }
  while (c1 == c2);
  
  return c1 - c2;
}

int as_strncmpi (const char *s1, const char *s2, size_t n)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if ((p1 == p2) || (n == 0))
    return 0;

  do
    {
      c1 = tolower (*p1++);
      c2 = tolower (*p2++);
      if (c1 == '\0')
        break;
    }
  while ((c1 == c2) && --n);

  return c1 - c2;
}
