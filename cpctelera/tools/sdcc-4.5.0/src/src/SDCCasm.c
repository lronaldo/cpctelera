/*-------------------------------------------------------------------------
  SDCCasm.c - header file for all types of stuff to support different assemblers.

  Copyright (C) 2000, Michael Hope <michaelh@juju.net.nz>

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
-------------------------------------------------------------------------*/

/* Provides output functions that modify the output string
   based on the input tokens and the assembler token mapping
   specification loaded.

   Note that the functions below only handle digit format modifiers.
   eg %02X is ok, but %lu and %.4u will fail.

   A 'token' is like !blah or %24f and is under the programmers
   control. */

#include <errno.h>

#include "common.h"
#include "dbuf_string.h"

static hTab *_h;

const char *
FileBaseName (const char *fileFullName)
{
  const char *p;

  if (!fileFullName)
    {
      return "unknown";
    }

  for (p = fileFullName + strlen (fileFullName) - 1;
       p >= fileFullName && (*p != '/' && *p != '\\' && *p != ':');
       --p)
    ;

  return p + 1;
}

void
dbuf_tvprintf (struct dbuf_s *dbuf, const char *format, va_list ap)
{
  /*
     Under Linux PPC va_list is a structure instead of a primitive type,
     and doesn't like being passed around.  This version turns everything
     into one function.

     Supports:
      !tokens
      %[CIFN] - special formats with no argument (ie list isnt touched)
      All of the system formats

     This is acheived by expanding the tokens and zero arg formats into
     one big format string, which is passed to the native printf.
   */
  static int count;
  struct dbuf_s tmpDBuf;
  const char *noTokens;
  const char *sz = format;
  const char *begin = NULL;

  /* First pass: expand all of the macros */
  dbuf_init (&tmpDBuf, INITIAL_INLINEASM);

  while (*sz)
    {
      if (*sz == '!')
        {
          /* Start of a token.  Search until the first
             [non alpha, *] and call it a token. */
          const char *t;
          struct dbuf_s token;

          if (begin)
            {
              /* copy what we have until now */
              dbuf_append (&tmpDBuf, begin, sz - begin);
              begin = NULL;
            }

          dbuf_init (&token, 64);
          ++sz;
          while (isalpha ((unsigned char) *sz) || *sz == '*')
            {
              dbuf_append (&token, sz++, 1);
            }
          /* Now find the token in the token list */
          if ((t = shash_find (_h, dbuf_c_str (&token))))
            {
              dbuf_append_str (&tmpDBuf, t);
            }
          else
            {
              /* Token not recognized as a valid macro: macro is not expanded */
              dbuf_append_char (&tmpDBuf, '!');
              dbuf_append (&tmpDBuf, dbuf_get_buf (&token), dbuf_get_length (&token));
            }
          dbuf_destroy (&token);
        }
      else
        {
          if (!begin)
            begin = sz;
          ++sz;
        }
    }

  if (begin)
    {
      /* copy what we have until now */
      dbuf_append (&tmpDBuf, begin, sz - begin);
      begin = NULL;
    }

  /* Second pass: Expand any macros that we own */
  sz = noTokens = dbuf_detach_c_str (&tmpDBuf);

  /* recycle tmpDBuf */
  dbuf_init (&tmpDBuf, INITIAL_INLINEASM);

  while (*sz)
    {
      if (*sz == '%')
        {
          if (begin)
            {
              /* copy what we have until now */
              dbuf_append (&tmpDBuf, begin, sz - begin);
              begin = NULL;
            }

          // See if its one that we handle.
          ++sz;
          switch (*sz)
            {
            case 'C':
              // Code segment name.
              dbuf_append_str (&tmpDBuf, CODE_NAME);
              ++sz;
              break;

            case 'F':
              // Source file name.
              dbuf_append_str (&tmpDBuf, fullSrcFileName);
              ++sz;
              break;

            case 'N':
              // Current function name.
              dbuf_append_str (&tmpDBuf, currFunc->rname);
              ++sz;
              break;

            case 'I':
              // Unique ID.
              dbuf_printf (&tmpDBuf, "%u", ++count);
              ++sz;
              break;

            default:
              // Not one of ours.  Copy until the end.
              dbuf_append_char (&tmpDBuf, '%');
              while (!isalpha ((unsigned char) *sz))
                dbuf_append_char (&tmpDBuf, *sz++);

              dbuf_append_char (&tmpDBuf, *sz++);
              break;
            }
        }
      else
        {
          if (!begin)
            begin = sz;
          ++sz;
        }
    }

  if (begin)
    {
      /* copy what we have until now */
      dbuf_append (&tmpDBuf, begin, sz - begin);
      begin = NULL;
    }

  dbuf_free (noTokens);

  dbuf_vprintf (dbuf, dbuf_c_str (&tmpDBuf), ap);

  dbuf_destroy (&tmpDBuf);
}

void
dbuf_tprintf (struct dbuf_s *dbuf, const char *szFormat, ...)
{
  va_list ap;
  va_start (ap, szFormat);
  dbuf_tvprintf (dbuf, szFormat, ap);
  va_end (ap);
}

void
tfprintf (FILE *fp, const char *szFormat, ...)
{
  va_list ap;
  struct dbuf_s dbuf;

  dbuf_init (&dbuf, INITIAL_INLINEASM);

  va_start (ap, szFormat);
  dbuf_tvprintf (&dbuf, szFormat, ap);
  va_end (ap);

  fwrite (dbuf_get_buf (&dbuf), 1, dbuf_get_length (&dbuf), fp);
  dbuf_destroy (&dbuf);
}

void
asm_addTree (const ASM_MAPPINGS * pMappings)
{
  const ASM_MAPPING *pMap;

  /* Traverse down first */
  if (pMappings->pParent)
    asm_addTree (pMappings->pParent);
  pMap = pMappings->pMappings;
  while (pMap->szKey && pMap->szValue)
    {
      shash_add (&_h, pMap->szKey, pMap->szValue);
      pMap++;
    }
}

/*-----------------------------------------------------------------*/
/* printILine - return the readable i-code for this ic             */
/*-----------------------------------------------------------------*/
const char *
printILine (iCode * ic)
{
  struct dbuf_s tmpBuf;
  iCodeTable *icTab = getTableEntry (ic->op);

  wassert (icTab);

  dbuf_init (&tmpBuf, 1024);

  if (INLINEASM == ic->op)
    dbuf_append_str (&tmpBuf, "inline");
  else
    {
      /* stuff the temporary file with the readable icode */
      icTab->iCodePrint (&tmpBuf, ic, icTab->printName);
    }

  /* null terminate the buffer */
  dbuf_chomp (&tmpBuf);

  return dbuf_detach_c_str (&tmpBuf);
}

/*-----------------------------------------------------------------*/
/* skipLine - skip the line from file infp                         */
/*-----------------------------------------------------------------*/
static int
skipLine (FILE * infp)
{
  int c;
  static char is_eof = 0;
  size_t len = 0;

  if (is_eof)
    return 0;

  while ((c = getc (infp)) != '\n' && EOF != c)
    ++len;

  if (EOF == c)
    {
      if (len)
        {
          /* EOF in the middle of the line */
          is_eof = 1;
          return 1;
        }
      else
        return 0;
    }
  else
    return 1;
}

/*-----------------------------------------------------------------*/
/* printCLine - return the c-code for this lineno                  */
/*-----------------------------------------------------------------*/
/* int rewinds=0; */
const char *
printCLine (const char *srcFile, int lineno)
{
  static FILE *inFile = NULL;
  static struct dbuf_s line;
  static struct dbuf_s lastSrcFile;
  static char dbufInitialized = 0;
  static int inLineNo = 0;
  size_t len;

  if (!dbufInitialized)
    {
      dbuf_init (&line, 1024);
      dbuf_init (&lastSrcFile, PATH_MAX);
      dbufInitialized = 1;
    }
  else
    {
      /* empty the dynamic buffer */
      dbuf_set_length (&line, 0);
    }

  if (inFile)
    {
      if (strcmp (dbuf_c_str (&lastSrcFile), srcFile) != 0)
        {
          fclose (inFile);
          inFile = NULL;
          inLineNo = 0;
          dbuf_set_length (&lastSrcFile, 0);
          dbuf_append_str (&lastSrcFile, srcFile);
        }
    }

  if (!inFile)
    {
      if (!(inFile = fopen (srcFile, "r")))
        {
          /* can't open the file:
             don't panic, just return the error message */
          dbuf_printf (&line, "ERROR: %s", strerror (errno));

          return dbuf_detach_c_str (&line);
        }
      else
        {
          dbuf_set_length (&lastSrcFile, 0);
          dbuf_append_str (&lastSrcFile, srcFile);
        }
    }

  if (inLineNo > lineno)
    {
      /* past the lineno: rewind the file pointer */
      rewind (inFile);
      inLineNo = 0;
      /* rewinds++; */
    }

  /* skip lines until lineno */
  while (inLineNo + 1 < lineno)
    {
      if (!skipLine (inFile))
        goto err_no_line;
      ++inLineNo;
    }

  /* get the line */
  if (0 != (len = dbuf_getline (&line, inFile)))
    {
      const char *inLineString;

      ++inLineNo;

      /* remove the trailing NL */
      dbuf_chomp (&line);

      inLineString = dbuf_detach_c_str (&line);

      /* skip leading spaces */
      while (isspace (*inLineString))
        ++inLineString;

      return inLineString;
    }

err_no_line:
  dbuf_printf (&line, "ERROR: no line number %d in file %s", lineno, srcFile);

  return dbuf_detach_c_str (&line);
}

static const ASM_MAPPING _asxxxx_mapping[] = {
  {"labeldef", "%s::"},
  {"slabeldef", "%s:"},
  {"tlabeldef", "%05d$:"},
  {"tlabel", "%05d$"},
  {"immed", "#"},
  {"zero", "#0x00"},
  {"one", "#0x01"},
  {"area", ".area %s"},
  {"areacode", ".area %s"},
  {"areadata", ".area %s"},
  {"areahome", ".area %s"},
  {"ascii", ".ascii \"%s\""},
  {"ds", ".ds %d"},
  {"db", ".db"},
  {"dbs", ".db %s"},
  {"dw", ".dw"},
  {"dws", ".dw %s"},
  {"constbyte", "0x%02x"},
  {"constword", "0x%04x"},
  {"immedword", "#0x%04x"},
  {"immedbyte", "#0x%02x"},
  {"hashedstr", "#%s"},
  {"lsbimmeds", "#<(%s)"},
  {"msbimmeds", "#>(%s)"},
  {"module", ".module %s"},
  {"global", ".globl %s"},
  {"fileprelude", ""},
  {"functionheader",
   "; ---------------------------------\n"
   "; Function %s\n"
   "; ---------------------------------"},
  {"functionlabeldef", "%s:"},
  {"globalfunctionlabeldef", "%s::"},
  {"bankimmeds", "b%s"},
  {"hashedbankimmeds", "#b%s"},
  {"los", "(%s & 0xFF)"},
  {"his", "(%s >> 8)"},
  {"hihis", "(%s >> 16)"},
  {"hihihis", "(%s >> 24)"},
  {"lod", "(%d & 0xFF)"},
  {"hid", "(%d >> 8)"},
  {"hihid", "(%d >> 16)"},
  {"hihihid", "(%d >> 24)"},
  {"lol", "(%05d$ & 0xFF)"},
  {"hil", "(%05d$ >> 8)"},
  {"hihil", "(%05d$ >> 16)"},
  {"hihihil", "(%05d$ >> 24)"},
  {"equ", "="},
  {"bequ", "b%s = %i"},
  {"org", ".org 0x%04X"},
  {NULL, NULL}
};

static const ASM_MAPPING _asxxxx_smallpdk_mapping[] = {
  {"labeldef", "%s::"},
  {"slabeldef", "%s:"},
  {"tlabeldef", "%05d$:"},
  {"tlabel", "%05d$"},
  {"immed", "#"},
  {"zero", "#0x00"},
  {"one", "#0x01"},
  {"area", ".area %s"},
  {"areacode", ".area %s"},
  {"areadata", ".area %s"},
  {"areahome", ".area %s"},
  {"ascii", ".ascii \"%s\""},
  {"ds", ".ds %d"},
  {"db", "ret"},
  {"dbs", "ret %s"},
  {"dw", ".dw"},
  {"dws", ".dw %s"},
  {"constbyte", "#0x%02x"},
  {"constword", "0x%04x"},
  {"immedword", "#0x%04x"},
  {"immedbyte", "#0x%02x"},
  {"hashedstr", "#%s"},
  {"lsbimmeds", "#<(%s)"},
  {"msbimmeds", "#>(%s)"},
  {"module", ".module %s"},
  {"global", ".globl %s"},
  {"fileprelude", ""},
  {"functionheader",
   "; ---------------------------------\n"
   "; Function %s\n"
   "; ---------------------------------"},
  {"functionlabeldef", "%s:"},
  {"globalfunctionlabeldef", "%s::"},
  {"bankimmeds", "b%s"},
  {"hashedbankimmeds", "#b%s"},
  {"los", "(%s & 0xFF)"},
  {"his", "(%s >> 8)"},
  {"hihis", "(%s >> 16)"},
  {"hihihis", "(%s >> 24)"},
  {"lod", "(%d & 0xFF)"},
  {"hid", "(%d >> 8)"},
  {"hihid", "(%d >> 16)"},
  {"hihihid", "(%d >> 24)"},
  {"lol", "(%05d$ & 0xFF)"},
  {"hil", "(%05d$ >> 8)"},
  {"hihil", "(%05d$ >> 16)"},
  {"hihihil", "(%05d$ >> 24)"},
  {"equ", "="},
  {"org", ".org 0x%04X"},
  {NULL, NULL}
};

static const ASM_MAPPING _gas_mapping[] = {
  {"labeldef", "%s:"},
  {"slabeldef", "%s:"},
  {"tlabeldef", "%05d$:"},
  {"tlabel", "%05d$"},
  {"immed", "#"},
  {"zero", "#0x00"},
  {"one", "#0x01"},
  {"area", ".section %s"},
  {"areacode", ".section %s,\"ax\""},
  {"areadata", ".section %s,\"rw\""},
  {"areahome", ".section %s,\"ax\""},
  {"ascii", ".ascii\t\"%s\""},
  {"ds", ".ds\t%d"},
  {"db", ".db"},
  {"dbs", ".db\t%s"},
  {"dw", ".dw"},
  {"dws", ".dw\t%s"},
  {"constbyte", "0x%02X"},
  {"constword", "0x%04X"},
  {"immedword", "#0x%04X"},
  {"immedbyte", "#0x%02X"},
  {"hashedstr", "#%s"},
  {"lsbimmeds", "#<%s"},
  {"msbimmeds", "#>%s"},
  {"module", ".file\t\"%s.c\""},
  {"global", ".globl\t%s"},
  {"extern", ".extern\t%s"},
  {"fileprelude", ""},
  {"functionheader",
   "; ---------------------------------\n"
   "; Function %s\n"
   "; ---------------------------------"},
  {"functionlabeldef", "%s:"},
  {"globalfunctionlabeldef", "%s:"},
  {"bankimmeds", "b%s"},
  {"hashedbankimmeds", "#b%s"},
  {"los", "%s & 0xFF"},
  {"his", "%s >> 8"},
  {"hihis", "%s >> 16"},
  {"hihihis", "%s >> 24"},
  {"lod", "%d & 0xFF"},
  {"hid", "%d >> 8"},
  {"hihid", "%d >> 16"},
  {"hihihid", "%d >> 24"},
  {"lol", "%05d$ & 0xFF"},
  {"hil", "%05d$ >> 8"},
  {"hihil", "%05d$ >> 16"},
  {"hihihil", "%05d$ >> 24"},
  {NULL, NULL}
};

static const ASM_MAPPING _a390_mapping[] = {
  {"labeldef", "%s:"},
  {"slabeldef", "%s:"},
  {"tlabeldef", "L%05d:"},
  {"tlabel", "L%05d"},
  {"immed", "#"},
  {"zero", "#0"},
  {"one", "#1"},
  {"area", "; SECTION NOT SUPPORTED"},
  {"areacode", "; SECTION NOT SUPPORTED"},
  {"areadata", "; SECTION NOT SUPPORTED"},
  {"areahome", "; SECTION NOT SUPPORTED"},
  {"ascii", "db \"%s\""},
  {"ds", "; STORAGE NOT SUPPORTED"},
  {"db", "db"},
  {"dbs", "db \"%s\""},
  {"dw", "dw"},
  {"dws", "dw %s"},
  {"constbyte", "0%02xh"},
  {"constword", "0%04xh"},
  {"immedword", "#0%04Xh"},
  {"immedbyte", "#0%02Xh"},
  {"hashedstr", "#%s"},
  {"lsbimmeds", "#<%s"},
  {"msbimmeds", "#>%s"},
  {"module", "; .file \"%s.c\""},
  {"global", "; .globl %s"},
  {"fileprelude", ""},
  {"functionheader",
   "; ---------------------------------\n"
   "; Function %s\n"
   "; ---------------------------------"},
  {"functionlabeldef", "%s:"},
  {"globalfunctionlabeldef", "%s::"},
  {"bankimmeds", "b%s"},
  {"hashedbankimmeds", "#b%s"},
  {"los", "(%s & 0FFh)"},
  {"his", "((%s / 256) & 0FFh)"},
  {"hihis", "((%s / 65536) & 0FFh)"},
  {"hihihis", "((%s / 16777216) & 0FFh)"},
  {"lod", "(%d & 0FFh)"},
  {"hid", "((%d / 256) & 0FFh)"},
  {"hihid", "((%d / 65536) & 0FFh)"},
  {"hihihid", "((%d / 16777216) & 0FFh)"},
  {"lol", "(L%05d & 0FFh)"},
  {"hil", "((L%05d / 256) & 0FFh)"},
  {"hihil", "((L%05d / 65536) & 0FFh)"},
  {"hihihil", "((L%09d / 16777216) & 0FFh)"},
  {"equ", " equ"},
  {"org", ".org 0x%04X"},
  {NULL, NULL}
};

const ASM_MAPPINGS asm_asxxxx_mapping = {
  NULL,
  _asxxxx_mapping
};

const ASM_MAPPINGS asm_asxxxx_smallpdk_mapping = {
  NULL,
  _asxxxx_smallpdk_mapping
};

const ASM_MAPPINGS asm_gas_mapping = {
  NULL,
  _gas_mapping
};

const ASM_MAPPINGS asm_a390_mapping = {
  NULL,
  _a390_mapping
};

