/*
 * Simulator of microcontrollers (utils.cc)
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

#include "ddconfig.h"

#if defined(HAVE_VASPRINTF) && !defined(_GNU_SOURCE)
  /* define before including stdio.h to enable vasprintf() declaration */
  #define _GNU_SOURCE
#endif
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
//#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#ifdef HAVE_FMOD
#include <math.h>
#endif

  // prj
#include "stypes.h"
//#include "pobjcl.h"

#include "globals.h"
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


const char *
get_id_string(struct id_element *ids, int id)
{
  int i= 0;

  while (ids[i].id_string &&
	 id != ids[i].id)
    i++;
  return(ids[i].id_string);
}

const char *
get_id_string(struct id_element *ids, int id, const char *def)
{
  const char *s= get_id_string(ids, id);

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
#ifdef HAVE_VSNPRINTF
  msg = (char*)malloc(80*25);
  vsnprintf(msg, 80*25, format, ap);
#else
  msg= (char*)malloc(80*25);
  vsprintf(msg, format, ap);
#endif
#endif
  return(msg);
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
  return("(unknown)");
}


char *
case_string(enum letter_case lcase, const char *str)
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
strend(const char *search_in, const char *what)
{
  if (!search_in ||
      !what ||
      !*search_in ||
      !*what)
    return false;
  const char *start= strstr(search_in, what);
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
filename_has_ext(class cl_f *f, const char *ext)
{
  const char *n;
  if (!f)
    return false;
  n= f->get_file_name();
  if (!n ||
      !*n)
    return false;

  if (strend(n, ext))
    return true;

  return false;
}

bool
is_hex_file(class cl_f *f)
{
  const char *n;
  if (!f)
    return false;
  n= f->get_file_name();
  if (!n ||
      !*n)
    return false;

  if (strend(n, ".ihx") ||
      strend(n, ".hex") ||
      strend(n, ".ihex"))
    return true;

  return false;
}

bool
is_asc_file(class cl_f *f)
{
  return filename_has_ext(f, ".asc");
}

bool
is_p2h_file(class cl_f *f)
{
  return filename_has_ext(f, ".p2h");
}

bool
is_omf_file(class cl_f *f)
{
  return filename_has_ext(f, ".omf");
}

bool
is_cdb_file(class cl_f *f)
{
  return filename_has_ext(f, ".cdb");
}

bool
is_s19_file(class cl_f *f)
{
  return filename_has_ext(f, ".s19");
}

bool
is_map_file(class cl_f *f)
{
  return filename_has_ext(f, ".map");
}

/*
  option_name=col_opt:col_opt

  option_name=
	prompt prompt_console command answer
	dump_address dump_label dump_number dump_char

  col_opt=
	B bold
	F faint
	I italic
	U underline
	D double_underline
	C crossedout
	O overline
	KL blink

  col_opt=
	black red green yellow blue magenta cyan white
	bblack bred bgreen byellow bblue bmagenta bcyan bwhite
	#RGB

*/

enum col_ctype_t
  {
   ct_none= 0,
   ct_bold= 0x01,
   ct_faint= 0x02,
   ct_italic= 0x04,
   ct_underl= 0x08,
   ct_dunderl= 0x10,
   ct_crossed= 0x20,
   ct_overl= 0x40,
   ct_blink= 0x80
  };

chars
colopt2ansiseq(char *opt)
{
  bool fg_rgb= false, bg_rgb= false;
  bool fg_bright= false, bg_bright= false;
  chars r= "", full= opt, tok= "";
  int fg= -1, bg= -1;
  int ctype= ct_none;
  class cl_option *o= application->options->get_option("color_bg");
  char *bgcolor;
  if (o) o->get_value(&bgcolor);
  
  if (!opt ||
      !*opt)
    return r;
  tok.start_parse();
  tok= full.token(":");
  while (tok.nempty())
    {
      const char *s= tok.c_str();
      if (tok=="bg")
	tok= bgcolor;
      if (tok=="black")
	{
	  if (fg<0)
	    fg= 0;
	  else
	    bg= 0;
	}
      else if (tok=="bblack")
	{
	  if (fg<0)
	    fg= 0, fg_bright= true;
	  else
	    bg= 0, bg_bright= true;
	}
      else if (tok=="red")
	{
	  if (fg<0)
	    fg= 1;
	  else
	    bg= 1;
	}
      else if (tok=="bred")
	{
	  if (fg<0)
	    fg= 1, fg_bright= true;
	  else
	    bg= 1, bg_bright= true;
	}
      else if (tok=="green")
	{
	  if (fg<0)
	    fg= 2;
	  else
	    bg= 2;
	}
      else if (tok=="bgreen")
	{
	  if (fg<0)
	    fg= 2, fg_bright= true;
	  else
	    bg= 2, bg_bright= true;
	}
      else if (tok=="yellow")
	{
	  if (fg<0)
	    fg= 3;
	  else
	    bg= 3;
	}
      else if (tok=="byellow")
	{
	  if (fg<0)
	    fg= 3, fg_bright= true;
	  else
	    bg= 3, bg_bright= true;
	}
      else if (tok=="blue")
	{
	  if (fg<0)
	    fg= 4;
	  else
	    bg= 4;
	}
      else if (tok=="bblue")
	{
	  if (fg<0)
	    fg= 4, fg_bright= true;
	  else
	    bg= 4, bg_bright= true;
	}
      else if (tok=="magenta")
	{
	  if (fg<0)
	    fg= 5;
	  else
	    bg= 5;
	}
      else if (tok=="bmagenta")
	{
	  if (fg<0)
	    fg= 5, fg_bright= true;
	  else
	    bg= 5, bg_bright= true;
	}
      else if (tok=="cyan")
	{
	  if (fg<0)
	    fg= 6;
	  else
	    bg= 6;
	}
      else if (tok=="bcyan")
	{
	  if (fg<0)
	    fg= 6, fg_bright= true;
	  else
	    bg= 6, bg_bright= true;
	}
      else if (tok=="white")
	{
	  if (fg<0)
	    fg= 7;
	  else
	    bg= 7;
	}
      else if (tok=="bwhite")
	{
	  if (fg<0)
	    fg= 7, fg_bright= true;
	  else
	    bg= 7, bg_bright= true;
	}
      else if (*s == '#')
	{
	  int c= strtol(&s[1], NULL, 16);
	  if (fg<0)
	    fg= c, fg_rgb= true;
	  else
	    bg= c, bg_rgb= true;
	}
      else
	{
	  int i;
	  if (strcspn(s, "bBfFiIuUdDcCoOkKlL") == 0)
	    for (i=0; s[i]; i++)
	      {
		switch (toupper(s[i]))
		  {
		  case 'B': ctype|= ct_bold; break;
		  case 'F': ctype|= ct_faint; break;
		  case 'I': ctype|= ct_italic; break;
		  case 'U': ctype|= ct_underl; break;
		  case 'D': ctype|= ct_dunderl; break;
		  case 'C': ctype|= ct_crossed; break;
		  case 'O': ctype|= ct_overl; break;
		  case 'K': ctype|= ct_blink; break;
		  case 'L': ctype|= ct_blink; break;
		  }
	      }
	}
      tok= full.token(":");
    }
    
  /* set character rendering mode */
  if (ctype != ct_none)
    {
      if (ctype & ct_bold) 	r.append("\033[1m");
      if (ctype & ct_faint)	r.append("\033[2m");
      if (ctype & ct_italic)	r.append("\033[3m");
      if (ctype & ct_underl)	r.append("\033[4m");
      if (ctype & ct_dunderl)	r.append("\033[21m");
      if (ctype & ct_crossed)	r.append("\033[9m");
      if (ctype & ct_overl)	r.append("\033[53m");
      if (ctype & ct_blink)	r.append("\033[5m");
    }

  /* Background color */
  if (bg >= 0)
    {
      if (bg_rgb)
	{
	  r.appendf("\033[48;2;%d;%d;%dm", (bg>>16)&0xff, (bg>>8)&0xff, bg&0xff);
	}
      else
	{
	  int i= 40+bg;
	  if (bg_bright)
	    i= 100+bg;
	  r.appendf("\033[%dm", i);
	}
    }
  
  /* Foreground color */
  if (fg >= 0)
    {
      if (fg_rgb)
	{
	  r.appendf("\033[38;2;%d;%d;%dm", (fg>>16)&0xff, (fg>>8)&0xff, fg&0xff);
	}
      else
	{
	  int i= 30+fg;
	  if (fg_bright)
	    i= 90+fg;
	  r.appendf("\033[%dm", i);
	}
    }
  
  return r;
}

double
strtoscale(const char *scale, const char **units)
{
  double d = 1.0;
  const char *u;

  if (!units)
    units = &u;

  *units = &scale[0];

  if (scale[0])
    {
      *units = &scale[1];
      switch (scale[0])
        {
          case 'f':
            d = 1 / 1000000000000000.0;
            break;
          case 'p':
            d = 1 / 1000000000000.0;
            break;
          case 'n':
            d = 1 / 1000000000.0;
            break;
          case 'u':
            d = 1 / 1000000.0;
            break;
          case 'm':
            d = 1 / 1000.0;
            break;
          default:
            if (!strncmp(scale, "µ", sizeof("µ") - 1))
              {
                d = 1 / 1000000.0;
                *units = &scale[sizeof("µ")];
              }
            else
              {
                if (scale[1] != 'i')
                  {
                    switch (scale[0])
                      {
                        case 'k':
                          d = 1000.0;
                          break;
                        case 'M':
                          d = 1000000.0;
                          break;
                        case 'G':
                          d = 1000000000.0;
                          break;
                        case 'T':
                          d = 1000000000000.0;
                          break;
                        case 'P':
                          d = 1000000000000000.0;
                          break;
                        default:
                          *units = &scale[0];
                          break;
                      }
                  }
                else
                  {
                    *units = &scale[2];
                    switch (scale[0])
                      {
                        case 'k':
                          d = 1024.0;
                          break;
                        case 'M':
                          d = 1024.0 * 1024.0;
                          break;
                        case 'G':
                          d = 1024.0 * 1024.0 * 1024.0;
                          break;
                        case 'T':
                          d = 1024.0 * 1024.0 * 1024.0 * 1024.0;
                          break;
                        case 'P':
                          d = 1024.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0;
                          break;
                        default:
                          *units = &scale[0];
                          break;
                      }
                  }
              }
            break;
        }
    }

  return d;
}


/* Custom random number generator */

#define PHI 0x9e3779b9

static u32_t Q[4096], c = 362436;

void
srnd(unsigned int seed)
{
  int i;

  Q[0] = seed;
  Q[1] = seed + PHI;
  Q[2] = seed + PHI + PHI;
  
  for (i = 3; i < 4096; i++)
    Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}

unsigned int
urnd(void)
{
  u64_t t, a = 18782LL;
  static u32_t i = 4095;
  u32_t x, r = 0xfffffffe;
  i = (i + 1) & 4095;
  t = a * Q[i] + c;
  c = (t >> 32);
  x = t + c;
  if (x < c) {
    x++;
    c++;
  }
  return (Q[i] = r - x);
}

u8_t
urnd8()
{
  return urnd();
}

u16_t
urnd16()
{
  return urnd();
}

u32_t
urnd32()
{
  return urnd();
}


double
ddfmod(double x, double y)
{
#ifdef HAVE_FMOD
  return fmod(x, y);
#else
  long int q= x / y;
  return x - q * y;
#endif  
}


/* End of utils.src/utils.cc */
