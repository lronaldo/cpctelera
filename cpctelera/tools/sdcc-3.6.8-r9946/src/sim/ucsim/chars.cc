/*
 * Simulator of microcontrollers (chars.cc)
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "charscl.h"


chars::chars(void)
{
  chars_string= 0;
  chars_length= 0;
  dynamic= false;
  pars_pos= 0;
}

chars::chars(char *s)
{
  chars_string= 0;
  chars_length= 0;
  allocate_string(s);
}

chars::chars(const char *s)
{
  if ((chars_string= (char*)s) != NULL)
    chars_length= strlen(s);
  else
    chars_length= 0;
  dynamic= false;
  pars_pos= 0;
}
/*
chars::chars(const char *s)
{
  chars_string= 0;
  chars_length= 0;
  allocate_string((char*)s);
}
*/
chars::chars(const chars &cs)
{
  chars_string= 0;
  chars_length= 0;
  allocate_string((char*)cs);
}

chars::chars(const char *, const char *fmt, ...)
{
  va_list ap;
  char n[1000];

  va_start(ap, fmt);
  vsnprintf(n, 999, fmt, ap);
  va_end(ap);

  chars_string= strdup(n);
  chars_length= strlen(n);
  dynamic= true;
  pars_pos= 0;
}

chars::~chars(void)
{
  deallocate_string();
}


void
chars::allocate_string(char *s)
{
  char *n= NULL;
  int l= 0;
  bool d= false;
  
  if (s)
    {
      l= strlen(s);
      n= (char*)malloc(l+1);
      strcpy(n, s);
      d= true;
    }
  deallocate_string();
  chars_length= l;
  chars_string= n;
  dynamic= d;
  pars_pos= 0;
}

void
chars::deallocate_string(void)
{
  if (chars_string)
    if (dynamic)
      free(chars_string);
  chars_string= 0;
  chars_length= 0;
  dynamic= 0;
}


chars
chars::token(chars delims)
{
  chars c= (char*)NULL;

  if (delims.len() < 1)
    return c;
  if (pars_pos >= chars_length)
    return c;
  if (chars_length < 1)
    return c;

  int l;
  // skip initial delims first;
  l= strspn(&chars_string[pars_pos], (char*)delims);
  pars_pos+= l;
  if (pars_pos >= chars_length)
    return c;
  // skip chars not in delims: search token end
  l= strcspn(&chars_string[pars_pos], (char*)delims);
  if (l > 0)
    {
      // found
      int i;
      for (i= pars_pos; i < pars_pos+l; i++)
	c+= chars_string[i];
      pars_pos= i;
      // skip delims at end
      l= strspn(&chars_string[pars_pos], (char*)delims);
      pars_pos+= l;
      return c;
    }
  // not found more
  return c;
}

chars &
chars::append(char *s)
{
  if (!s)
    return(*this);

  char *temp= (char*)malloc(chars_length + strlen(s) + 1);
  if (chars_string)
    {
      strcpy(temp, chars_string);
      if (dynamic)
	free(chars_string);
    }
  else
    temp[0]= '\0';
  strcat(temp, s);
  chars_string= temp;
  chars_length+= strlen(s);
  dynamic= true;
  
  return *this;
}

chars &
chars::append(char c)
{
  if (!c)
    return(*this);

  char *temp= (char*)malloc(chars_length + 1 + 1);
  if (chars_string)
    {
      strcpy(temp, chars_string);
      if (dynamic)
	free(chars_string);
    }
  else
    temp[0]= '\0';
  char b[2];
  b[0]= c;
  b[1]= 0;
  strcat(temp, b);
  chars_string= temp;
  chars_length+= 1;
  dynamic= true;

  return *this;
}

chars &
chars::append(const char *format, ...)
{
  va_list ap;
  char n[1000];
  
  va_start(ap, format);
  vsnprintf(n, 999, format, ap);
  va_end(ap);

  char *temp= (char*)malloc(chars_length + strlen(n) + 1);
  if (chars_string)
    {
      strcpy(temp, chars_string);
      if (dynamic)
	free(chars_string);
    }
  else
    temp[0]= '\0';
  strcat(temp, n);
  chars_string= temp;
  chars_length+= strlen(n);
  dynamic= true;
  
  return *this;
}

chars &
chars::format(const char *format, ...)
{
  deallocate_string();

  va_list ap;
  char n[1000];
  
  va_start(ap, format);
  vsnprintf(n, 999, format, ap);
  va_end(ap);

  char *temp= (char*)malloc(chars_length + strlen(n) + 1);
  if (chars_string)
    {
      strcpy(temp, chars_string);
      if (dynamic)
	free(chars_string);
    }
  else
    temp[0]= '\0';
  strcat(temp, n);
  chars_string= temp;
  chars_length+= strlen(n);
  dynamic= true;
  
  return *this;
}

bool
chars::empty()
{
  return chars_length == 0;
}

bool
chars::is_null()
{
  return chars_string == NULL;
}

// Assignment operators
chars &
chars::operator=(char *s)
{
  allocate_string(s);
  return(*this);
}

chars &
chars::operator=(const chars &cs)
{
  allocate_string((char*)cs);
  return(*this);
}


// Arithmetic operators

chars
chars::operator+(char c)
{
  char b[2];
  b[0]= c;
  b[1]= 0;
  chars temp(chars_string);
  return temp.append(b);
}

chars
chars::operator+(char *s)
{
  chars temp(chars_string);
  return(temp.append(s));
}

chars
chars::operator+(const chars &cs)
{
  chars temp(chars_string);
  return(temp.append(cs));
}

chars
operator+(char c, const chars &cs)
{
  char b[2];
  b[0]= c;
  b[1]= 0;
  chars temp(b);
  return(temp.append((char*)cs));
};

chars
operator+(char *s, const chars &cs)
{
  chars temp(s);
  return(temp.append((char*)cs));
};


// Boolean operators
bool
chars::equal(char *s)
{
  if ((chars_string &&
       !s) ||
      (!chars_string &&
       s))
    return(0);
  if (!chars_string &&
      !s)
    return(1);
  return(strcmp(chars_string, s) == 0);
}

bool
chars::operator==(char *s)
{
  return(equal(s));
}

bool
chars::operator==(const char *s)
{
  return(equal((char*)s));
}

bool
chars::operator==(chars &cs)
{
  return(*this == (char*)cs);
}

bool
operator==(char *s, const chars &cs)
{
  return((chars(cs)).equal(s));
}

bool
operator==(const char *s, const chars &cs)
{
  return((chars(cs)).equal((char*)s));
}

bool
chars::operator!=(char *s)
{
  return(!equal(s));
}

bool
chars::operator!=(const char *s)
{
  return(!equal((char*)s));
}

bool
chars::operator!=(chars &cs)
{
  return(*this != (char*)cs);
}

bool
operator!=(char *s, const chars &cs)
{
  return(!(chars(cs)).equal(s));
}

bool
operator!=(const char *s, const chars &cs)
{
  return(!(chars(cs)).equal((char*)s));
}

/*
cchars::cchars(const char *s):
chars(s)
{
}
*/
/*
cchars::cchars(char const *s):
chars(s)
{
}

cchars::~cchars(void)
{
  deallocate_string();
}

void
cchars::allocate_string(const char *s)
{
  deallocate_string();
  allocate_string((char*)s);
}

void
cchars::deallocate_string(void)
{
  chars_string= 0;
  chars_length= 0;
}
*/

/* End of chars.cc */
