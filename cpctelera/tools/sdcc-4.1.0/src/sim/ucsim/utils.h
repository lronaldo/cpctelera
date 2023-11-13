/*
 * Simulator of microcontrollers (utils.h)
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

#ifndef UTILS_HEADER
#define UTILS_HEADER

#include <stdio.h>
#include <stdarg.h>

// prj
#include "stypes.h"
#include "charscl.h"
#include "fiocl.h"


//#define TRACE printf
#define TRACE   1 ? (void)0 : (*(void (*)(const char *, ...))0)


extern int get_sub_opt(char **option,
		       const char * const *tokens,
		       char **valuep);
extern const char *get_id_string(struct id_element *ids, int id);
extern const char *get_id_string(struct id_element *ids, int id, const char *def);
extern int get_string_id(struct id_element *ids, char *str);
extern int get_string_id(struct id_element *ids, char *str, int def);
extern char *vformat_string(const char *format, va_list ap);
extern char *format_string(const char *format, ...);
extern void print_char_octal(char c, FILE *f);
extern const char *object_name(class cl_base *o);
extern char *case_string(enum letter_case lcase, const char *str);
extern chars cbin(long data, int bits);

extern double dnow(void);

extern int strispn(char *s, char c);
extern bool strend(const char *search_in, const char *what);
extern bool valid_sym_name(char *s);

extern bool is_hex_file(class cl_f *f);
extern bool is_asc_file(class cl_f *f);
extern bool is_omf_file(class cl_f *f);
extern bool is_cdb_file(class cl_f *f);

extern chars colopt2ansiseq(char *opt);


#endif

/* End of utils.h */
