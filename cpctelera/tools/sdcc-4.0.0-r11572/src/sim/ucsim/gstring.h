/**
 * Copyright 1999, 2000 by PC Drew
 *
 * All rights reserved.
 *
 * This file is a part of the gstring class - a string class for
 * C++ programs.
 *
 * The gstring class, including all files distributed with it,
 * is free software; you can redistribute it and/or use it and/or modify it
 * under the terms of the Python License (http://www.python.org/doc/Copyright.html)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef _GSTRING_H
#define _GSTRING_H

//#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

class gstring
{
public:
  // Constructors.
  gstring(void);
  //gstring(char);
  gstring(char*);
  gstring(int);
  gstring(double);

  // Copy Constructor.
  gstring(const gstring&);

  // Destructor.
  virtual ~gstring(void);

  // ASCII to Integer.
  inline int a2i(void)
  {
    assert(str != NULL);
    return (atoi(str));
  };

  // ASCII to Double.
  inline double a2d(void)
  {
    assert(str != NULL);
    return (atof(str));
  };

  // String length.
  inline int length(void)
  {
    assert(str != NULL);
    return (strlen(str));
  };

  // Repeat the string _x times.
  gstring& repeatme(int);
  inline gstring repeat(int _x)
  {
    return (gstring(str).repeatme(_x));
  };

  // Return the number of times _token appears in the string.
  int ntokens(char*);
  /*inline int ntokens(char _token)
  {
    return (ntokens((char*)_token));
    };*/
  inline int ntokens(gstring _token)
  { 
    return (ntokens((char*)_token));
  };

  // Return the number of times _token appears in the string, +1.
  // This is, theoretically, the number of fields seperated by _token.
  /*inline int nfields(char _token)
  {
    return (ntokens((char*)_token) + 1);
    };*/
  inline int nfields(char* _token)
  {
    return (ntokens(_token) + 1);
  };
  inline int nfields(gstring _token)
  {
    return (ntokens((char*)_token) + 1);
  };

  // Chop of the _xth character off the string.
  gstring& chop(int = 1);

  // Return a substring.
  gstring at(int, int);
  gstring first(int = 1);
  gstring last(int = 1);

  // Convert to upper case.
  gstring& upcaseme(void);
  inline gstring upcase(void)
  {
    return (gstring(str).upcaseme());
  };

  // Convert to lower case.
  gstring& downcaseme(void);
  inline gstring downcase(void)
  {
    return (gstring(str).downcaseme());
  };

  // Explode the string, delimted by a token, into an array of gstrings.
  gstring* explode(char*);
  /*inline gstring* explode(char _token)
  {
    return (explode((char*)_token));
    };*/
  inline gstring* explode(gstring _token)
  {
    return (explode((char*)_token));
  };

  // Append.
  gstring& append(char*);
  /*inline gstring& append(char _s)
  {
    return (append((char*)_s));
    };*/
  inline gstring& append(const gstring& _gstr)
  {
    return (append((char*)_gstr));
  };

  // Prepend.
  gstring& prepend(char*);
  /*inline gstring& prepend(char _s)
  {
    return (prepend((char*)_s));
    };*/
  inline gstring& prepend(gstring _gstr)
  {
    return (prepend((char*)_gstr));
  };

  // Addition assignment operators.
  /*inline gstring& operator +=(char _s)
  {
    return (append((char*)_s));
    };*/
  inline gstring& operator +=(char* _str)
  {
    return (append(_str));
  };
  inline gstring& operator +=(const gstring& _gstr)
  {
    return (append((char*)_gstr));
  };

  // Assignment operators.
  //gstring& operator =(char);
  gstring& operator =(char *);
  gstring& operator =(double);
  gstring& operator =(const gstring&);
  gstring& operator =(int);

  // Use for casting.  You can do both (char*)gstring and
  // (const char*)gstring.
  * operator char(void) { return str; };
  * operator char(void) const { return str; };

private:
  // Use for creating and destroying current gstrings.
  int create(char* _str);
  int destroy(void);

  // The string itself.
  char* str;
};

// Useful for converting one array type to another.
char** tochararray(gstring*, int);
gstring* togstringarray(char**, int);

// Implode an array of gstrings or char*s into a gstring.
gstring implode(char**, char*, int);
/*inline gstring implode(char** _strarr, char _token, int _num)
{
  return (implode(_strarr, (char*)_token, _num));
};*/
inline gstring implode(char** _strarr, gstring _token, int _num)
{
  return (implode(_strarr, (char*)_token, _num));
};
gstring implode(gstring*, char*, int);
/*inline gstring implode(gstring* _gstrarr, char _token, int _num)
{
  return (implode(_gstrarr, (char*)_token, _num));
};*/
inline gstring implode(gstring* _gstrarr, gstring _token, int _num)
{
  return (implode(_gstrarr, (char*)_token, _num));
};

// Addition operators.
inline gstring operator +(const gstring& _gstr, char* _str)
{
  gstring gstr(_gstr); return (gstr.append(_str));
};
inline gstring operator +(char* _str, const gstring& _gstr)
{
  gstring gstr(_str); return (gstr.append(_gstr));
};
inline gstring operator +(const gstring& _gstr1, const gstring& _gstr2)
{
  gstring gstr(_gstr1); return (gstr.append(_gstr2));
};

// Boolean equality operators.
/*inline bool operator ==(const gstring& _gstr, char _s)
{
  return (strcmp((char*)_gstr, (char*)_s) == 0);
};*/
inline bool operator ==(const gstring& _gstr, char* _str)
{
  return (strcmp((char*)_gstr, _str) == 0);
};
inline bool operator ==(const gstring& _gstr1, gstring _gstr2)
{
  return (strcmp((char*)_gstr1, (char*)_gstr2) == 0);
};

// Boolean inequality operators.
/*inline bool operator !=(const gstring& _gstr, char _s)
{
  return (strcmp((char*)_gstr, (char*)_s) != 0);
};*/
inline bool operator !=(const gstring& _gstr, char* _str)
{
  return (strcmp((char*)_gstr, _str) != 0);
};
inline bool operator !=(const gstring& _gstr1, const gstring& _gstr2)
{
  return (strcmp((char*)_gstr1, (char*)_gstr2) != 0);
};

// Boolean comparison operators.
/*inline bool operator <=(const gstring& _gstr, char _s)
{
  return (strcmp((char*)_gstr, (char*)_s) <= 0);
};*/
inline bool operator <=(const gstring& _gstr, char* _str)
{
  return (strcmp((char*)_gstr, _str) <= 0);
};
inline bool operator <=(const gstring& _gstr1, const gstring& _gstr2)
{
  return (strcmp((char*)_gstr1, (char*)_gstr2) <= 0);
};
/*inline bool operator >=(const gstring& _gstr, char _s)
{
  return (strcmp((char*)_gstr, (char*)_s) >= 0);
};*/
inline bool operator >=(const gstring& _gstr, char* _str)
{
  return (strcmp((char*)_gstr, _str) >= 0);
};
inline bool operator >=(const gstring& _gstr1, const gstring& _gstr2)
{
  return (strcmp((char*)_gstr1, (char*)_gstr2) >= 0);
};
/*inline bool operator <(const gstring& _gstr, char _s)
{
  return (strcmp((char*)_gstr, (char*)_s) < 0);
};*/
inline bool operator <(const gstring& _gstr, char* _str)
{
  return (strcmp((char*)_gstr, _str) < 0);
};
inline bool operator <(const gstring& _gstr1, const gstring& _gstr2)
{
  return (strcmp((char*)_gstr1, (char*)_gstr2) < 0);
};
/*inline bool operator >(const gstring& _gstr, char _s)
{
  return (strcmp((char*)_gstr, (char*)_s) > 0);
};*/
inline bool operator >(const gstring& _gstr, char* _str)
{
  return (strcmp((char*)_gstr, _str) > 0);
};
inline bool operator >(const gstring& _gstr1, const gstring& _gstr2)
{
  return (strcmp((char*)_gstr1, (char*)_gstr2) > 0);
};

// Stream operators (i.e. cin >> gstring or cout << gstring).
/*inline ostream& operator <<(ostream& _out, const gstring _gstr)
{
  return (_out << (char*)_gstr);
};
istream& operator >>(istream&, gstring&);*/

#endif 
