/* xa_main.c - Paul's XA51 Assembler

   Copyright 1997,2002 Paul Stoffregen (paul at pjrc dot com)

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* adapted from the osu8asm project, 1995 */
/* http://www.pjrc.com/tech/osu8/index.html */

/* 
   made "relocatable" by johan.knol@iduna.nl for sdcc
   
   This isn't a standalone assembler anymore. It's only purpose is to
   create relocatable modules (that has to be processed with xa_link) 
   out of sdcc-generated .xa files
*/

#define D(x) x

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define printf(x...) fprintf(stderr,x)

#include "xa_main.h"
#include "xa_version.h"
extern void yyrestart(FILE *new_file);
extern int yyparse();


char modulename[PATH_MAX];
char infilename[PATH_MAX];
char outfilename[PATH_MAX];
char listfilename[PATH_MAX];
char symfilename[PATH_MAX];

/* global variables */

FILE *frel, *fmem, *list_fp, *sym_fp;
extern FILE *yyin;
extern char *yytext;
extern char last_line_text[];
struct symbol *sym_list=NULL;
struct target *targ_list=NULL;
int lineno=1;
int p1=0, p2=0, p3=0;
int expr_result, expr_ok, jump_dest, inst;
int opcode;
char symbol_name[1000];
struct area_struct area[NUM_AREAS];
int current_area=0;

char rel_line[2][132];

char *areaToString (int area) {
  switch (area) 
    {
    case AREA_CSEG: return "CSEG";
    case AREA_DSEG: return "DSEG";
      //case AREA_OSEG: return "OSEG";
      //case AREA_ISEG: return "ISEG";
    case AREA_BSEG: return "BSEG";
    case AREA_XSEG: return "XSEG";
    case AREA_XISEG: return "XISEG";
    case AREA_XINIT: return "XINIT";
    case AREA_GSINIT: return "GSINIT";
      //case AREA_GSFINAL: return "GSFINAL";
      //case AREA_HOME: return "HOME";
      //case AREA_SSEG: return "SSEG";
    }
  return ("UNKNOW");
}

/* "mem" is replaced by area[current_area].alloc_position */
/* int mem=0; */   /* mem is location in memory */

/* add symbols to list when we find their definition in pass #1 */
/* we will evaluate their values in pass #2, and figure out if */
/* they are branch targets betweem passes 1 and 2.  Every symbol */
/* should appear exactly once in this list, since it can't be redefined */

struct symbol * build_sym_list(char *thename)
{
	struct symbol *new, *p;

	if ((p=findSymbol(thename))) {
	  if (p->isdef) {
	    fprintf (stderr, "error: symbol %s already defined\n", thename);
	    exit (1);
	  } else {
	    return p;
	  }
	}

	//printf("  Symbol: %s  Line: %d\n", thename, lineno);
	new = (struct symbol *) malloc(sizeof(struct symbol));
	new->name = (char *) malloc(strlen(thename)+1);
	strcpy(new->name, thename);
	new->value = 0;
	new->istarget = 0;
	new->isdef = 0;
	new->isbit = 0;
	new->isreg = 0;
	new->line_def = lineno - 1;
	new->area = current_area;
	new->mode = 'X'; // start with an external
	new->next = NULL;
	if (sym_list == NULL) return (sym_list = new);
	p = sym_list;
	while (p->next != NULL) p = p->next;
	p->next = new;
	return (new);
}

struct symbol *findSymbol (char *thename) {
  struct symbol *p;
  for (p=sym_list; p; p=p->next) {
    if (strcasecmp(thename, p->name)==0) {
      return p;
    }
  }
  return NULL;
}

int assign_value(char *thename, int thevalue, char mode) {
  struct symbol *p;
  
  p = sym_list;
  while (p != NULL) {
    if (!(strcasecmp(thename, p->name))) {
      p->area=current_area;
      p->value = thevalue;
      p->isdef = 1;
      p->mode = mode;
      return (0);
    }
    p = p->next;
  }
  fprintf(stderr, "Internal Error!  Couldn't find symbol\n");
  exit(1);
}

int mk_bit(char *thename, int area)
{
        struct symbol *p;

        p = sym_list;
        while (p != NULL) {
                if (!(strcasecmp(thename, p->name))) {
                        p->isbit = 1;
			p->area = area;
                        return (0);
                }
                p = p->next;
        }
        fprintf(stderr, "Internal Error!  Couldn't find symbol\n");
        exit(1);
}

int mk_sfr(char *thename)
{
        struct symbol *p;

        p = sym_list;
        while (p != NULL) {
                if (!(strcasecmp(thename, p->name))) {
                        p->issfr = 1;
			p->area = 0;
                        return (0);
                }
                p = p->next;
        }
        fprintf(stderr, "Internal Error!  Couldn't find symbol\n");
        exit(1);
}


int mk_reg(char *thename)
{
        struct symbol *p;

        p = sym_list;
        while (p != NULL) {
                if (!(strcasecmp(thename, p->name))) {
                        p->isreg = 1;
                        return (0);
                }
                p = p->next;
        }
        fprintf(stderr, "Internal Error!  Couldn't find symbol\n");
        exit(1);
}

int mk_global(char *thename)
{
  struct symbol *p;
  
  p = sym_list;
  while (p != NULL) {
    if (!(strcasecmp(thename, p->name))) {
      p->global = 1;
      return (0);
    }
    p = p->next;
  }
  fprintf(stderr, "Internal Error!  Couldn't find symbol\n");
  exit(1);
}

int get_value(char *thename)
{
  struct symbol *p;
  p = sym_list;
  while (p != NULL) {
    if (!(strcasecmp(thename, p->name))) {
      if (p->mode=='=')
	;//return 0;
      return (p->value);
    }
    p = p->next;
  }
  fprintf(stderr, "Internal Error!  Couldn't find symbol value\n");
  exit(1);
}
		


/* add every branch target to this list as we find them */
/* ok if multiple entries of same symbol name in this list */

struct target * build_target_list(char *thename)
{
	struct target *new, *p;
	new = (struct target *) malloc(sizeof(struct target));
	new->name = (char *) malloc(strlen(thename)+1);
	strcpy(new->name, thename);
	new->next = NULL;
	if (targ_list == NULL) return (targ_list = new);
	p = targ_list;
	while (p->next != NULL) p = p->next;
	p->next = new;
	return (new);
}

/* figure out which symbols are branch targets */

void flag_targets()
{
	struct symbol *p_sym;
	struct target *p_targ;
	p_targ = targ_list;
	while (p_targ != NULL) {
		p_sym = sym_list;
		while (p_sym != NULL) {
			if (!strcasecmp(p_sym->name, p_targ->name))
				p_sym->istarget = 1;
			p_sym = p_sym->next;
		}
		p_targ = p_targ->next;
	}
}

void print_symbol_table()
{
  struct symbol *p;
  p = sym_list;
  while (p != NULL) {
#if 0
    fprintf(sym_fp, "Sym in %-5s: %s\n", areaToString(p->area), p->name);
    fprintf(sym_fp, "  at: 0x%04X (%5d)", p->value, p->value);
    fprintf(sym_fp, " Def:%s", p->isdef ? "Yes" : "No ");
    fprintf(sym_fp, " Bit:%s", p->isbit ? "Yes" : "No ");
    fprintf(sym_fp, " Target:%s", p->istarget ? "Yes" : "No ");
    fprintf(sym_fp, " Line %d\n", p->line_def);
#else
    if (p->issfr) {
      fprintf (sym_fp, "%-7s", "SFR");
    } else if (p->isbit && !p->area) {
      fprintf (sym_fp, "%-7s", "SBIT");
    } else if (p->mode=='=') {
      fprintf (sym_fp,"ABS    ");
    } else if (!p->isdef) {
      fprintf (sym_fp,"EXTRN  ");
    } else {
      fprintf (sym_fp, "%-7s", areaToString(p->area));
    }
    fprintf (sym_fp, " 0x%04x (%5d)", p->value, p->value);
    fprintf (sym_fp, " %s", p->isdef ? "D" : "-");
    fprintf (sym_fp, "%s", p->isbit ? "B" : "-");
    fprintf (sym_fp, "%s", p->istarget ? "T" : "-");
    fprintf (sym_fp, " %s\n", p->name);
#endif
    p = p->next;
  }
}

/* check that every symbol is in the table only once */

void check_redefine()
{
  struct symbol *p1, *p2;
  p1 = sym_list;
  while (p1 != NULL) {
    p2 = p1->next;
    while (p2 != NULL) {
      if (!strcasecmp(p1->name, p2->name)) {
	fprintf(stderr, "Error: symbol '%s' redefined on line %d", 
		p1->name, p2->line_def);
	fprintf(stderr, ", first defined on line %d\n", p1->line_def);
	exit(1);
      }
      p2 = p2->next;
    }
    p1 = p1->next;
  }
}

int is_target(char *thename)
{
	struct symbol *p;
	p = sym_list;
	while (p != NULL) {
		if (!strcasecmp(thename, p->name)) return (p->istarget);
		p = p->next;
	}
	return (0);
}

int is_bit(char *thename)
{
        struct symbol *p;
        p = sym_list;
        while (p != NULL) {
                if (!strcasecmp(thename, p->name)) return (p->isbit);
                p = p->next;
        }
        return (0);
}

int is_reg(char *thename)
{
        struct symbol *p;
        p = sym_list;
        while (p != NULL) {
                if (!strcasecmp(thename, p->name)) return (p->isreg);
                p = p->next;
        }
        return (0);
}


struct symbol *is_def(char *thename)
{
  struct symbol *p;
  p = sym_list;
  while (p != NULL) {
    if (!strcasecmp(thename, p->name) && p->isdef) 
      return p;
    p = p->next;
  }
  return NULL;
}

struct symbol *is_ref(char *thename) {
  struct symbol *p;
  p = sym_list;
  while (p != NULL) {
    if (strcasecmp(thename, p->name)==0) 
      return p;
    p = p->next;
  }
  return NULL;
}

int is_abs(char *thename) {
  struct symbol *p;
  p = sym_list;
  while (p != NULL) {
    if (strcasecmp(thename, p->name)==0) 
      return p->mode == '=';
    p = p->next;
  }
  return 0;
}

/* this routine is used to dump a group of bytes to the output */
/* it is responsible for generating the list file and sending */
/* the bytes one at a time to the object code generator */
/* this routine is also responsible for generatine the list file */
/* though is it expected that the lexer has placed all the actual */
/* original text from the line in "last_line_text" */

static short last_area=-1;

int debug=0;

void out(int *byte_list, int num) {
  struct symbol *p;
  int i, first=1;
  
  if (num > 0) fprintf(list_fp, "%06X: ", MEM_POS);
  else fprintf(list_fp, "\t");
  
  if (last_area!=current_area) {
    // emit area information
    if (area[current_area].size) {
      fprintf (frel, "A %s size %d flags 0\n", 
	       areaToString(current_area),
	       area[current_area].size);
      if  (!area[current_area].defsEmitted) {
	for (p=sym_list; p; p=p->next) {
	  if (p->global && p->isdef && p->area==current_area) {
	    // skip temp labels
	    if (p->name[strlen(p->name)-1]!='$') {
	      if (p->mode=='=') {
		fprintf (frel, "S %s Abs%04x\n", p->name, p->value);
	      } else {
		fprintf (frel, "S %s Def%04x\n", p->name, p->value);
	      }
	    }
	  }
	}
	area[current_area].defsEmitted=1;
      }
    }
    last_area=current_area;
  }
  if (current_area==AREA_CSEG ||
      current_area==AREA_GSINIT ||
      current_area==AREA_XINIT) {
    if (num) {
      for (i=0; i<num; i++) {
	if ((i%16)==0) {
	  fprintf (frel, "%sT %04x", i ? "\n" : "", MEM_POS+i);
	}
	fprintf (frel, " %02x", byte_list[i]);
      }
      fprintf (frel, "\n");
      if (rel_line[0][0]) {
	fprintf (frel, "%s\n", rel_line[0]);
	if (rel_line[1][0]) {
	  fprintf (frel, "%s\n", rel_line[1]);
	}
      }
    }
    for (i=0; i<num; i++) {
      if (!first && (i % 4) == 0) fprintf(list_fp, "\t");
      fprintf(list_fp, "%02X", byte_list[i]);
      if ((i+1) % 4 == 0) {
	if (first) fprintf(list_fp, "\t%s\n", last_line_text);
	else fprintf(list_fp, "\n");
	first = 0;
      } else {
	if (i<num-1) fprintf(list_fp, " ");
      }
    }
  }
  if (first) {
    if (num < 3) fprintf(list_fp, "\t");
    fprintf(list_fp, "\t%s\n", last_line_text);
  } else {
    if (num % 4) fprintf(list_fp, "\n");
  }
  operand[0][0]='\0';
  operand[1][0]='\0';
  rel_line[0][0]='\0';
  rel_line[1][0]='\0';
}


/* add NOPs to align memory location on a valid branch target address */

void pad_with_nop()
{
  static int nops[] = {NOP_OPCODE, NOP_OPCODE, NOP_OPCODE, NOP_OPCODE};
  int num;
  
  last_line_text[0] = '\0';
  
  for(num=0; (MEM_POS + num) % BRANCH_SPACING; num++) {
    sprintf (last_line_text, "\tnop\t; word allignment");
  }
  if (p3) out(nops, num);
  MEM_POS += num;
}

/* print branch out of bounds error */

void boob_error()
{
	fprintf(stderr, "Error: branch out of bounds");
	fprintf(stderr, " in line %d\n", lineno);
	exit(1);
}

/* turn a string like "10010110b" into an int */

int binary2int(char *str)
{
	register int i, j=1, sum=0;
	
	for (i=strlen(str)-2; i >= 0; i--) {
		sum += j * (str[i] == '1');
		j *= 2;
	}
	return (sum);
}

void print_usage(int);

void init_areas(void)
{
  area[AREA_CSEG].start=area[AREA_CSEG].alloc_position = 0;
  area[AREA_DSEG].start=area[AREA_DSEG].alloc_position = 0;
  area[AREA_BSEG].start=area[AREA_BSEG].alloc_position = 0;
  area[AREA_XSEG].start=area[AREA_XSEG].alloc_position = 0;
  area[AREA_XISEG].start=area[AREA_XISEG].alloc_position = 0;
  area[AREA_XINIT].start=area[AREA_XINIT].alloc_position = 0;
  area[AREA_GSINIT].start=area[AREA_GSINIT].alloc_position = 0;
  area[AREA_GSFINAL].start=area[AREA_GSFINAL].alloc_position = 0;
  area[AREA_HOME].start=area[AREA_HOME].alloc_position = 0;
}

void relPrelude() {
  //char buffer[132];
  int i, areas=0, globals=0;
  struct symbol *p;

  fprintf (frel, "SDCCXA rel, version %1.1f\n", version);
  for (i=1; i<NUM_AREAS; i++) {
    if ((area[i].size=area[i].alloc_position-area[i].start)) {
      areas++;
    }
  }
  for (p=sym_list; p; p=p->next) {
    if (p->isdef) {
      // skip temp labels
      if (p->name[strlen(p->name)-1]!='$') {
	globals++;
      }
    }
  }
  fprintf (frel, "H %d areas %d global symbols\n", areas, globals);
  fprintf (frel, "M %s\n", modulename);
  for (p=sym_list; p; p=p->next) {
    if (!p->isdef) {
      fprintf (frel, "S %s Ref0000\n", p->name);
    }
  }
}

void printVersion() {
  printf("\nPaul's XA51 Assembler\n");
  printf("Copyright 1997,2002 Paul Stoffregen\n\n");
  printf("This program is free software; you can redistribute it\n");
  printf("and/or modify it under the terms of the GNU General Public\n");
  printf("License, Version 2, published by the Free Software Foundation\n\n");
  printf("This program is distributed in the hope that it will be useful,\n");
  printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
}

int verbose=0, createSymbolFile=0;

void process_args(int argc, char **argv) 
{
  int i=0;

  if (argc < 2) print_usage(1);
  
  while (++i<argc && *argv[i]=='-') {
    if (strcmp(argv[i], "--version")==0) {
      printVersion();
      exit (0);
    }
    if (strcmp(argv[i], "--help")==0) {
      print_usage(0);
    }
    if (strcmp(argv[i], "-v")==0) {
      verbose++;
      continue;
    }
    if (strcmp(argv[i], "-s")==0) {
      createSymbolFile++;
      continue;
    }
    print_usage(1);
  }

  if (i!=argc-1) {
    // only 1 source file for now
    print_usage(1);
  }

  strcpy(infilename, argv[i]);

  if (strncasecmp(infilename+strlen(infilename)-4, ".asm", 3)) {
    fprintf (stderr, "unrecognized input file: \"%s\"\n", argv[i]);
    print_usage(1);
  }

  strcpy(modulename, infilename);
  modulename[strlen(modulename)-4] = '\0';
  sprintf (outfilename, "%s.rel", modulename);
  sprintf (listfilename, "%s.lst", modulename);
  if (createSymbolFile) {
    sprintf (symfilename, "%s.sym", modulename);
  }
}

/* pass #1 (p1=1) find all symbol defs and branch target names */
/* pass #2 (p2=1) align branch targets, evaluate all symbols */
/* pass #3 (p3=1) produce object code */

int main(int argc, char **argv)
{
	process_args (argc, argv);

	yyin = fopen(infilename, "r");
	if (yyin == NULL) {
		fprintf(stderr, "Can't open file '%s'.\n", infilename);
		exit(1);
	}
	frel = fopen(outfilename, "w");
	if (frel == NULL) {
		fprintf(stderr, "Can't write file '%s'.\n", outfilename);
		exit(1);
	}
	list_fp = fopen(listfilename, "w");
	if (list_fp == NULL) {
		fprintf(stderr, "Can't write file '%s'.\n", listfilename);
		exit(1);
	}
	if (createSymbolFile) {
	  sym_fp = fopen(symfilename, "w");
	  if (sym_fp == NULL) {
	    fprintf(stderr, "Can't write file '%s'.\n", symfilename);
	    exit(1);
	  }
	}

	if (verbose) printf("Pass 1: Building Symbol Table:\n");
	p1 = 1;
	init_areas();
	yyparse();
	flag_targets();
	check_redefine();

	if (verbose) printf("Pass 2: Aligning Branch Targets:\n");
	p1 = 0;
	p2 = 1;
	rewind(yyin);
	yyrestart(yyin);
	lineno = 1;
	init_areas();
	yyparse();

	relPrelude();
	if (createSymbolFile) print_symbol_table();

	if (verbose) printf("Pass 3: Generating Object Code:\n");
	p2 = 0;
	p3 = 1;
	rewind(yyin);
	yyrestart(yyin);
	lineno = 1;
	init_areas();
	yyparse();

	fclose(yyin);
	return 0;
}


void print_usage(int fatal)
{
  FILE *out = fatal ? stderr : stdout;

  fprintf (out, "Usage: xa_asm [-s] [-v] file.xa\n");
  fprintf (out, "  -v            verbose: show progress\n");
  fprintf (out, "  -s            create symbol file\n");
  fprintf (out, "  --version     show version/copyright info and exit\n");
  fprintf (out, "  --help        show this and exit\n");
#if 0
  // some usefull options I can think of.
  fprintf (out, "  -m            create map file\n");
  fprintf (out, "  -ss           create symbol file sorted by symbol\n");
  fprintf (out, "  -sa           create symbol file sorted by segment/address\n");
  fprintf (out, "  --no-temps    supress temp symbols in map and sym file\n");
  fprintf (out, "  --code-loc=#  sets the start address of the code\n");
  fprintf (out, "  --xdata-loc=# sets the start address of the external data\n");
  fprintf (out, "  --stack-loc=# sets the start address of the stack\n");
#endif
  exit(fatal);
}

