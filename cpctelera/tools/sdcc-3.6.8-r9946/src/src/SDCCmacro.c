/*-------------------------------------------------------------------------
  SDCCmain.c - Macro support code.

             Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1999)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/

#include "common.h"
#include "dbuf_string.h"

char *
eval_macros (hTab * pvals, const char *pfrom)
{
  bool fdidsomething = FALSE;
  char quote = '\0';
  struct dbuf_s dbuf;

  assert (pvals);
  assert (pfrom);

  dbuf_init (&dbuf, 256);
  while (*pfrom)
    {
      switch (*pfrom)
        {
        case '"':
        case '\'':
          if (quote != '\0')
            {
              /* write previous quote */
              dbuf_append_char (&dbuf, quote);
            }
          quote = *pfrom++;
          break;

        case '{':
          {
            const char *pend = ++pfrom;
            const char *pval;
            char *name;

            /* Find the end of macro */
            while (*pend && '}' != *pend)
              {
                pend++;
              }
            if ('}' != *pend)
              {
                wassertl (0, "Unterminated macro expansion");
              }

            name = Safe_strndup (pfrom, pend - pfrom);

            /* Look up the value in the hash table */
            pval = shash_find (pvals, name);
            Safe_free (name);

            if (NULL == pval)
              {
                /* Empty macro value */
                if ('\0' != quote)
                  {
                    /* It was a quote */
                    if (pend[1] == quote)
                      {
                        /* Start quote equals end quote: skip both */
                        ++pend;
                      }
                    else
                      {
                        /* Start quote not equals end quote: add both */
                        dbuf_append_char (&dbuf, quote);
                      }
                  }
              }
            else
              {
                if ('\0' != quote)
                  {
                    dbuf_append_char (&dbuf, quote);
                  }
                dbuf_append_str (&dbuf, pval);
                fdidsomething = TRUE;
              }

            quote = '\0';
            pfrom = pend + 1;
          }
          break;

        default:
          if ('\0' != quote)
            {
              dbuf_append_char (&dbuf, quote);
              quote = '\0';
            }

          dbuf_append_char (&dbuf, *pfrom++);
        }
    }

  if ('\0' != quote)
    {
      dbuf_append_char (&dbuf, quote);
    }


  /* If we did something then recursivly expand any expanded macros */
  if (fdidsomething)
    {
      char *ret = eval_macros (pvals, dbuf_c_str (&dbuf));
      dbuf_destroy (&dbuf);
      return ret;
    }

  return dbuf_detach_c_str (&dbuf);
}

char *
mvsprintf (hTab * pvals, const char *pformat, va_list ap)
{
  char *p;
  struct dbuf_s dbuf;

  dbuf_init (&dbuf, 256);

  /* Recursivly evaluate all the macros in the string */
  p = eval_macros (pvals, pformat);

  /* Evaluate all the arguments */
  dbuf_vprintf (&dbuf, p, ap);
  Safe_free (p);

  /* Recursivly evaluate any macros that were used as arguments */
  p = eval_macros (pvals, dbuf_c_str (&dbuf));
  dbuf_destroy (&dbuf);
  return p;
}

char *
msprintf (hTab * pvals, const char *pformat, ...)
{
  va_list ap;
  char *pret;

  va_start (ap, pformat);

  pret = mvsprintf (pvals, pformat, ap);

  va_end (ap);

  return pret;
}

void
mfprintf (FILE * fp, hTab * pvals, const char *pformat, ...)
{
  va_list ap;
  char *p;

  va_start (ap, pformat);

  p = mvsprintf (pvals, pformat, ap);

  va_end (ap);

  fputs (p, fp);
  Safe_free (p);
}
