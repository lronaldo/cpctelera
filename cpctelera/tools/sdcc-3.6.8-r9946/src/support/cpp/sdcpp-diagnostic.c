/* diagnostic subroutines for the SDCC
   Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008,
   2009, 2010 Free Software Foundation, Inc.
   Copyright (C) 2010 Borut Razem
   Contributed by Gabriel Dos Reis <gdr@codesourcery.com>

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
#include "internal.h"
#include "sdcpp.h"

#ifndef _
# define _(msgid) (msgid)
#endif

/* from libcpp/line-map.c */

/* Print the file names and line numbers of the #include commands
   which led to the map MAP, if any, to stderr.  Nothing is output if
   the most recently listed stack is the same as the current one.  */

void
linemap_print_containing_files (struct line_maps *set,
                                const struct line_map *map)
{
  if (MAIN_FILE_P (map) || set->last_listed == map->included_from)
    return;

  set->last_listed = map->included_from;
  map = INCLUDED_FROM (set, map);

  fprintf (stderr,  _("In file included from %s:%u"),
           map->to_file, LAST_SOURCE_LINE (map));

  while (! MAIN_FILE_P (map))
    {
      map = INCLUDED_FROM (set, map);
      /* Translators note: this message is used in conjunction
         with "In file included from %s:%ld" and some other
         tricks.  We want something like this:

         | In file included from sys/select.h:123,
         |                  from sys/types.h:234,
         |                  from userfile.c:31:
         | bits/select.h:45: <error message here>

         with all the "from"s lined up.
         The trailing comma is at the beginning of this message,
         and the trailing colon is not translated.  */
      fprintf (stderr, _(",\n                 from %s:%u"),
               map->to_file, LAST_SOURCE_LINE (map));
    }

  fputs (":\n", stderr);
}

/* from libcpp/errors.c */

/* Print the logical file location (LINE, COL) in preparation for a
   diagnostic.  Outputs the #include chain if it has changed.  A line
   of zero suppresses the include stack, and outputs the program name
   instead.  */
static void
print_location (cpp_reader *pfile, source_location line, unsigned int col)
{
  if (line == 0)
    fprintf (stderr, "%s: ", progname);
  else
    {
      const struct line_map *map;
      unsigned int lin;

      map = linemap_lookup (pfile->line_table, line);
      linemap_print_containing_files (pfile->line_table, map);

      lin = SOURCE_LINE (map, line);
      if (col == 0)
	{
	  col = SOURCE_COLUMN (map, line);
	  if (col == 0)
	    col = 1;
	}

      if (lin == 0)
	fprintf (stderr, "%s:", map->to_file);
      else
	fprintf (stderr, "%s:%u:%u:", map->to_file, lin, col);

      fputc (' ', stderr);
    }
}

/* from c-common.c */

/* Callback from cpp_error for PFILE to print diagnostics from the
   preprocessor.  The diagnostic is of type LEVEL, at location
   LOCATION unless this is after lexing and the compiler's location
   should be used instead, with column number possibly overridden by
   COLUMN_OVERRIDE if not zero; MSG is the translated message and AP
   the arguments.  Returns true if a diagnostic was emitted, false
   otherwise.  */

bool
c_cpp_error (cpp_reader *pfile ATTRIBUTE_UNUSED, int level, int reason ATTRIBUTE_UNUSED,
	     location_t location, unsigned int column_override,
	     const char *msg, va_list *ap)
{
/* Moved here from cpplib.h */
/* Extracts a diagnostic level from an int.  */
#define CPP_DL_EXTRACT(l)       (l & 0xf)
/* Nonzero if a diagnostic level is one of the warnings.  */
#define CPP_DL_WARNING_P(l)     (CPP_DL_EXTRACT (l) >= CPP_DL_WARNING \
                                 && CPP_DL_EXTRACT (l) <= CPP_DL_PEDWARN)

  switch (level)
    {
    case CPP_DL_WARNING:
    case CPP_DL_PEDWARN:
      if (cpp_in_system_header (pfile)
	  && ! warn_system_headers)
	return false;
      /* Fall through.  */

    case CPP_DL_WARNING_SYSHDR:
      if (warnings_are_errors
	  || (level == CPP_DL_PEDWARN && flag_pedantic_errors))
	{
	  level = CPP_DL_ERROR;
	  ++errorcount;
	}
      else if (inhibit_warnings)
	return false;
      break;

    case CPP_DL_ERROR:
      /* ICEs cannot be inhibited.  */
    case CPP_DL_ICE:
      ++errorcount;
      break;
    }

  print_location (pfile, location, column_override);

  if (CPP_DL_WARNING_P (level))
    fputs (_("warning: "), stderr);
  else if (level == CPP_DL_ICE)
    fputs (_("internal error: "), stderr);
  else if (level == CPP_DL_FATAL)
    fputs (_("fatal error: "), stderr);
  else
    fputs (_("error: "), stderr);

  vfprintf (stderr, _(msg), *ap);
  putc ('\n', stderr);

  if (level == CPP_DL_FATAL) {
    fputs(_("compilation terminated.\n"), stderr);
    exit (FATAL_EXIT_CODE);
  }

  return true;
}
