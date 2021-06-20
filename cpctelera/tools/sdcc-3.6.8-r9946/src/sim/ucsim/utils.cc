/*
 * Simulator of microcontrollers (utils.cc)
 *
 * Copyright (C) 1997,16 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include "ddconfig.h"

#if defined(HAVE_VASPRINTF) && !defined(_GNU_SOURCE)
  /* define before including stdio.h to enable vasprintf() declaration */
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "i_string.h"

  // prj
#include "stypes.h"
#include "pobjcl.h"

#include "utils.h"


int
get_sub_opt(char **option, const char * const *tokens, char **valuep)
{
  char *end, *equ;
  int i;

  if (!(end= strchr(*option, ',')))
    end= *option + strlen(*option);
  else
    *end++= '\0';
  if ((equ= strchr(*option, '=')))
    {
      *valuep= equ+1;
      *equ= '\0';
    }
  else
    *valuep= 0;
  i= 0;
  while (tokens[i] &&
	 strcmp(*option, tokens[i]))
    i++;
  if (!tokens[i])
    *valuep= *option;
  *option= end;
  return tokens[i]?i:-1;
}


char *
get_id_string(struct id_element *ids, int id)
{
  int i= 0;

  while (ids[i].id_string &&
	 id != ids[i].id)
    i++;
  return(cchars(ids[i].id_string));
}

char *
get_id_string(struct id_element *ids, int id, char *def)
{
  char *s= get_id_string(ids, id);

  return(s?s:def);
}

int
get_string_id(struct id_element *ids, char *str)
{
  int i= 0;

  while (ids[i].id_string &&
	 strcmp(ids[i].id_string, str) != 0)
    i++;
  return(ids[i].id);
}

int
get_string_id(struct id_element *ids, char *str, int def)
{
  int i= 0;

  while (ids[i].id_string &&
	 strcmp(ids[i].id_string, str) != 0)
    i++;
  return(ids[i].id_string?ids[i].id:def);
}


char *
vformat_string(const char *format, va_list ap)
{
  char *msg= NULL;
#ifdef HAVE_VASPRINTF
  if (0 > vasprintf(&msg, format, ap))
    msg = NULL;
  return(msg);
#else
#  ifdef HAVE_VSNPRINTF
  msg = (char*)malloc(80*25);
  vsnprintf(msg, 80*25, format, ap);
  return(msg);
#  else
#    ifdef HAVE_VPRINTF
  msg = (char*)malloc(80*25);
  vsprintf(msg, format, ap); /* Dangerous */
  return(msg);
#    endif
#  endif
#endif
}

char *
format_string(const char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  char *s= vformat_string(format, ap);
  va_end(ap);
  return(s);
}


void
print_char_octal(char c, FILE *f)
{
  if (strchr("\a\b\f\n\r\t\v\"", c))
    switch (c)
      {
      case '\a': fprintf(f, "\a"); break;
      case '\b': fprintf(f, "\b"); break;
      case '\f': fprintf(f, "\f"); break;
      case '\n': fprintf(f, "\n"); break;
      case '\r': fprintf(f, "\r"); break;
      case '\t': fprintf(f, "\t"); break;
      case '\v': fprintf(f, "\v"); break;
      case '\"': fprintf(f, "\""); break;
      }
  else if (isprint(c))
    fprintf(f, "%c", c);
  else
    fprintf(f, "\\%03o", (int)c);
}


const char *
object_name(class cl_base *o)
{
  const char *name= 0;

  if (o)
    name= o->get_name();
  if (name &&
      *name)
    return(name);
  return(cchars("(unknown)"));
}


char *
case_string(enum letter_case lcase, char *str)
{
  char *p= strdup(str);
  char *s= p;

  switch (lcase)
    {
    case case_upper:
      while (p && *p) {
	*p= toupper(*p);
	p++;
      }
      break;
    case case_lower:
      while (p && *p) {
	*p= tolower(*p);
	p++;
      }
      break;
    case case_case:
      if (!p || *p == '\0')
	break;
      while (isspace(*p)) p++;
      if (*p)
	*p= toupper(*p);
      break;
    }
  return(s);
}

chars
cbin(long data, int bits)
{
  long mask= 1;
  chars c= "";
  
  mask= mask << ((bits >= 1)?(bits-1):0);
  while (bits--)
    {
      c+= (data&mask)?'1':'0';
      mask>>= 1;
    }
  return c;
}

/*char *
case_string(enum letter_case lcase, const char *str)
{
  char *p= NIL;

  if (!str ||
      !*str)
    return(NIL);
  p= strdup(str);
  return case_string(lcase, p);
}*/

double
dnow(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double)tv.tv_sec + ((double)tv.tv_usec/1000000.0);
}

int
strispn(char *s, char c)
{
  if (!s || !*s)
    return 0;
  char *p= strchr(s, c);
  if (!p)
    return -1;
  return p-s;
}

/* Return true if "serach_in" string ends with string "what" */

bool
strend(char *search_in, char *what)
{
  if (!search_in ||
      !what ||
      !*search_in ||
      !*what)
    return false;
  char *start= strstr(search_in, what);
  if (start == NULL)
    return false;
  if (start[strlen(what)] == '\0')
    return true;
  return false;
}

bool
valid_sym_name(char *s)
{
  if (!s || !*s)
    return false;
  if (!isalpha(*s) &&
      (*s != '_'))
    return false;
  char *p= s+1;
  for (; *p; p++)
    {
      if (!isalnum(*p) &&
	  (*p != '_'))
	return false;
    }
  return true;
}


bool
is_hex_file(class cl_f *f)
{
  char *n;
  if (!f)
    return false;
  n= f->get_file_name();
  if (!n ||
      !*n)
    return false;

  if (strend(n, cchars(".ihx")) ||
      strend(n, cchars(".hex")) ||
      strend(n, cchars(".ihex")))
    return true;

  return false;
}

bool
is_omf_file(class cl_f *f)
{
  char *n;
  if (!f)
    return false;
  n= f->get_file_name();
  if (!n ||
      !*n)
    return false;

  if (strend(n, cchars(".omf")))
    return true;

  return false;
}

bool
is_cdb_file(class cl_f *f)
{
  char *n;
  if (!f)
    return false;
  n= f->get_file_name();
  if (!n ||
      !*n)
    return false;

  if (strend(n, cchars(".cdb")))
    return true;

  return false;
}


/* End of utils.cc */
