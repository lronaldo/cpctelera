/*
 * Simulator of microcontrollers (charscl.h)
 *
 * Copyright (C) 1997 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
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
  mutable int pars_pos;
public:
  chars(void);
  chars(char *s);
  chars(const char *s);
  chars(const chars &cs);
  chars(const char *, const char *fmt, ...);
  virtual ~chars(void);
private:
  void allocate_string(const char *s);
  void deallocate_string(void);

public:
  const char *c_str(void) const { return chars_string; }
  const char *cstr(void) const { return chars_string; }
  char *str(void) { return chars_string; }
  char c(int idx);
  chars &append(const char *s);
  chars &append(char c);
  chars &appendf(const char *format, ...);
  chars &appendn(const char *src, int n);
  chars &format(const char *format, ...);
  bool empty() const { return chars_length == 0; }
  bool nempty() const { return !empty(); }
  bool is_null()const { return !chars_string; }
  chars &uppercase(void);
  chars &subst(const char *what, char with);
  chars &substr(int start, int maxlen);
  int len() const { return chars_length; }
  int length() const { return chars_length; }
  void start_parse(void) const { start_parse(0); }
  void start_parse(int at) const { pars_pos= at; }
  chars token(const char *delims) const;
  unsigned int htoi(void);
  unsigned long long int htoll(void);
  void ltrim(void);
  void rtrim(void);
  void trim() { ltrim(); rtrim(); }
  void lrip(const char *cset);
  void rrip(const char *cset);
  void rrip(int nuof_chars);
  void rip(const char *cset) { lrip(cset); rrip(cset); }
  // search
  bool starts_with(const char *x) const;
  int first_pos(char c);
  long int lint(void);
  long int lint(int base);
public:
  // Operators

  // Cast
  operator const char*(void) const { return(chars_string); };
  // Assignment
  chars &operator=(const char *s);
  chars &operator=(const chars &cs);
  // Arithmetic
  chars operator+(char c) const;
  chars operator+(const char *s) const;
  chars &operator+=(char c) { return(append(c)); }
  chars &operator+=(const char *s) { return(append(s)); }
  // Boolean
  bool equal(const char *) const;
  bool operator==(const char *s) const;
  bool operator!=(const char *s) const;
};

extern chars operator+(char s, const chars &cs);


#endif

/* End of utils.src/charscl.h */
