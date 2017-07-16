/*
 * Simulator of microcontrollers (charscl.h)
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

#ifndef CHARSCL_HEADER
#define CHARSCL_HEADER


class chars
{
protected:
  char *chars_string;	// stores the value
  int chars_length;	// track of string length
  bool dynamic;
  int pars_pos;
public:
  chars(void);
  chars(char *s);
  chars(const char *s);
  chars(const chars &cs);
  chars(const char *, const char *fmt, ...);
  virtual ~chars(void);
private:
  virtual void allocate_string(char *s);
  virtual void deallocate_string(void);

public:
  virtual chars &append(char *s);
  virtual chars &append(char c);
  virtual chars &append(const char *format, ...);
  virtual chars &format(const char *format, ...);
  virtual bool empty();
  virtual bool is_null();
  virtual int len() { return chars_length; }
  virtual void start_parse(void) { start_parse(0); }
  virtual void start_parse(int at) { pars_pos= at; }
  virtual chars token(chars delims);
  
public:
  // Operators

  // Cast
  operator char*(void) { return(chars_string); };
  operator char*(void) const { return(chars_string); };
  // Assignment
  chars &operator=(char *s);
  chars &operator=(const chars &cs);
  // Arithmetic
  chars operator+(char c);
  chars operator+(char *s);
  chars operator+(const chars &cs);
  chars &operator+=(char c) { return(append(c)); }
  chars &operator+=(char *s) { return(append(s)); }
  chars &operator+=(const chars &cs) { return(append((char*)cs)); }
  // Boolean
  bool equal(char *);
  bool operator==(char *s);
  bool operator==(const char *s);
  bool operator==(chars &cs);
  bool operator!=(char *s);
  bool operator!=(const char *s);
  bool operator!=(chars &cs);
};

extern chars operator+(char s, const chars &cs);
extern chars operator+(char *s, const chars &cs);
extern bool operator==(char *s, const chars &cs);
extern bool operator==(const char *s, const chars &cs);
extern bool operator!=(char *s, const chars &cs);
extern bool operator!=(const char *s, const chars &cs);

/*
class cchars: public chars
{
 public:
  //cchars(const char *s);
  cchars(char const *s);
  virtual ~cchars(void);
 private:
  virtual void allocate_string(const char *s);
  virtual void deallocate_string(void);
};
*/

#define cchars chars

#endif

/* End of charscl.h */
