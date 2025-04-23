/*
  dbuf_string.c - Append formatted string to the dynamic buffer
  version 1.2.2, March 20th, 2012

  Copyright (c) 2002-2012 Borut Razem

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street - Fifth Floor,
  Boston, MA 02110-1301, USA.
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dbuf_string.h"


/*
 * Append string to the end of the buffer.
 * The buffer is null terminated.
 */

int
dbuf_append_str (struct dbuf_s *dbuf, const char *str)
{
  size_t len;
  assert (str != NULL);

  len = strlen (str);
  if (dbuf_append (dbuf, str, len + 1))
    {
      --dbuf->len;
      return 1;
    }
  else
    return 0;
}

/*
 * Prepend string to the beginning of the buffer.
 * The buffer is null terminated.
 */

int
dbuf_prepend_str (struct dbuf_s *dbuf, const char *str)
{
  size_t len;
  assert (str != NULL);

  len = strlen (str);
  return (dbuf_prepend (dbuf, str, len));
}

/*
 * Append single character to the end of the buffer.
 * The buffer is null terminated.
 */

int
dbuf_append_char (struct dbuf_s *dbuf, char chr)
{
  char buf[2];
  buf[0] = chr;
  buf[1] = '\0';
  if (dbuf_append (dbuf, buf, 2))
    {
      --dbuf->len;
      return 1;
    }
  else
    return 0;
}

/*
 * Prepend single character to the end of the buffer.
 * The buffer is null terminated.
 */

int
dbuf_prepend_char (struct dbuf_s *dbuf, char chr)
{
  char buf[2];
  buf[0] = chr;
  buf[1] = '\0';
  return (dbuf_prepend_str (dbuf, buf));
}

/*
 * Append the formatted string to the end of the buffer.
 * The buffer is null terminated.
 */

int
dbuf_vprintf (struct dbuf_s *dbuf, const char *format, va_list args)
{
  va_list args2;
  va_copy (args2, args);
  int size = vsnprintf (0, 0, format, args2) + 1;
  va_end (args2);

  assert (dbuf != NULL);
  assert (dbuf->alloc != 0);
  assert (dbuf->buf != NULL);

  if (0 != _dbuf_expand (dbuf, size))
    {
      int len = vsprintf (&(((char *)dbuf->buf)[dbuf->len]), format, args);

      if (len >= 0)
        {
          /* if written length is greater then the calculated one,
             we have a buffer overrun! */
          assert (len <= size);
          dbuf->len += len;
        }
      return len;
    }

  return 0;
}


/*
 * Append the formatted string to the end of the buffer.
 * The buffer is null terminated.
 */

int
dbuf_printf (struct dbuf_s *dbuf, const char *format, ...)
{
  va_list arg;
  int len;

  va_start (arg, format);
  len = dbuf_vprintf (dbuf, format, arg);
  va_end (arg);

  return len;
}


/*
 * Append line from file to the dynamic buffer
 * The buffer is null terminated.
 */

size_t
dbuf_getline (struct dbuf_s *dbuf, FILE *infp)
{
  int c;
  char chr;

  while ((c = getc (infp)) != '\n' && c != EOF)
    {
      chr = c;

      dbuf_append (dbuf, &chr, 1);
    }

  /* add trailing NL */
  if (c == '\n')
    {
      chr = c;

      dbuf_append (dbuf, &chr, 1);
    }

  /* terminate the line without increasing the length */
  if (0 != _dbuf_expand (dbuf, 1))
    ((char *)dbuf->buf)[dbuf->len] = '\0';

  return dbuf_get_length (dbuf);
}


/*
 * Remove trailing newline from the string.
 * The buffer is null terminated.
 * It returns the total number of characters removed.
 */

size_t
dbuf_chomp (struct dbuf_s *dbuf)
{
  size_t i = dbuf->len;
  size_t ret;

  if (i != 0 && '\n' == ((char *)dbuf->buf)[i - 1])
    {
      --i;
      if (i != 0 && '\r' == ((char *)dbuf->buf)[i - 1])
        {
          --i;
        }
    }

  ret = dbuf->len - i;
  dbuf->len = i;

  /* terminate the line without increasing the length */
  if (_dbuf_expand(dbuf, 1) != 0)
    ((char *)dbuf->buf)[dbuf->len] = '\0';

  return ret;
}


/*
 * Write dynamic buffer to the file.
 */

void
dbuf_write (struct dbuf_s *dbuf, FILE *dest)
{
  fwrite (dbuf_get_buf (dbuf), 1, dbuf_get_length (dbuf), dest);
}


/*
 * Write dynamic buffer to the file and destroy it.
 */

void
dbuf_write_and_destroy (struct dbuf_s *dbuf, FILE *dest)
{
  dbuf_write (dbuf, dest);

  dbuf_destroy (dbuf);
}

