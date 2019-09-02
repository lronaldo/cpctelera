/*-------------------------------------------------------------------------
  device.c - Accomodates subtle variations in PIC16 devices

  Copyright (C) 2000, Scott Dattalo scott@dattalo.com
  PIC16 port:
  Copyright (C) 2002, Martin Dubuc m.dubuc@rogers.com

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

#include <stdio.h>

#include "common.h"   // Include everything in the SDCC src directory
#include "newalloc.h"
#include "dbuf_string.h"

#include "main.h"
#include "pcode.h"
#include "ralloc.h"
#include "device.h"

void pic16_printIval (symbol * sym, sym_link * type, initList * ilist, char ptype, void *p);
extern void pic16_pCodeConstString (char *name, const char *value, unsigned length);

stats_t statistics = { 0, 0, 0, 0 };

#define DEVICE_FILE_NAME    "pic16devices.txt"

static PIC16_device default_device = {
  { "p18f452", "18f452", "pic18f452", "f452" },
  0x600,
  0x80,
  { /* configuration words */
    0x300001, 0x30000d,
    { { 0x27, 0, 0xff } /* 1 */ , { 0x0f, 0, 0xff } /* 2 */ ,
      { 0x0f, 0, 0xff } /* 3 */ , {  -1 , 0, 0xff } /* 4 */ ,
      { 0x01, 0, 0xff } /* 5 */ , { 0x85, 0, 0xff } /* 6 */ ,
      {  -1 , 0, 0xff } /* 7 */ , { 0x0f, 0, 0xff } /* 8 */ ,
      { 0xc0, 0, 0xff } /* 9 */ , { 0x0f, 0, 0xff } /* a */ ,
      { 0xe0, 0, 0xff } /* b */ , { 0x0f, 0, 0xff } /* c */ ,
      { 0x40, 0, 0xff } /* d */ }
  },
  { /* ID locations */
    0x200000, 0x200007,
    { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
      { 0, 0 }, { 0, 0 }, { 0, 0 } }
  },
  0,
  NULL
};

PIC16_device *pic16 = &default_device;
static PIC16_device *devices = NULL;

extern set *includeDirsSet;
extern set *userIncDirsSet;

extern char *iComments2;

void
pic16_dump_equates (FILE *of, set *equs)
{
  reg_info *r;

  r = setFirstItem (equs);
  if (!r)
    return;

  fprintf (of, "\n%s", iComments2);
  fprintf (of, ";\tEquates to used internal registers\n");
  fprintf (of, "%s", iComments2);

  for (; r; r = setNextItem (equs))
    {
      fprintf (of, "%s\tequ\t0x%03x\n", r->name, r->address);
    } // for
}


void
pic16_dump_access (FILE *of, set *section)
{
  reg_info *r;

  r = setFirstItem (section);
  if (!r)
    return;

  fprintf (of, "%s", iComments2);
  fprintf (of, ";\tAccess bank symbols\n");
  fprintf (of, "%s", iComments2);

  fprintf (of, "\tudata_acs\n");
  for (; r; r = setNextItem (section))
    {
      fprintf (of, "%s\tres\t%d\n", r->name, r->size);
      statistics.adsize += r->size;
    } // for
}

int
regCompare (const void *a, const void *b)
{
  const reg_info *const *i = a;
  const reg_info *const *j = b;

  /* Sort primarily by the address ... */
  if ((*i)->address > (*j)->address)
    return (1);

  if ((*i)->address < (*j)->address)
    return (-1);

  /* ... and secondarily by size. */
  /* Register size sorting may have strange results, use with care! */
  if ((*i)->size > (*j)->size)
    return (1);

  if ((*i)->size < (*j)->size)
    return (-1);

  /* Finally, if in same address and same size, sort by name. */
  return (strcmp ((*i)->name, (*j)->name));
}

int
symCompare (const void *a, const void *b)
{
  const symbol *const *i = a;
  const symbol *const *j = b;

  /* Sort primarily by the address ... */
  if (SPEC_ADDR ((*i)->etype) > SPEC_ADDR ((*j)->etype))
    return (1);

  if (SPEC_ADDR ((*i)->etype) < SPEC_ADDR ((*j)->etype))
    return (-1);

  /* ... and secondarily by size. */
  /* Register size sorting may have strange results, use with care! */
  if (getSize ((*i)->etype) > getSize ((*j)->etype))
    return (1);

  if (getSize ((*i)->etype) < getSize ((*j)->etype))
    return (-1);

  /* Finally, if in same address and same size, sort by name. */
  return (strcmp ((*i)->rname, (*j)->rname));
}

void
pic16_dump_usection (FILE *of, set *section, int fix)
{
  static int abs_usection_no = 0;
  static unsigned int usection_no = 0;
  reg_info *r, *rprev;
  unsigned int init_addr, i;
  reg_info **rlist;
  reg_info *r1;

  /* put all symbols in an array */
  if (!elementsInSet (section))
    return;

  rlist = Safe_calloc (elementsInSet (section), sizeof (reg_info *));
  r = rlist[0];
  i = 0;
  for (rprev = setFirstItem (section); rprev; rprev = setNextItem (section))
    {
      rlist[i] = rprev;
      i++;
    } // for

  if (!i)
    {
      if (rlist)
        Safe_free (rlist);

      return;
    } // if

  /* sort symbols according to their address */
  qsort (rlist, i, sizeof (reg_info *), regCompare);

  if (!fix)
    {
#define EMIT_SINGLE_UDATA_SECTION       0
#if EMIT_SINGLE_UDATA_SECTION
      fprintf (of, "\n\n\tudata\n");
      for (r = setFirstItem (section); r; r = setNextItem (section))
        {
          fprintf (of, "%s\tres\t%d\n", r->name, r->size);
          statistics.udsize += r->size;
        } // for
#else
      for (r = setFirstItem (section); r; r = setNextItem (section))
        {
          //fprintf (of, "\nudata_%s_%s\tudata\n", moduleName, r->name);
          fprintf (of, "\nudata_%s_%u\tudata\n", moduleName, usection_no++);
          fprintf (of, "%s\tres\t%d\n", r->name, r->size);
          statistics.udsize += r->size;
        } // for
#endif
    }
  else
    {
      unsigned int j = 0;
      unsigned int prev_size = 0;

      rprev = NULL;
      init_addr = (rlist[j]->address & 0x0FFF); // warning(s) emitted below
      fprintf (of, "\n\nustat_%s_%02d\tudata\t0X%04X\n", moduleName, abs_usection_no++, (init_addr & 0x0FFF));

      for (j = 0; j < i; j++)
        {
          r = rlist[j];
          r1 = NULL;
          if (j < i - 1)
            r1 = rlist[j + 1];

          init_addr = (r->address & 0x0FFF);
          if (init_addr != r->address)
            {
              fprintf (stderr, "%s: WARNING: Changed address of pinned variable %s from 0x%x to 0x%x\n",
                moduleName, r->name, r->address, init_addr);
            } // if

          if ((rprev && (init_addr != ((rprev->address & 0x0FFF) + prev_size))))
            fprintf (of, "\n\nustat_%s_%02d\tudata\t0X%04X\n", moduleName, abs_usection_no++, init_addr);

          /* XXX: Does not handle partial overlap correctly. */
          if (r1 && (init_addr == (r1->address & 0x0FFF)))
            {
              prev_size = 0;
              fprintf (of, "%-15s\n", r->name);
            }
          else
            {
              prev_size = r->size;
              fprintf (of, "%-15s\tres\t%d\n", r->name, prev_size);
              statistics.udsize += prev_size;
            }

          rprev = r;
        } // for
    } // if

  Safe_free (rlist);
}

void
pic16_dump_gsection (FILE *of, set *sections)
{
  reg_info *r;
  sectName *sname;

  for (sname = setFirstItem (sections); sname; sname = setNextItem (sections))
    {
      if (!strcmp (sname->name, "access"))
        continue;

      fprintf (of, "\n\n%s\tudata\n", sname->name);

      for (r = setFirstItem (sname->regsSet); r; r = setNextItem (sname->regsSet))
        {
#if 0
          fprintf (stderr, "%s:%d emitting variable %s for section %s (%p)\n",
              __FILE__, __LINE__, r->name, sname->name, sname);
#endif
          fprintf (of, "%s\tres\t%d\n", r->name, r->size);
          statistics.udsize += r->size;
        } // for
    } // for
}

void
pic16_dump_isection (FILE *of, set *section, int fix)
{
  static int abs_isection_no = 0;
  symbol *s, *sprev;
  unsigned int init_addr, i;
  symbol **slist;

  /* put all symbols in an array */
  if (!elementsInSet (section))
    return;

  slist = Safe_calloc (elementsInSet (section), sizeof (symbol *));
  s = slist[0];
  i = 0;
  for (sprev = setFirstItem (section); sprev; sprev = setNextItem (section))
    {
      slist[i] = sprev;
      i++;
    } // for

  if (!i)
    {
      if (slist)
        Safe_free (slist);

      return;
    } // if

  /* sort symbols according to their address */
  qsort (slist, i, sizeof (symbol *), symCompare);

  pic16_initDB ();

  if (!fix)
    {
      fprintf (of, "\n\n\tidata\n");
      for (s = setFirstItem (section); s; s = setNextItem (section))
        {
          if (s->ival)
            {
              fprintf (of, "%s", s->rname);
              pic16_printIval (s, s->type, s->ival, 'f', (void *)of);
              pic16_flushDB ('f', (void *)of);
            }
          else
            {
              if (IS_ARRAY (s->type) && IS_CHAR (s->type->next)
                  && SPEC_CVAL (s->etype).v_char)
                {
                  //fprintf (stderr, "%s:%d printing code string from %s\n", __FILE__, __LINE__, s->rname);
                  pic16_pCodeConstString (s->rname , SPEC_CVAL (s->etype).v_char, getSize (s->type));
                }
              else
                {
                  assert (0);
                } // if
            } // if
        } // for
    }
  else
    {
      unsigned int j = 0;

      sprev = NULL;
      init_addr = SPEC_ADDR (slist[j]->etype);
      fprintf (of, "\n\nistat_%s_%02d\tidata\t0X%04X\n", moduleName, abs_isection_no++, init_addr);

      for (j = 0; j < i; j++)
        {
          s = slist[j];
          init_addr = SPEC_ADDR (s->etype);

          if (sprev && (init_addr > (SPEC_ADDR (sprev->etype) + getSize (sprev->etype))))
            fprintf(of, "\nistat_%s_%02d\tidata\t0X%04X\n", moduleName, abs_isection_no++, init_addr);

          if (s->ival)
            {
              fprintf (of, "%s", s->rname);
              pic16_printIval (s, s->type, s->ival, 'f', (void *)of);
              pic16_flushDB ('f', (void *)of);
            }
          else
            {
              if (IS_ARRAY (s->type) && IS_CHAR (s->type->next)
                  && SPEC_CVAL (s->etype).v_char)
                {
                  //fprintf (stderr, "%s:%d printing code string from %s\n", __FILE__, __LINE__, s->rname);
                  pic16_pCodeConstString (s->rname , SPEC_CVAL (s->etype).v_char, getSize (s->type));
                }
              else
                {
                  assert (0);
                } // if
            } // if

          sprev = s;
        } // for
    } // if

  Safe_free (slist);
}

void
pic16_dump_int_registers (FILE *of, set *section)
{
  reg_info *r, *rprev;
  int i;
  reg_info **rlist;

  /* put all symbols in an array */
  if (!elementsInSet (section))
    return;

  rlist = Safe_calloc (elementsInSet (section), sizeof (reg_info *));
  r = rlist[0];
  i = 0;
  for (rprev = setFirstItem (section); rprev; rprev = setNextItem (section))
    {
      rlist[i] = rprev;
      i++;
    } // for

  if (!i)
    {
      if (rlist)
        Safe_free (rlist);

      return;
    } // if

  /* sort symbols according to their address */
  qsort (rlist, i, sizeof (reg_info *), regCompare);

  fprintf (of, "\n\n; Internal registers\n");

  fprintf (of, "%s\tudata_ovr\t0x0000\n", ".registers");
  for (r = setFirstItem (section); r; r = setNextItem (section))
    {
      fprintf (of, "%s\tres\t%d\n", r->name, r->size);
      statistics.intsize += r->size;
    } // for

  Safe_free (rlist);
}

/**
 * Find the device structure for the named device.
 * Consider usind pic16_find_device() instead!
 *
 * @param   name
 *      a name for the desired device
 * @param   head
 *      a pointer to the head of the list of devices
 * @return
 *      a pointer to the structure for the desired
 *      device, or NULL
 */
static PIC16_device *
find_in_list(const char *name, PIC16_device *head)
{
    int i;

    while (head) {
        for (i = 0; i < 4; i++) {
            if (0 == STRCASECMP(head->name[i], name)) {
                return (head);
            } // if
        } // for

        head = head->next;
    } // while

    return (NULL);
}

/**
 * Print a list of supported devices.
 * If --verbose was given, also emit key characteristics (memory size,
 * access bank split point, address range of SFRs and config words).
 *
 * @param   head
 *      a pointer to the head of the list of devices
 */
static void
pic16_list_devices(PIC16_device *head)
{
    int i = 0;

    if (options.verbose) {
        printf("device        RAM  split       config words\n");
    } // if
    while (head) {
        printf("%-10s  ", head->name[0]);
        if (options.verbose) {
            printf("%5d   0x%02x    0x%06x..0x%06x\n",
                    head->RAMsize,
                    head->acsSplitOfs,
                    head->cwInfo.confAddrStart,
                    head->cwInfo.confAddrEnd);
        } else {
            i++;
            if (0 == (i % 6)) {
                printf("\n");
            } // if
        } // if
        head = head->next;
    } // while
    printf("\n");
}

/**
 * Read a single line from the given file.
 *
 * @param   file
 *      a pointer to the open file to read
 * @return
 *      a pointer to a malloc'ed copy of the (next) line, or NULL
 */
static char *
get_line (FILE *file)
{
  static struct dbuf_s dbuf;
  static int initialized = 0;

  if (!initialized)
    {
      dbuf_init (&dbuf, 129);
      initialized = 1;
    }
  else
    dbuf_set_length (&dbuf, 0);


  if (dbuf_getline (&dbuf, file) != 0)
    {
      dbuf_chomp (&dbuf);
      /* (char *) type cast is an ugly hack since pic16_find_device() modifies the buffer */
      return (char *)dbuf_get_buf (&dbuf);
    }
  else
    {
      dbuf_destroy(&dbuf);
      initialized = 0;
      return NULL;
    }
}

/**
 * Truncate the given string in place (!) at the first '#' character (if any).
 *
 * @param   line
 *      a pointer to the string to truncate
 * @return
 *      a pointer to the truncated string (i.e., line)
 */
static char *
strip_comment (char *line)
{
  char *l = line;
  char c;

  if (!line)
    {
      return (line);
    } // if

  while (0 != (c = *l))
    {
      if ('#' == c)
        {
          *l = 0;
          return (line);
        } // if
      l++;
    } // while

  return (line);
}

/**
 * Report errors in the device specification.
 *
 * @param   msg
 *      a pointer to the detailed message
 */
#define SYNTAX(msg) do {                                \
    fprintf(stderr, "%s:%d: Syntax error: %s\n",        \
            DEVICE_FILE_NAME, lineno, msg);             \
} while (0)

/**
 * Locate and read in the device specification (if required) and
 * return the device structure for the named device.
 *
 * @param   name
 *      a pointer to the name of the desired device
 * @return
 *      a pointer to the device structure, or NULL
 */
static PIC16_device *
pic16_find_device(const char *name)
{
  const char *path;
  char buffer[PATH_MAX];
  char *line, *key;
  const char *sep = " \t\n\r";
  FILE *f = NULL;
  PIC16_device *d = NULL, *template;
  PIC16_device *head = NULL, *tail = NULL;
  set *_sets[] = { userIncDirsSet, includeDirsSet };
  set **sets = &_sets[0];
  int lineno = 0;
  int res, i;
  int val[4];

  if (!devices)
    {
      //printf("%s: searching %s\n", __func__, DEVICE_FILE_NAME);

      // locate the specification file in the include search paths
      for (i = 0; (NULL == f) && (i < 2); i++)
        {
          for (path = setFirstItem(sets[i]);
               (NULL == f) && path;
               path = setNextItem(sets[i]))
            {
              SNPRINTF(&buffer[0], PATH_MAX, "%s%s%s",
                       path, DIR_SEPARATOR_STRING, DEVICE_FILE_NAME);
              //printf("%s: checking %s\n", __func__, &buffer[0]);
              f = fopen(&buffer[0], "r");
            } // for
        } // for
    } // if

  if (devices)
    {
      // list already set up, nothing to do
    }
  else if (NULL == f)
    {
      fprintf(stderr, "ERROR: device list %s not found, specify its path via -I<path>\n",
              DEVICE_FILE_NAME);
      d = &default_device;
    }
  else
    {
      // parse the specification file and construct a linked list of
      // supported devices
      d = NULL;
      while (NULL != (line = get_line(f)))
        {
          strip_comment(line);
          //printf("%s: read %s\n", __func__, line);
          lineno++;
          key = strtok(line, sep);
          if (!key)
            {
              // empty line---ignore
            }
          else if (0 == strcmp(key, "name"))
            {
              // name %<name>s
              if (d)
                {
                  if (tail)
                    {
                      tail->next = d;
                    }
                  else
                    {
                      head = d;
                    } // if
                  tail = d;
                  d = NULL;
                } // if

              res = sscanf(&line[1 + strlen(key)], " %16s", &buffer[3]);
              if ((1 < res) || (3 > strlen(&buffer[3])))
                {
                  SYNTAX("<name> (e.g., 18f452) expected.");
                }
              else
                {
                  d = Safe_alloc(sizeof(PIC16_device));

                  // { "p18f452", "18f452", "pic18f452", "f452" }
                  buffer[0] = 'p';
                  buffer[1] = 'i';
                  buffer[2] = 'c';
                  d->name[3] = Safe_strdup(&buffer[5]);
                  d->name[2] = Safe_strdup(&buffer[0]);
                  d->name[1] = Safe_strdup(&buffer[3]);
                  buffer[2] = 'p';
                  d->name[0] = Safe_strdup(&buffer[2]);
                } // if
            }
          else if (0 == strcmp(key, "using"))
            {
              // using %<name>s
              res = sscanf(&line[1 + strlen(key)], " %16s", &buffer[0]);
              if ((1 < res) || (3 > strlen(&buffer[3])))
                {
                  SYNTAX("<name> (e.g., 18f452) expected.");
                }
              else
                {
                  template = find_in_list(&buffer[0], head);
                  if (!template)
                    {
                      SYNTAX("<name> (e.g., 18f452) expected.");
                    }
                  else
                    {
                      memcpy(&d->RAMsize, &template->RAMsize,
                             ((char *)&d->next) - ((char *)&d->RAMsize));
                    } // if
                } // if
            }
          else if (0 == strcmp(key, "ramsize"))
            {
              // ramsize %<bytes>i
              res = sscanf(&line[1 + strlen(key)], " %i", &val[0]);
              if (res < 1)
                {
                  SYNTAX("<bytes> (e.g., 256) expected.");
                }
              else
                {
                  d->RAMsize = val[0];
                } // if
            }
          else if (0 == strcmp(key, "split"))
            {
              // split %<offset>i
              res = sscanf(&line[1 + strlen(key)], " %i", &val[0]);
              if (res < 1)
                {
                  SYNTAX("<offset> (e.g., 0x80) expected.");
                }
              else
                {
                  d->acsSplitOfs = val[0];
                } // if
            }
          else if (0 == strcmp(key, "configrange"))
            {
              // configrange %<first>i %<last>i
              res = sscanf(&line[1 + strlen(key)], " %i %i",
                           &val[0], &val[1]);
              if (res < 2)
                {
                  SYNTAX("<first> <last> (e.g., 0xf60 0xfff) expected.");
                }
              else
                {
                  d->cwInfo.confAddrStart = val[0];
                  d->cwInfo.confAddrEnd = val[1];
                } // if
            }
          else if (0 == strcmp(key, "configword"))
            {
              // configword %<address>i %<mask>i %<value>i [%<and-mask>i]
              res = sscanf(&line[1 + strlen(key)], " %i %i %i %i",
                           &val[0], &val[1], &val[2], &val[3]);
              if (res < 3)
                {
                  SYNTAX("<address> <mask> <value> [<and-mask>] (e.g., 0x200001 0x0f 0x07) expected.");
                }
              else
                {
                  val[0] -= d->cwInfo.confAddrStart;
                  if ((val[0] < 0)
                      || (val[0] > (d->cwInfo.confAddrEnd - d->cwInfo.confAddrStart))
                      || (val[0] >= CONFIGURATION_WORDS))
                    {
                      SYNTAX("address out of bounds.");
                    }
                  else
                    {
                      d->cwInfo.crInfo[val[0]].mask = val[1];
                      d->cwInfo.crInfo[val[0]].value = val[2];
                      d->cwInfo.crInfo[val[0]].andmask = 0;
                      if (res >= 4)
                        {
                          // apply extra mask (e.g., to disable XINST)
                          d->cwInfo.crInfo[val[0]].andmask = val[3];
                        } // if
                    } // if
                } // if
            }
          else if (0 == strcmp(key, "idlocrange"))
            {
              // idlocrange %<first>i %<last>i
              res = sscanf(&line[1 + strlen(key)], " %i %i",
                           &val[0], &val[1]);
              if (res < 2)
                {
                  SYNTAX("<first> <last> (e.g., 0xf60 0xfff) expected.");
                }
              else
                {
                  d->idInfo.idAddrStart = val[0];
                  d->idInfo.idAddrEnd = val[1];
                } // if
            }
          else if (0 == strcmp(key, "idword"))
            {
              // idword %<address>i %<value>i
              res = sscanf(&line[1 + strlen(key)], " %i %i",
                           &val[0], &val[1]);
              if (res < 2)
                {
                  SYNTAX("<address> <value> (e.g., 0x3fffff 0x00) expected.");
                }
              else
                {
                  val[0] -= d->idInfo.idAddrStart;
                  if ((val[0] < 0)
                      || (val[0] > (d->idInfo.idAddrEnd - d->idInfo.idAddrStart))
                      || (val[0] >= IDLOCATION_BYTES))
                    {
                      SYNTAX("address out of bounds.");
                    }
                  else
                    {
                      d->idInfo.irInfo[val[0]].value = val[1];
                    } // if
                } // if
            }
          else if (0 == strcmp(key, "XINST"))
            {
              // XINST %<supported>i
              res = sscanf(&line[1 + strlen(key)], " %i", &val[0]);
              if (res < 1)
                {
                  SYNTAX("<supported> (e.g., 1) expected.");
                }
              else
                {
                  d->xinst = val[0];
                } // if
            }
          else
            {
              printf("%s: Invalid keyword in %s ignored: %s\n",
                     __func__, DEVICE_FILE_NAME, key);
            } // if
        } // while

      if (d)
        {
          if (tail)
            {
              tail->next = d;
            }
          else
            {
              head = d;
            } // if
          tail = d;
          d = NULL;
        } // if

      devices = head;

      fclose(f);
    } // if

  d = find_in_list(name, devices);
  if (!d)
    {
      d = &default_device;
    } // if

  return (d);
}

/*-----------------------------------------------------------------*
 *
 *-----------------------------------------------------------------*/
void pic16_init_pic(const char *pic_type)
{
    pic16 = pic16_find_device(pic_type);

    if (&default_device == pic16) {
        if (pic_type) {
            fprintf(stderr, "'%s' was not found.\n", pic_type);
        } else {
            fprintf(stderr, "No processor has been specified (use -pPROCESSOR_NAME)\n");
        } // if

        if (devices) {
            fprintf(stderr,"Valid devices are (use --verbose for more details):\n");
            pic16_list_devices(devices);
        } // if
        exit(EXIT_FAILURE);
    } // if
}

/*-----------------------------------------------------------------*
 *  char *pic16_processor_base_name(void) - Include file is derived from this.
 *-----------------------------------------------------------------*/

const char *pic16_processor_base_name(void)
{
  if(!pic16)
    return NULL;

  return pic16->name[0];
}

#define DEBUG_CHECK     0

/*
 * return 1 if register wasn't found and added, 0 otherwise
 */
int checkAddReg(set **set, reg_info *reg)
{
  reg_info *tmp;


        if(!reg)return 0;
#if DEBUG_CHECK
        fprintf(stderr, "%s: about to insert REGister: %s ... ", __FUNCTION__, reg->name);
#endif

        for(tmp = setFirstItem(*set); tmp; tmp = setNextItem(*set)) {
                if(!strcmp(tmp->name, reg->name))break;
        }

        if(!tmp) {
                addSet(set, reg);
#if DEBUG_CHECK
                fprintf(stderr, "added\n");
#endif
                return 1;
        }

#if DEBUG_CHECK
        fprintf(stderr, "already added\n");
#endif
  return 0;
}

int checkAddSym(set **set, symbol *sym)
{
  symbol *tmp;

        if(!sym)return 0;
#if DEBUG_CHECK
        fprintf(stderr, "%s: about to add SYMbol: %s ... ", __FUNCTION__, sym->name);
#endif

        for(tmp = setFirstItem( *set ); tmp; tmp = setNextItem(*set)) {
                if(!strcmp(tmp->name, sym->name))break;
        }

        if(!tmp) {
                addSet(set, sym);
#if DEBUG_CHECK
                fprintf(stderr, "added\n");
#endif
                return 1;
        }

#if DEBUG_CHECK
        fprintf(stderr, "already added\n");
#endif

  return 0;
}

int checkSym(set *set, symbol *sym)
{
  symbol *tmp;

        if(!sym)return 0;

#if DEUG_CHECK
        fprintf(stderr, "%s: about to search for SYMbol: %s ... ", __FUNCTION__, sym->name);
#endif

        for(tmp = setFirstItem( set ); tmp; tmp = setNextItem( set )) {
                if(!strcmp(tmp->name, sym->name))break;
        }

        if(!tmp) {
#if DEBUG_CHECK
                fprintf(stderr, "not found\n");
#endif
                return 0;
        }

#if DEBUG_CHECK
        fprintf(stderr, "found\n");
#endif

  return 1;
}

/*-----------------------------------------------------------------*
 * void pic16_groupRegistersInSection - add each register to its   *
 *      corresponding section                                      *
 *-----------------------------------------------------------------*/
void pic16_groupRegistersInSection(set *regset)
{
  reg_info *reg;
  sectSym *ssym;
  int docontinue=0;

        for(reg=setFirstItem(regset); reg; reg = setNextItem(regset)) {

#if 0
                fprintf(stderr, "%s:%d group registers in section, reg: %s (used: %d, %p)\n",
                        __FILE__, __LINE__, reg->name, reg->wasUsed, reg);
#endif
                if((reg->wasUsed
                        && !(reg->regop && SPEC_EXTR(OP_SYM_ETYPE(reg->regop))))
                  ) {

                        /* avoid grouping registers that have an initial value,
                         * they will be added later in idataSymSet */
                        if(reg->regop && (OP_SYMBOL(reg->regop)->ival && !OP_SYMBOL(reg->regop)->level))
                                continue;

#if 0
                        fprintf(stderr, "%s:%d register %s alias:%d fix:%d ival=%i level=%i code=%i\n",
                                __FILE__, __LINE__, reg->name, reg->alias, reg->isFixed,
                                        (reg->regop?(OP_SYMBOL(reg->regop)->ival?1:0):-1),
                                        (reg->regop?(OP_SYMBOL(reg->regop)->level):-1),
                                        (reg->regop?(IS_CODE(OP_SYM_ETYPE(reg->regop))):-1) );
#endif

                        docontinue=0;
                        for(ssym=setFirstItem(sectSyms);ssym;ssym=setNextItem(sectSyms)) {
                                if(!strcmp(ssym->name, reg->name)) {
//                                      fprintf(stderr, "%s:%d section found %s (%p) with var %s\n",
//                                                      __FILE__, __LINE__, ssym->section->name, ssym->section, ssym->name);
                                        if(strcmp(ssym->section->name, "access")) {
                                                addSet(&ssym->section->regsSet, reg);
                                                docontinue=1;
                                                break;
                                        } else {
                                                docontinue=0;
                                                reg->accessBank = 1;
                                                break;
                                        }
                                }
                        }

                        if(docontinue)continue;

//                      fprintf(stderr, "%s:%d reg: %s\n", __FILE__, __LINE__, reg->name);

                        if(reg->alias == 0x80) {
                                checkAddReg(&pic16_equ_data, reg);
                        } else
                        if(reg->isFixed) {
                                checkAddReg(&pic16_fix_udata, reg);
                        } else
                        if(!reg->isFixed) {
                                if(reg->pc_type == PO_GPR_TEMP)
                                        checkAddReg(&pic16_int_regs, reg);
                                else {
                                        if(reg->accessBank) {
                                                if(reg->alias != 0x40)
                                                        checkAddReg(&pic16_acs_udata, reg);
                                        } else
                                                checkAddReg(&pic16_rel_udata, reg);
                                }
                        }
                }
        }
}


/*-----------------------------------------------------------------*
 *  void pic16_assignConfigWordValue(int address, int value)
 *
 * All high performance RISC CPU PICs have seven config word starting
 * at address 0x300000.
 * This routine will assign a value to that address.
 *
 *-----------------------------------------------------------------*/
void
pic16_assignConfigWordValue(int address, unsigned int value)
{
  int i;

  for (i = 0; i < pic16->cwInfo.confAddrEnd - pic16->cwInfo.confAddrStart + 1; i++)
    {
      if ((address == pic16->cwInfo.confAddrStart + i)
          && (pic16->cwInfo.crInfo[i].mask != -1)
          && (pic16->cwInfo.crInfo[i].mask != 0))
        {

#if 0
          fprintf(stderr, "setting location 0x%x to value 0x%x, mask: 0x%x, test: 0x%x\n",
                  pic16->cwInfo.confAddrStart + i,
                  (~value) & 0xff,
                  pic16->cwInfo.crInfo[i].mask,
                  (pic16->cwInfo.crInfo[i].mask) & (~value));
#endif

#if 0
          if ((((pic16->cwInfo.crInfo[i].mask) & (~value)) & 0xff) != ((~value) & 0xff))
            {
              fprintf(stderr, "%s:%d a wrong value has been given for configuration register 0x%x\n",
                      __FILE__, __LINE__, address);
              return;
            } // if
#endif

          pic16->cwInfo.crInfo[i].value = (value & 0xff);
          if (pic16->cwInfo.crInfo[i].andmask
              && ((value & 0xff) != (value & 0xff & pic16->cwInfo.crInfo[i].andmask)))
            {
              // apply andmask if effective
              printf ("INFO: changing configuration word at 0x%x from 0x%x to 0x%x due to %s\n",
                      address,
                      (value & 0xff),
                      (value & 0xff & pic16->cwInfo.crInfo[i].andmask),
                      DEVICE_FILE_NAME);
              pic16->cwInfo.crInfo[i].value &= pic16->cwInfo.crInfo[i].andmask;
            } // if
          pic16->cwInfo.crInfo[i].emit = 1;
          return;
        } // if
    } // for
}

void
pic16_assignIdByteValue(int address, char value)
{
  int i;

  for (i = 0; i < pic16->idInfo.idAddrEnd - pic16->idInfo.idAddrStart + 1; i++)
    {
      if (address == pic16->idInfo.idAddrStart + i)
        {
          pic16->idInfo.irInfo[i].value = value;
          pic16->idInfo.irInfo[i].emit = 1;
        } // if
    } // for
}
