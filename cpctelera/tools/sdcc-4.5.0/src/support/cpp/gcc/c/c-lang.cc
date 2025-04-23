/* Language-specific hook definitions for C front end.
   Copyright (C) 1991-2022 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */


#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "c-tree.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "c-objc-common.h"

#include "../c-family/c-pragma.h" // sdcpp BUG
#include "../c-family/c-common.h" // sdcpp BUG

enum c_language_kind c_language = clk_c;
#if 0 // sdcpp




/* Lang hooks common to C and ObjC are declared in c-objc-common.h;
   consequently, there should be very few hooks below.  */

#undef LANG_HOOKS_NAME
#define LANG_HOOKS_NAME "GNU C"
#endif

#define untested() (fprintf(stderr,"@@#\n@@@:%s:%u:%s\n", \
			   __FILE__, __LINE__, __func__))

#if 1 
/* SDCC specific
   pedantic_parse_number pragma */
static void
do_pragma_pedantic_parse_number (cpp_reader *pfile)
{ untested();
    const cpp_token *tok = cpp_get_token (pfile);
	 cpp_options* opts = cpp_get_options(pfile);

  if (tok->type == CPP_PLUS)
    {
      ++opts->pedantic_parse_number;
    }
  else if (tok->type == CPP_MINUS)
    {
      --opts->pedantic_parse_number;
    }
  else
    {
      cpp_error (pfile, CPP_DL_ERROR,
                 "invalid #pragma pedantic_parse_number directive, need '+' or '-'");
    }
}

/* SDCC _asm specific
   switch _asm block preprocessing on / off */
static void
do_pragma_preproc_asm (cpp_reader *pfile)
{ untested();
  const cpp_token *tok = cpp_get_token (pfile);
  cpp_options* opts = cpp_get_options(pfile);

  if (tok->type == CPP_PLUS)
    {
      ++opts->preproc_asm;
    }
  else if (tok->type == CPP_MINUS)
    {
      --opts->preproc_asm;
    }
  else
    {
      cpp_error (pfile, CPP_DL_ERROR,
                 "invalid #pragma preproc_asm directive, need '+' or '-'");
    }
}

#endif
#undef LANG_HOOKS_INIT
#define LANG_HOOKS_INIT c_objc_common_init
#undef LANG_HOOKS_INIT_OPTIONS
void
sdcpp_init_options (unsigned int decoded_options_count ATTRIBUTE_UNUSED,
		  struct cl_decoded_option *decoded_options ATTRIBUTE_UNUSED)
{
	c_common_init_options(decoded_options_count, decoded_options);

#if 0 // TODO
  /* Kevin abuse for SDCC. */
  cpp_register_pragma(parse_in, 0, "sdcc_hash", do_pragma_sdcc_hash, false);
#endif

  /* SDCC _asm specific */
  cpp_register_pragma(parse_in, 0, "preproc_asm", do_pragma_preproc_asm, false);
  /* SDCC specific */
  cpp_register_pragma(parse_in, 0, "pedantic_parse_number", do_pragma_pedantic_parse_number, false);
}

// call the above instead of c_common_init_options
#define LANG_HOOKS_INIT_OPTIONS sdcpp_init_options

#if 0 // sdcpp
#undef LANG_HOOKS_INIT_TS
#define LANG_HOOKS_INIT_TS c_common_init_ts

#if CHECKING_P
#undef LANG_HOOKS_RUN_LANG_SELFTESTS
#define LANG_HOOKS_RUN_LANG_SELFTESTS selftest::run_c_tests
#endif /* #if CHECKING_P */

#undef LANG_HOOKS_GET_SUBSTRING_LOCATION
#define LANG_HOOKS_GET_SUBSTRING_LOCATION c_get_substring_location

#endif
/* Each front end provides its own lang hook initializer.  */
struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;

#if CHECKING_P

namespace selftest {

/* Implementation of LANG_HOOKS_RUN_LANG_SELFTESTS for the C frontend.  */

void
run_c_tests (void)
{
  /* Run selftests shared within the C family.  */
  c_family_tests ();

  /* Additional C-specific tests.  */
}

} // namespace selftest

#endif /* #if CHECKING_P */

// sdcpp #include "gtype-c.h"
