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

#include "gstring.h"

/**
 * Precondition: none.
 * Postcondition: a new gstring is created, with an empty  string.
 */
gstring::gstring(void)
{
    create("");
}

/**
 * Precondition: _s is a char.
 * Postcondition: a new gstring is returned, with the value of _s.
 */
/*gstring::gstring(char _s)
{
    create((char*)_s);
}*/

/**
 * Precondition: _str is a char*.
 * Postcondition: a new gstring is returned, with the value of _str.
 */
gstring::gstring(char* _str)
{
    create(_str);
}

/**
 * Precondition: _x is an integer.
 * Postcondition: a new gstring is returned, with the string value of _x.
 */
gstring::gstring(int _x)
{
    char* str = new char;

    sprintf(str, "%i", _x);
    create(str);

    delete [] str;
}

/**
 * Precondition: _y is an double.
 * Postcondition: a new gstring is returned, with the string value of _y.
 */
gstring::gstring(double _y)
{
    char* str = new char;

    sprintf(str, "%f", _y);
    create(str);

    delete [] str;
}

/**
 * Precondition: _gstr is a gstring.
 * Postcondition: a copy of the existing gstring is returned.
 */
gstring::gstring(const gstring& _gstr)
{
    create((char*)_gstr);
}

/**
 * Precondition: none.
 * Postcondition: the current gstring is destroyed.
 */
gstring::~gstring(void)
{
    destroy();
}

/**
 * Precondition: _str is a char*.
 * Postcondition: _str is copied into the new gstring.
 */
int gstring::create(char* _str)
{
    assert(_str != NULL);

    // We want to create our own char*, instead of using
    // strdup() because new will never return NULL, and
    // malloc() (used by strdup()) might.
    str = new char[strlen(_str)];
    strcpy(str, _str);

    return 0;
}

/**
 * Precondition: none.
 * Postcondition: the current gstring is deleted.
 */
int gstring::destroy(void)
{
    delete [] str;

    return 0;
}

/**
 * Precondition: _index is the first char in the substring, starting at 0,
 *   _num is the number of chars in the substring.
 * Postcondition: the substring starting at _index and going _num chars
 *   in length is returned, as a gstring.
 */
gstring gstring::at(int _index, int _num)
{
    int len = length();

    assert(_index >= 0 && _num >= 1 && _index <= len - _num);

    char* temp_str = new char[len - _index];
    char* begin = str;
    char* end = temp_str;

    // Go to the character that is at _index and copy the
    // rest of the char* to temp_str.
    begin += _index;
    strcpy(temp_str, begin);

    // Go to the character that is at _index + _num and set
    // it equal to the null terminator.
    end += _num;
    *end = '\0';

    // Create a new gstring from the substring that we extracted.
    gstring gstr(temp_str);

    // Clean up.
    delete [] temp_str;

    return gstr;
}

/**
 * Precondition: _num is either an integer >= 1 or it is not specified.
 *   If the function is called without any args (i.e. first()), then
 *   _num = 1 by default.
 * Postcondition: the substring from the beginning of the string, going
 *   _num chars in length is returned.
 */
gstring gstring::first(int _num)
{
    return (at(0, _num));
}

/**
 * Precondition: _num is either an integer >= 1 or it is not specified.
 *   If the function is called without any args (i.e. first()), then
 *   _num = 1 by default.
 * Postcondition: the substring _num chars from the end of the string, going
 *   to the end of the string is to returned.
 */
gstring gstring::last(int _num)
{
    return (at(length() - _num, _num));
}

/**
 * Precondition: _num is the number of times you want the string repeated.
 * Postcondition: the current string is changed to be the current string,
 *   repeated _num times.
 */
gstring& gstring::repeatme(int _num)
{
    assert(str != NULL && _num >= 1);

    char* temp_str = new char[length() * _num];

    // Tack str onto the end of temp_str, _num times.
    for (int i = 0; i < _num; i++) {
        strcat(temp_str, str);
    }

    destroy();
    create(temp_str);

    delete [] temp_str;

    return *this;
}

/**
 * Precondition: _token is a char*.
 * Postcondition: returns the number of occurences of _token in the string.
 */
int gstring::ntokens(char* _token)
{
  char* temp_str = str;
  int len = strlen(_token);
  int i = 0;

  assert(_token != NULL && len >= 1);

  // Iterate through the string...
  for ( ; *temp_str != '\0'; temp_str++) {
    if (*temp_str == *_token && strncmp(_token, temp_str, len) == 0) {
      i++;
    }
  }

  return i;
}

/**
 * Precondition: _token is a char*.
 * Postcondition: an array of gstrings is returned.  The contents of each
 *   gstring in the array is the value either from the beginning of the
 *   original string to the first occurance of _token or from one occurance
 *   of _token to the next.  _token will not be returned in any of the strings.
 *
 * **NOTE**
 * Don't initialize you're array (i.e. call "new") before calling
 * this function...this function will do that for you.  You do, however,
 * need to call "delete [] array" in your own code.
 */
gstring* gstring::explode(char* _token)
{
    assert(_token != NULL && strlen(_token) >= 1);

    int i;
    int n = nfields(_token);
    char* ptr;
    char* temp_str = new char[length()];
    gstring* arr = new gstring[n];

    strcpy(temp_str, str);
    for (i = 0, ptr = strtok(temp_str, _token); ptr != NULL;
            i++, ptr = strtok(NULL, _token)) {
        arr[i] = ptr;
    }

    delete [] temp_str;

    return arr;
}

/**
 * Precondition: _arr is an array of char*'s and _token is a char*, one or more characters
 *     in length.
 * Postcondition: value returned is each char* in the array joined
 *     together by _token.
 */
gstring implode(char** _arr, char* _token, int _num)
{
    assert(_arr != NULL && _token != NULL && strlen(_token) >= 1);

    gstring s = _arr[0];

    for (int i = 1; i < _num; i++) {
        s.append(_token);
        s.append(_arr[i]);
    }

    return s;
}

/**
 * Precondition: _arr is an array of gstrings and _token is a char*, one or more characters
 *     in length.
 * Postcondition: value returned is each gstring in the array joined
 *     together by _token.
 */
gstring implode(gstring* _arr, char* _token, int _num)
{
    assert(_arr != NULL && _token != NULL);

    gstring s = _arr[0];

    for (int i = 1; i < _num; i++) {
        s.append(_token + _arr[i]);
    }

    return s;
}

/**
 * Precondition: _x is the number of characters to chop off the end of the string.
 *     The default value (chop called without any parameters) is 1 -- the last
 *     character will be removed.
 * Postcondition: the current gstring has the last _x characters removed from the string.
 *     together by _token.
 */
gstring& gstring::chop(int _x)
{
    int len = length() - _x;
    char* temp_str = new char[len];

    assert(_x >= 0);

    // This allows implode to join the strings together with a "" string (nothing).
    if (_x > 0) {
        strcpy(temp_str, str);
        temp_str[len] = '\0';

        destroy();
        create(temp_str);
    }

    delete [] temp_str;

    return *this;
}

/**
 * Precondition: _str is not NULL.
 * Postcondition: _str is appended to current gstring.
 */
gstring& gstring::append(char* _str)
{
    assert(_str != NULL);

    char* temp_str = new char[length() + strlen(_str)];

    strcpy(temp_str, str);
    strcat(temp_str, _str);

    destroy();
    create(temp_str);

    delete [] temp_str;

    return *this;
}

/**
 * Precondition: _str is not NULL.
 * Postcondition: _str is prepended to current gstring.
 */
gstring& gstring::prepend(char* _str)
{
    assert(_str != NULL);

    char* temp_str = new char[strlen(_str) + length()];

    strcpy(temp_str, _str);
    strcat(temp_str, str);

    destroy();
    create(temp_str);

    delete [] temp_str;

    return *this;
}

/**
 * Precondition: current gstring is not NULL.
 * Postcondition: current gstring is converted to uppercase.
 */
gstring& gstring::upcaseme(void)
{
    char* ptr = str;
    int len = length();

    assert(str != NULL);

    for (int i = 0; i < len; i++, ptr++) {
        *ptr = (char)toupper((int) * ptr);
    }

    return *this;
}

/**
 * Precondition: current gstring is not equal to NULL.
 * Postcondition: current gstring is converted to lowercase.
 */
gstring& gstring::downcaseme(void)
{
    char* ptr = str;
    int len = length();

    assert(str != NULL);

    for (int i = 0; i < len; i++, ptr++) {
        *ptr = (char)tolower((int) * ptr);
    }

    return *this;
}

/**
 * Precondition: _gstrarr is an array of gstrings and _num is the number of
 *   gstrings in the array.
 * Postcondition: the value returned is an exact copy of _gstrarr, only it
 *   it is an array of char*'s instead.
 */
char** tochararray(gstring* _gstrarr, int _num)
{
    char** strarr = new char * [_num];

    for (int i = 0; i < _num; i++) {
        strarr[i] = (char*)_gstrarr[i];
    }

    return strarr;
}

/**
 * Precondition: _strarr is an array of char*s and _num is the number of
 *   char*'s in the array.
 * Postcondition: the value returned is an exact copy of _strarr, only it
 *   it is an array of gstrings instead.
 */
gstring* togstringarray(char** _strarr, int _num)
{
    gstring* gstrarr = new gstring[_num];

    for (int i = 0; i < _num; i++) {
        gstrarr[i] = _strarr[i];
    }

    return gstrarr;
}

/*gstring& gstring::operator =(char _s)
{
    assert(_s != NULL);

    if (str != NULL) {
        destroy();
    }
    create((char*)_s);
    return *this;
}*/

gstring& gstring::operator =(char* _str)
{
    assert(_str != NULL);

    if (str != NULL) {
        destroy();
    }
    create(_str);
    return *this;
}

gstring& gstring::operator =(int _x)
{
  assert(_x != 0/*NULL*/);

  char* temp_str = new char;

  if (str != NULL) {
    destroy();
  }

  sprintf(temp_str, "%i", _x);
  create(temp_str);

  delete [] temp_str;
  return *this;
}

gstring& gstring::operator =(double _y)
{
  assert(_y != 0/*NULL*/);

  char* temp_str = new char;

  if (str != NULL) {
    destroy();
  }

  sprintf(temp_str, "%f", _y);
  create(temp_str);

  delete [] temp_str;
  return *this;
}

gstring& gstring::operator =(const gstring& _gstr)
{
    assert((char*)_gstr != NULL);

    if (str != NULL) {
        destroy();
    }

    create((char*)_gstr);

    return *this;
}

/*istream& operator >>(istream& in, gstring& _gstr)
{
    char** temp_str = new char * [1];

    in.gets(temp_str);
    _gstr = temp_str[0];

    delete [] temp_str[0];
    delete [] temp_str;

    return in;
}*/
