/*
 * Simulator of microcontrollers (cmd.src/cmdutil.cc)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

//#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "cmdutil.h"


/*
 * Processing escape sequencies in a string
 */

char *
proc_escape(char *string, int *len)
{
  char  spec_chars[]= "fnrtvab\"";
  char  spec[]= "\f\n\r\t\v\a\b\"";
  char  *s, *str, *p;

  s  = string;
  str= (char *)malloc(strlen(string)+1);
  p  = str;
  while (*s)
    {
      char *spec_c;

      if (*s == '\\' &&
	  *(s+1))
	{
	  s++;
	  if (*s == '0')
	    {
	      if (!isdigit(*(s+1)))
		{
		  *p++= '\0';
		  s++;
		}
	      else
		{
		  char *octal, *chk, data;
		  int i, j;
		  i= strspn(s, "01234567");
		  octal= (char *)malloc(i+1);
		  j= 0;
		  while (*s &&
			 (j < i))
		    octal[j++]= *s++;
		  octal[j]= '\0';
		  data= strtol(octal, &chk, 8);
		  if (!chk || !(*chk))
		    *p++= data;
		}
	    }
	  else
	    if ((spec_c= strchr(spec_chars, *s)) != NULL)
	      {
		*p++= spec[spec_c-spec_chars];
		s++;
	      }
	    else
	      *p++= *s++;
	}
      else
	*p++= *s++;
    }
  *p= '\0';
  *len= p-str;
  return(str);
}


int
bool_name(char *s, int *val)
{
  int v= 0;
  
  if (!s || !*s)
    return 0;
  if (strspn(s, "0nNfF") > 0)
    {
      if (val)
	*val= 0;
      return 1;
    }
  if (strspn(s, "1yYtT") > 0)
    {
      if (val)
	*val= 1;
      return 1;
    }
  if (strspn(s, "oO") == 1)
    {
      if (toupper(s[1]) == 'N')
	v= 1;
      else if (toupper(s[1]) == 'F')
	v= 0;
      else
	return 0;
      if (val)
	*val= v;
      return 1;
    }
  return 0;
}

/* End of cmd.src/cmdutil.cc */
