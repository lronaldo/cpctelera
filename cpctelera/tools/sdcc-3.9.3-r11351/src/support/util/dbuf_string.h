/*
  dbuf_string.h - Append formatted string to the dynamic buffer
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


#ifndef __DBUF_STRING_H
#define __DBUF_STRING_H

#include <stdarg.h>
#include <stdio.h>
#include "dbuf.h"

/* Attribute `nonnull' was valid as of gcc 3.3.  */
#ifndef ATTRIBUTE_NONNULL
# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)
#  define ATTRIBUTE_NONNULL(m) __attribute__ ((__nonnull__ (m)))
# else
#  define ATTRIBUTE_NONNULL(m)
# endif /* GNUC >= 3.3 */
#endif /* ATTRIBUTE_NONNULL */

/* The __-protected variants of `format' and `printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
#ifndef ATTRIBUTE_PRINTF
# if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#  define ATTRIBUTE_PRINTF(m, n) __attribute__ ((__format__ (__printf__, m, n))) ATTRIBUTE_NONNULL(m)
#else
#  define ATTRIBUTE_PRINTF(m, n)
# endif /* GNUC >= 2.7 */
#endif /* ATTRIBUTE_PRINTF */

#ifdef __cplusplus
extern "C" {
#endif

int dbuf_append_str(struct dbuf_s *dbuf, const char *str);
int dbuf_prepend_str(struct dbuf_s *dbuf, const char *str);
int dbuf_append_char(struct dbuf_s *dbuf, char chr);
int dbuf_prepend_char(struct dbuf_s *dbuf, char chr);
int dbuf_vprintf(struct dbuf_s *dbuf, const char *format, va_list args);
int dbuf_printf (struct dbuf_s *dbuf, const char *format, ...) ATTRIBUTE_PRINTF(2, 3);
size_t dbuf_getline(struct dbuf_s *dbuf, FILE *infp);
size_t dbuf_chomp (struct dbuf_s *dbuf);
void dbuf_write (struct dbuf_s *dbuf, FILE *dest);
void dbuf_write_and_destroy (struct dbuf_s *dbuf, FILE *dest);

#ifdef __cplusplus
}
#endif

#endif  /* __DBUF_STRING_H */
