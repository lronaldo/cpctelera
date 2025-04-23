/* xa_main.h - Paul's XA51 Assembler

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

#define SIZE8 0
#define SIZE16 1
#define SIZE32 2
#define UNKNOWN -1
#define WORD_REG 16384
#define BYTE_REG 32768

/* max # of bytes in db directive */
#define MAX_DB          2500

/* max # char in symbol name */
#define MAX_SYMBOL      1024

/* max # of bytes per line */
#define MAX_LINE	4096

/* REL() computes branch operand from dest and memory */
/* location of the jump instruction itself */
/* this is later adjusted by one for jcu, of course */
#define BRANCH_SPACING 2
#define REL(dest, mem) (((dest)-((((mem)+1)/(\
BRANCH_SPACING))*(BRANCH_SPACING)))/(BRANCH_SPACING))

#define NOP_OPCODE 0	/* opcode for NOP */

/* a linked list of all the symbols */

struct symbol {
        char *name;
        int value;
        int istarget;   /* 1 if a branch target, 0 otherwise */
        int isdef;      /* 1 if defined, 0 if no value yet */
        int line_def;   /* line in which is was defined */
        int isbit;      /* 1 if a bit address, 0 otherwise */
        int issfr;
	int isreg;	/* 1 if a register, 0 otehrwise */
        int global ;    /* is defined as global */
        char mode;      /* Absolute, Relative, Tmplabel, eXternal */
        short lk_index; /* symbol index for the linker */
        int area;       /* the area that this symbol is in */
        struct symbol *next; };

/* a list of all the symbols that are branch targets */
/* (and will need to get aligned on 4 byte boundries) */

struct target {
        char *name;
        struct target *next; };

struct area_struct {
        int start;
	int alloc_position;
        int defsEmitted;
        int size;
};

extern int current_area;

#define MEM_POS (area[current_area].alloc_position)

enum {
  AREA_CSEG=1,
  AREA_DSEG,
  // AREA_OSEG,
  // AREA_ISEG,
  AREA_BSEG,
  AREA_XSEG,
  AREA_XISEG,
  AREA_XINIT,
  AREA_GSINIT,
  AREA_GSFINAL,
  AREA_HOME,
  AREA_SSEG,
  NUM_AREAS=AREA_SSEG
};

extern struct area_struct area[NUM_AREAS];

extern FILE *yyin;
extern char *yytext;
extern int lineno;
extern int p1, p2, p3, mem, m_len;

extern struct symbol * build_sym_list(char *thename);
extern int assign_value(char *thename, int thevalue, char mode);
extern int mk_bit(char *thename, int current_area);
extern int mk_reg(char *thename);
extern void out(int *byte_list, int num);
extern int is_target(char *thename);
extern void pad_with_nop();
extern int binary2int(char *str);
extern int is_bit(char *thename);
extern int is_reg(char *thename);
extern struct symbol * is_def(char *thename);
extern struct symbol * is_ref(char *thename);
extern int get_value(char *thename);
extern struct symbol *findSymbol (char *thename);
extern char rel_line[2][132];
extern char operand[2][MAX_SYMBOL];
extern void error(char*);
int mk_bit(char*, int);
int mk_sfr(char*);
int mk_global(char*);
struct target * build_target_list(char *thename);
struct symbol * build_sym_list(char *);
int find_size_reg(int op1spec);
int find_size0(int isize);
int find_size1(int isize, int op1spec);
int find_size2(int isize, int op1spec, int op2spec);
int yyerror(char *s);
int imm_data4_signed(int value);
int imm_data4_unsigned(int value);
int imm_data5_unsigned(int value);
int imm_data8(int value);
int imm_data16(int value);
int reg(int reg_spec);
int reg_indirect(int reg_spec);
int lsb(int value);
int msb(int value);
int direct_addr(int value);
int bit_addr(int value);
int rel16(int pos, int dest);
int rel8(int pos, int dest);
char *areaToString (int area);

FILE *frel, *fmem, *list_fp, *sym_fp;

extern void relout();
