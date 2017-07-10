%{
/* xa_rasm.y - This file is part of Paul's XA51 Assembler

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

/* Author contact: paul@pjrc.com */

/* parser for the 51-XA assembler, Paul Stoffregen, July 1997 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xa_main.h"
  
int op[MAX_DB];
int size;
int inst_size;
int arith_opcode, short_opcode, num_op, opcode0, opcode1;
int shift_imm_opcode, shift_reg_opcode, rotate_opcode;
int stack_addr_opcode, stack_reg_opcode, branch_opcode;
int rlist_reg_bank, rlist_bitmask, rlist_size;
int db_count, dw_count, i;
char symbol_name[MAX_SYMBOL], base_symbol_name[MAX_SYMBOL]={'\0'};
char operand[2][MAX_SYMBOL]={{'\0'},{'\0'}};

extern char lex_sym_name[];
extern int yylex();

extern void yyrestart(FILE *new_file);
extern char * disasm(int byte, int memory_location);
void error(char *s);


void RELOC_FF(unsigned where, unsigned pc, short rl) {
  // pc = PC of the next instruction
  struct symbol *sym;
  if ((sym=findSymbol(operand[0]))) {
    if (sym->mode=='X' || sym->area!=current_area) {
      sprintf (rel_line[rl], "R %04x REL_FF %s %04x", 
	       where, sym->name, pc);
    }
  }
}
 
void RELOC_FFFF(unsigned where, unsigned pc, short rl) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[0]))) {
    if (sym->mode=='X' || sym->area!=current_area) {
      sprintf (rel_line[rl], "R %04x REL_FFFF %s %04x", 
	       where, sym->name, pc);
    }
  }
}
 
void RELOC_ABS_0F(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq])) && sym->mode!='A') {
    sprintf (rel_line[seq], "R %04x ABS_0F %s 0", where, sym->name);
  }
}

void RELOC_BIT_03FF(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq])) && sym->mode!='A') {
    sprintf (rel_line[seq], "R %04x BIT_03FF %s 0", where, sym->name);
  }
}

void RELOC_DIR_07FF(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq])) && sym->mode!='A') {
    sprintf (rel_line[seq], "R %04x DIR_07FF %s 0", where, sym->name);
  }
}

void RELOC_DIR_70FF(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq])) && sym->mode!='A') {
    sprintf (rel_line[seq], "R %04x DIR_70FF %s 0", where, sym->name);
  }
}

void RELOC_ABS_FF(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq])) && sym->mode!='A') {
    sprintf (rel_line[seq], "R %04x DIR_FF %s 0", where, sym->name);
  }
}

void RELOC_ABS_FFFF(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq]))) {
    switch (sym->mode) {
    case 'A':
      // sfr or sbit, already in instruction
    case '=':
      // equat, already in instruction
      break;
    case 'X':
      // external reference
      sprintf (rel_line[seq], "R %04x ABS_FFFF %s %04x", where, sym->name,
	       sym->value);
      break;
    case 'R':
      // absolute in current segment
      sprintf (rel_line[seq], "R %04x ABS_PC PC %04x", where, sym->value);
      break;
    default:
      fprintf (stderr, "unknown ABS_FFFF\n");
      exit (1);
    }
  }
}
 
void RELOC_DIR_0700FF(unsigned where, int seq) {
  struct symbol *sym;
  if ((sym=findSymbol(operand[seq])) && sym->mode!='A') {
    sprintf (rel_line[seq], "R %04x ABS_0700FF %s 0", where, sym->name);
  }
}
 
%}

%token ADD ADDC ADDS AND ANL ASL ASR BCC BCS BEQ BG BGE BGT
%token BKPT BL BLE BLT BMI BNE BNV BOV BPL BR CALL CJNE CLR
%token CMP CPL DA DIV DIVU DJNZ FCALL FJMP JB JBC JMP JNB JNZ
%token JZ LEA LSR MOV MOVC MOVS MOVX MUL MULU NEG NOP NORM
%token OR ORL POP POPU PUSH PUSHU RESET RET RETI RL RLC RR RRC
%token SETB SEXT SUB SUBB TRAP XCH XOR
%token REG DPTR PC A C USP
%token WORD BIT NUMBER CHAR STRING EOL LOCAL_LABEL
%token ORG EQU SFR DB DW BITDEF REGDEF LOW HIGH
%token RSHIFT LSHIFT
%token AREA AREA_NAME AREA_DESC DS
%token MODULE GLOBL 

%left '&' '|' '^'
%left RSHIFT LSHIFT
%left '+' '-'
%left '*' '/'
%nonassoc UNARY

%%
all:           line
             | line all;

line:          linesymbol ':' linenosym {
			if (p1) {
			        build_sym_list(symbol_name);
				if (current_area == AREA_BSEG) {
					mk_bit(symbol_name, current_area);
				}
			}
			if (p1 || p2) assign_value(symbol_name, MEM_POS, 'R');
			MEM_POS += $3;
		}
             | linenosym {
                        if (!is_abs(symbol_name)) {
			  MEM_POS += $1;
			}
		}

linenosym:     directive EOL {
	                if (p3) out(op, $1);
			$$ = $1;
		}
             | instruction EOL {
			if (p3) out(op, $1);
			$$ = $1;
		}
	     | EOL {
			if (p3) out(NULL, 0);
			$$ = 0;
		}
	     | error EOL	/* try to recover from any parse error */


directive:     '.' ORG expr {
			MEM_POS = $3;
			$$ = 0;
		}
	     | ORG expr {
			MEM_POS = $2;
			$$ = 0;
		}
             | '.' EQU symbol ',' expr { 
			if (p1) build_sym_list(symbol_name);
			if (p1 || p2) assign_value(symbol_name, $5, '?');
			$$ = 0;
		}
             | symbol '=' expr {
	                if (p1) build_sym_list(symbol_name);
			if (p1 || p2) assign_value(symbol_name, $3, '=');
	        }
	     | symbol SFR expr {
	                if (p1) build_sym_list(symbol_name);
			if (p1 || p2) assign_value(symbol_name, $3, 'A');
			if (p1 || p2) mk_sfr(symbol_name);
			$$ = 0;
		}
	     | '.' BITDEF bitsymbol ',' bit {
			if (p1) {
				build_sym_list(symbol_name);
				mk_bit(symbol_name, 0);
			}
			if (p1 || p2) assign_value(symbol_name, $5, '?');
			$$ = 0;
		}
	     | bitsymbol BITDEF bit {
			if (p1) {
                                build_sym_list(symbol_name);
                                mk_bit(symbol_name, 0);
                        }
                        if (p1 || p2) assign_value(symbol_name, $3, '?');
                        $$ = 0;
                }
	     | bitsymbol BITDEF expr {
	                if (p1) {
                                build_sym_list(symbol_name);
                                mk_bit(symbol_name, 0);
                        }
                        if (p1 || p2) assign_value(symbol_name, $3, 'A');
                        $$ = 0;
                }
	     | '.' REGDEF regsymbol ',' REG {
			if (p1) {
				build_sym_list(symbol_name);
				mk_reg(symbol_name);
			}
			if (p1 || p2) assign_value(symbol_name, $5, '?');
			$$ = 0;
		}
	     | regsymbol REGDEF REG {
			if (p1) {
                                build_sym_list(symbol_name);
                                mk_reg(symbol_name);
                        }
                        if (p1 || p2) assign_value(symbol_name, $3, '?');
                        $$ = 0;
                }

             | '.' db_directive bytes {
			$$ = db_count;
		}
	     | '.' dw_directive words {
			$$ = dw_count;
		}
	     | '.' AREA AREA_NAME AREA_DESC {
			if ($3 < 0 || $3 > NUM_AREAS) {
				error("Illegal Area Directive");
			}
			symbol_name[0] = '\0';
			current_area = $3;
			$$ = 0;
		}
	     | '.' MODULE WORD {
			/* ignore module definition */
			$$ = 0;
		}
	     | '.' GLOBL WORD {
	                mk_global(lex_sym_name);
			/* ignore global symbol declaration */
			$$ = 0;
		}
	     | '.' GLOBL bit {
			/* ignore bit symbol declaration */
 			$$ = 0;
		}
	     | '.' DS expr {
			/* todo: if CSEG, emit some filler bytes */
			$$ = $3;
		}

db_directive:	DB {db_count = 0;}


linesymbol:    normal_or_bit_symbol  { 
			strcpy(symbol_name, lex_sym_name);
			if (!strchr(lex_sym_name, ':')) {
				/* non-local label, remember base name */
				strcpy(base_symbol_name, lex_sym_name);
			}
			if (is_target(symbol_name)) pad_with_nop();
		}

normal_or_bit_symbol: WORD {$$ = $1;}
		| BIT {$$ = $1;}

bytes:		  byte_element
		| bytes ',' byte_element

byte_element:	expr {
			op[db_count] = $1 & 255;
			if (++db_count >= MAX_DB) {
				error("too many bytes, use two DB");
				db_count--;
			}
		}
		| STRING {
			for(i=1; i < strlen(yytext)-1; i++) {
				op[db_count++] = yytext[i];
				if (db_count >= MAX_DB) {
					error("too many bytes, use two DB");
					db_count--;
				}
			}
		}

dw_directive:	DW {dw_count = 0;}

words:		  words ',' word_element
		| word_element

word_element:	expr {
			op[dw_count] = $1 & 255;
			op[dw_count+1] = ($1 >> 8) & 255;
			dw_count += 2;
			if (dw_count >= MAX_DB) {
				error("too many bytes, use two DW");
				db_count -= 2;
			}
		}



symbol:     WORD  {
		strcpy(symbol_name, lex_sym_name);
		}

bitsymbol:    WORD { strcpy(symbol_name, lex_sym_name); }
	    | BIT  { strcpy(symbol_name, lex_sym_name); }


regsymbol:    WORD { strcpy(symbol_name, lex_sym_name); }
	    | REG  { strcpy(symbol_name, lex_sym_name); }

bit:	expr '.' expr {
		if ($3 < 0 || $3 > 7) {
			/* only 8 bits in a byte */
			error("Only eight bits in a byte");
		}
		$$ = 100000;	/* should really check $1 is valid */
		if ($1 >= 0x20 && $1 <= 0x3F) {
			$$ = $1 * 8 + $3;
		}
		if ($1 >= 0x400 && $1 <= 0x43F) {
			$$ = ($1 - 0x400) * 8 + $3 + 0x200;
		}
	}
	| REG '.' expr {
		$$ = 100000;
		if (find_size_reg($1) == SIZE8) {
			if ($3 < 0 || $3 > 7)
				error("byte reg has only 8 bits");
			$$ = reg($1) * 8 + $3;
		}
		if (find_size_reg($1) == SIZE16) {
			if ($3 < 0 || $3 > 15)
				error("word reg has only 16 bits");
			$$ = reg($1) * 16 + $3;
		}
	}
	| BIT {$$ = $1;}

jmpaddr:	WORD {
			$$ = $1;
			if (p1) build_target_list(lex_sym_name);
		}
	      | NUMBER {
			if ($1 & 1) error("Jump target must be aligned");
			$$ = $1;
		}


expr:	value	{$$ = $1;}
	| expr '+' expr {$$ = $1 + $3;}
	| expr '-' expr {$$ = $1 - $3;}
	| expr '*' expr {$$ = $1 * $3;}
	| expr '/' expr {$$ = $1 / $3;}
	| expr '&' expr {$$ = $1 & $3;}
	| expr '|' expr {$$ = $1 | $3;}
	| expr '^' expr {$$ = $1 ^ $3;}
	| expr RSHIFT expr {$$ = $1 >> $3;}
	| expr LSHIFT expr {$$ = $1 << $3;}
	| '-' expr %prec UNARY {$$ = $2 * -1;}
	| '+' expr %prec UNARY {$$ = $2;}
	| '(' expr ')' {$$ = $2;}
	| LOW expr %prec UNARY {$$ = $2 & 255;}
	| HIGH expr %prec UNARY {$$ = ($2 >> 8) & 255;}


value:	  NUMBER {$$ = $1;}
	| CHAR {$$ = $1;}
        | WORD { $$ = $1;}

rlist:	REG {
		rlist_bitmask = 1<<(reg($1) % 8);
		rlist_reg_bank = (reg($1) / 8) ? 1 : 0;
		rlist_size = find_size_reg($1);
	}
	| REG ',' rlist {
		rlist_bitmask |= 1<<(reg($1) % 8);
		if (rlist_reg_bank != ((reg($1) / 8) ? 1 : 0))
			error("register list may not mix 0-7/8-15 regs");
		if (rlist_size != find_size_reg($1))
			error("register list may not mix 8/16 bit registers");
	}





instruction:

  arith_inst REG ',' REG {
	$$ = 2;
	size = find_size2(inst_size, $2, $4);
	op[0] = arith_opcode * 16 + size * 8 + 1;
	op[1] = reg($2) * 16 + reg($4);
  }
| arith_inst REG ',' '[' REG ']' {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = arith_opcode * 16 + size * 8 + 2;
	op[1] = reg($2) * 16 + reg_indirect($5);
  }
| arith_inst '[' REG ']' ',' REG {
	$$ = 2;
	size = find_size1(inst_size, $6);
	op[0] = arith_opcode * 16 + size * 8 + 2;
	op[1] = reg($6) * 16 + 8 + reg_indirect($3);
  }
| arith_inst REG ',' '[' REG '+' expr ']' {
	size = find_size1(inst_size, $2);
	if ($7 >= -128 && $7 <= 127) {
		$$ = 3;
		op[0] = arith_opcode * 16 + size * 8 + 4;
		op[1] = reg($2) * 16 + reg_indirect($5);
		op[2] = ($7 >= 0) ? $7 : 256 + $7;
		RELOC_ABS_FF(MEM_POS+2,0);
	} else {
		$$ = 4;
		op[0] = arith_opcode * 16 + size * 8 + 5;
		op[1] = reg($2) * 16 + reg_indirect($5);
		op[2] = ($7 >= 0) ? msb($7) : msb(65536 + $7);
		op[3] = ($7 >= 0) ? lsb($7) : lsb(65536 + $7);
		RELOC_ABS_FFFF(MEM_POS+2,0);
	}
  }
| arith_inst '[' REG '+' expr ']' ',' REG {
	size = find_size1(inst_size, $8);
	if ($5 >= -128 && $5 <= 127) {
		$$ = 3;
		op[0] = arith_opcode * 16 + size * 8 + 4;
		op[1] = reg($8) * 16 + 8 + reg_indirect($3);
		op[2] = ($5 >= 0) ? $5 : 256 + $5;
		RELOC_ABS_FF(MEM_POS+2,0);
	} else {
		$$ = 4;
		op[0] = arith_opcode * 16 + size * 8 + 5;
		op[1] = reg($8) * 16 + 8 + reg_indirect($3);
		op[2] = ($5 >= 0) ? msb($5) : msb(65536 + $5);
		op[3] = ($5 >= 0) ? lsb($5) : lsb(65536 + $5);
		RELOC_ABS_FFFF(MEM_POS+2,0);
	}
  }
| arith_inst REG ',' '[' REG '+' ']' {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = arith_opcode * 16 + size * 8 + 3;
	op[1] = reg($2) * 16 + reg_indirect($5);
  }
| arith_inst '[' REG '+' ']' ',' REG {
	$$ = 2;
	size = find_size1(inst_size, $7);
	op[0] = arith_opcode * 16 + size * 8 + 3;
	op[1] = reg($7) * 16 + 8 + reg_indirect($3);
  }
| arith_inst WORD ',' REG {
	$$ = 3;
	size = find_size1(inst_size, $4);
	op[0] = arith_opcode * 16 + size * 8 + 6;
	op[1] = reg($4) * 16 + 8 + msb(direct_addr($2));
	op[2] = lsb(direct_addr($2));
	RELOC_DIR_07FF(MEM_POS+1, 0);
  }
| arith_inst REG ',' WORD {
	$$ = 3;
	size = find_size1(inst_size, $2);
	op[0] = arith_opcode * 16 + size * 8 + 6;
	op[1] = reg($2) * 16 + msb(direct_addr($4));
	op[2] = lsb(direct_addr($4));
	RELOC_DIR_07FF(MEM_POS+1, 0);
  }
| arith_inst REG ',' '#' expr {
	size = find_size1(inst_size, $2);
	if (size == SIZE8) {
		$$ = 3;
		op[0] = 0x91;
		op[1] = reg($2) * 16 + arith_opcode;
		op[2] = imm_data8($5);
		RELOC_ABS_FF(MEM_POS+2, 0);
	} else {
		$$ = 4;
		op[0] = 0x99;
		op[1] = reg($2) * 16 + arith_opcode;
		op[2] = msb(imm_data16($5));
		op[3] = lsb(imm_data16($5));
		RELOC_ABS_FFFF (MEM_POS+2, 0);
	}
  }
| arith_inst '[' REG ']' ',' '#' expr {
	size = find_size0(inst_size);
	if (size == SIZE8) {
		$$ = 3;
		op[0] = 0x92;
		op[1] = reg_indirect($3) * 16 + arith_opcode;
		op[2] = imm_data8($7);
		RELOC_ABS_FF(MEM_POS+2, 0);
	} else {
		$$ = 4;
		op[0] = 0x9A;
		op[1] = reg_indirect($3) * 16 + arith_opcode;
		op[2] = msb(imm_data16($7));
		op[3] = lsb(imm_data16($7));
		RELOC_ABS_FFFF (MEM_POS+2, 0);
	}
  }
| arith_inst '[' REG '+' ']' ',' '#' expr {
	size = find_size0(inst_size);
	if (size == SIZE8) {
		$$ = 3;
		op[0] = 0x93;
		op[1] = reg_indirect($3) * 16 + arith_opcode;
		op[2] = imm_data8($8);
		RELOC_ABS_FF(MEM_POS+2, 0);
	} else {
		$$ = 4;
		op[0] = 0x9B;
		op[1] = reg_indirect($3) * 16 + arith_opcode;
		op[2] = msb(imm_data16($8));
		op[3] = lsb(imm_data16($8));
		RELOC_ABS_FFFF (MEM_POS+2, 0);
	}
  }
| arith_inst '[' REG '+' expr ']' ',' '#' expr {
	size = find_size0(inst_size);
	if ($5 >= -128 && $5 <= 127) {
		if (size == SIZE8) {
			$$ = 4;
			op[0] = 0x94;
			op[1] = reg_indirect($3) * 16 + arith_opcode;
			op[2] = ($5 >= 0) ? $5 : 256 + $5;
			op[3] = imm_data8($9);
			RELOC_ABS_FF(MEM_POS+2, 0);
			RELOC_ABS_FF(MEM_POS+3, 1);
		} else {
			$$ = 5;
			op[0] = 0x9C;
			op[1] = reg_indirect($3) * 16 + arith_opcode;
			op[2] = ($5 >= 0) ? $5 : 256 + $5;
			op[3] = msb(imm_data16($9));
			op[4] = lsb(imm_data16($9));
			RELOC_ABS_FF(MEM_POS+2, 0);
			RELOC_ABS_FFFF(MEM_POS+3, 1);
		}
	} else {
		if (size == SIZE8) {
			$$ = 5;
			op[0] = 0x95;
			op[1] = reg_indirect($3) * 16 + arith_opcode;
			op[2] = ($5 >= 0) ? msb($5) : msb(65536 + $5);
			op[3] = ($5 >= 0) ? lsb($5) : lsb(65536 + $5);
			op[4] = imm_data8($9);
			RELOC_ABS_FFFF(MEM_POS+2,0);
			RELOC_ABS_FF(MEM_POS+4,1);
		} else {
			$$ = 6;
			op[0] = 0x9D;
			op[1] = reg_indirect($3) * 16 + arith_opcode;
			op[2] = ($5 >= 0) ? msb($5) : msb(65536 + $5);
			op[3] = ($5 >= 0) ? lsb($5) : lsb(65536 + $5);
			op[4] = msb(imm_data16($9));
			op[5] = lsb(imm_data16($9));
			RELOC_ABS_FFFF(MEM_POS+2, 0);
			RELOC_ABS_FFFF(MEM_POS+4, 1);
		}
	}
  }
| arith_inst WORD ',' '#' expr {
        size = find_size0(inst_size);
	if (size == SIZE8) {
		$$ = 4;
		op[0] = 0x96;
		op[1] = msb(direct_addr($2)) * 16 + arith_opcode;
		op[2] = lsb(direct_addr($2));
		op[3] = imm_data8($5);
		RELOC_DIR_70FF(MEM_POS+1,0);
		RELOC_ABS_FF(MEM_POS+3,1);
	} else {
		$$ = 5;
		op[0] = 0x9E;
		op[1] = msb(direct_addr($2)) * 16 + arith_opcode;
		op[2] = lsb(direct_addr($2));
		op[3] = msb(imm_data16($5));
		op[4] = lsb(imm_data16($5));
		RELOC_DIR_70FF(MEM_POS+1,0);
		RELOC_ABS_FFFF (MEM_POS+3,1);
	}
  }

/* the next 8 instructions are MOV, but because MOV was used in the */
/* arith_inst group, it will cause a shift/reduce conflict if used */
/* directly below... so we're forced to use arith_inst and then */
/* add a bit of code to make sure it was MOV and not the other ones */

| arith_inst '[' REG '+' ']' ',' '[' REG '+' ']' {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (1)");
	size = find_size0(inst_size);
	$$ = 2;
	op[0] = 0x90 + size * 8;
	op[1] = reg_indirect($3) * 16 + reg_indirect($8);
  }
| arith_inst WORD ',' '[' REG ']' {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (2)");
	size = find_size0(inst_size);
	$$ = 3;
	op[0] = 0xA0 + size * 8;
	op[1] = 128 + reg_indirect($5) * 16 + msb(direct_addr($2));
	op[2] = lsb(direct_addr($2));
	RELOC_DIR_07FF(MEM_POS+1, 0);
  }
| arith_inst '[' REG ']' ',' WORD {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (3)");
	size = find_size0(inst_size);
	$$ = 3;
	op[0] = 0xA0 + size * 8;
	op[1] = reg_indirect($3) * 16 + msb(direct_addr($6));
	op[2] = lsb(direct_addr($6));
	RELOC_DIR_07FF(MEM_POS+1, 0);
  }
| arith_inst WORD ',' WORD {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (4)");
	size = find_size0(inst_size);
	$$ = 4;
	op[0] = 0x97 + size * 8;
	op[1] = msb(direct_addr($2)) * 16 + msb(direct_addr($4));
	op[2] = lsb(direct_addr($2));
	op[3] = lsb(direct_addr($4));
	RELOC_DIR_70FF(MEM_POS+1, 0);
	RELOC_DIR_0700FF(MEM_POS+1, 1);
  }
| arith_inst REG ',' USP {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (5)");
	$$ = 2;
	op[0] = 0x90;
	op[1] = reg($2) * 16 + 15;
  }
| arith_inst USP ',' REG {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (6)");
	$$ = 2;
	op[0] = 0x98;
	op[1] = reg($4) * 16 + 15;
  }
| arith_inst C ',' bit {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (7)");
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x20 + msb(bit_addr($4));
	op[2] = lsb(bit_addr($4));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }
| arith_inst bit ',' C {
	/* this addr mode is only valid for MOV */
	if (arith_opcode != 8) error("Addr mode only valid for MOV (8)");
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x30 + msb(bit_addr($2));
	op[2] = lsb(bit_addr($2));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }

| MOVC REG ',' '[' REG '+' ']' {
	size = find_size1(inst_size, $2);
	$$ = 2;
	op[0] = 0x80 + size * 8;
	op[1] = reg($2) * 16 + reg_indirect($5);
  }
| MOVC A ',' '[' A '+' DPTR ']' {
	$$ = 2;
	op[0] = 0x90;
	op[1] = 0x4E;
  }
| MOVC A ',' '[' A '+' PC ']' {
	$$ = 2;
	op[0] = 0x90;
	op[1] = 0x4C;
  }
| MOVX REG ',' '[' REG ']' {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = 0xA7 + size * 8;
	op[1] = reg($2) * 16 + reg_indirect($5);
  }
| MOVX '[' REG ']' ',' REG {
	$$ = 2;
	size = find_size1(inst_size, $6);
	op[0] = 0xA7 + size * 8;
	op[1] = reg($6) * 16 + 8 + reg_indirect($3);
  }
| XCH REG ',' REG {
	$$ = 2;
	size = find_size2(inst_size, $2, $4);
	op[0] = 0x60 + size * 8;
	op[1] = reg($2) * 16 + reg($4);
  }
| XCH REG ',' '[' REG ']' {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = 0x50 + size * 8;
	op[1] = reg($2) * 16 + reg_indirect($5);
  }
| XCH REG ',' WORD {
	$$ = 3;
	size = find_size1(inst_size, $2);
	op[0] = 0xA0 + size * 8;
	op[1] = reg($2) * 16 + msb(direct_addr($4));
        op[2] = lsb(direct_addr($4));
	RELOC_DIR_07FF(MEM_POS+1, 0);
  }
| short_data_inst REG ',' '#' expr {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = short_opcode + size * 8 + 1;
	op[1] = reg($2) * 16 + imm_data4_signed($5);
	RELOC_ABS_0F(MEM_POS+1, 0);
  }
| short_data_inst '[' REG ']' ',' '#' expr {
	$$ = 2;
	size = find_size0(inst_size);
	op[0] = short_opcode + size * 8 + 2;
	op[1] = reg_indirect($3) * 16 + imm_data4_signed($7);
	RELOC_ABS_0F(MEM_POS+1, 0);
  }
| short_data_inst '[' REG '+' ']' ',' '#' expr {
	$$ = 2;
	size = find_size0(inst_size);
	op[0] = short_opcode + size * 8 + 3;
	op[1] = reg_indirect($3) * 16 + imm_data4_signed($8);
	RELOC_ABS_0F(MEM_POS+1, 0);
  }
| short_data_inst '[' REG '+' expr ']' ',' '#' expr {
	size = find_size0(inst_size);
	if ($5 >= -128 && $5 <= 127) {
		$$ = 3;
		op[0] = short_opcode + size * 8 + 4;
		op[1] = reg_indirect($3) * 16 + imm_data4_signed($9);
		op[2] = op[2] = ($5 >= 0) ? $5 : 256 + $5;
		RELOC_ABS_0F(MEM_POS+1, 1);
		RELOC_ABS_FF(MEM_POS+2, 0);
	} else {
		$$ = 4;
		op[0] = short_opcode + size * 8 + 5;
		op[1] = reg_indirect($3) * 16 + imm_data4_signed($9);
		op[2] = ($5 >= 0) ? msb($5) : msb(65536 + $5);
		op[3] = ($5 >= 0) ? lsb($5) : lsb(65536 + $5);
		RELOC_ABS_0F(MEM_POS+1, 1);
		RELOC_ABS_FFFF(MEM_POS+2, 0);
	}
  }
| short_data_inst expr ',' '#' expr {
	$$ = 3;
	size = find_size0(inst_size);
	op[0] = short_opcode + size * 8 + 6;
	op[1] = msb(direct_addr($2)) * 16 + imm_data4_signed($5);
	op[2] = lsb(direct_addr($2));
	RELOC_ABS_0F(MEM_POS+1, 0);
  }
| ANL C ',' bit {
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x40 + msb(bit_addr($4));
	op[2] = lsb(bit_addr($4));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }
| ANL C ',' '/' bit {
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x50 + msb(bit_addr($5));
	op[2] = lsb(bit_addr($5));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }

| ORL C ',' bit {
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x60 + msb(bit_addr($4));
	op[2] = lsb(bit_addr($4));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }
| ORL C ',' '/' bit {
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x70 + msb(bit_addr($5));
	op[2] = lsb(bit_addr($5));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }
| CLR bit {
	$$ = 3;
	op[0] = 0x08;
	op[1] = msb(bit_addr($2));
	op[2] = lsb(bit_addr($2));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }
| SETB bit {
	$$ = 3;
	op[0] = 0x08;
	op[1] = 0x10 + msb(bit_addr($2));
	op[2] = lsb(bit_addr($2));
	RELOC_BIT_03FF(MEM_POS+1, 0);
  }
| logical_shift_inst REG ',' REG {
	size = find_size1(inst_size, $2);
	if (find_size_reg($4) != SIZE8)
		error("Second register in logical shift must be byte size");
	$$ = 2;
	op[0] = shift_reg_opcode;
	switch (size) {
		case SIZE8:  op[0] += 0; break;
		case SIZE16: op[0] += 8; break;
		case SIZE32: op[0] += 12; break;
	}
	op[1] = reg($2) * 16 + reg($4);
  }
| logical_shift_inst REG ',' '#' NUMBER {
        size = find_size1(inst_size, $2);
        $$ = 2;
	if (shift_imm_opcode == -1)
		error("NORM may not use a constant");
        op[0] = shift_imm_opcode;
        switch (size) {
                case SIZE8:  op[0] += 0; break;
                case SIZE16: op[0] += 8; break;
                case SIZE32: op[0] += 12; break;
        }
	switch (size) {
		case SIZE8:
		case SIZE16:
			op[1] = reg($2) * 16 + imm_data4_unsigned($5);
			break;
		case SIZE32:
			op[1] = (reg($2) / 2) * 32 + imm_data5_unsigned($5);
			break;
	}
  }
| no_opperand_inst {
	$$ = num_op;
	op[0] = opcode0;
	op[1] = opcode1;
  }

| TRAP '#' NUMBER {
	$$ = 2;
	op[0] = 0xD6;
	op[1] = 0x30 + imm_data4_unsigned($3);
  }
| CPL REG {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = 0x90 + size * 8;
	op[1] = reg($2) * 16 + 10;
  }
| DA REG {
	$$ = 2;
	op[0] = 0x90;
	op[1] = reg($2) * 16 + 8;
  }
| NEG REG {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = 0x90 + size * 8;
	op[1] = reg($2) * 16 + 11;
  }
| SEXT REG {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = 0x90 + size * 8;
	op[1] = reg($2) * 16 + 9;
  }

| rotate_inst REG ',' '#' NUMBER {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = rotate_opcode + size * 8;
	op[1] = reg($2) * 16 + imm_data4_unsigned($5);
  }


| LEA REG ',' REG '+' expr {
	if ($6 >= -128 && $6 <= 127) {
		$$ = 3;
		op[0] = 0x40;
		op[1] = reg($2) * 16 + reg_indirect($4);
		op[2] = ($6 >= 0) ? $6 : 256 + $6;
		RELOC_ABS_FF(MEM_POS+2, 0);
	} else {
		op[0] = 0x48;
		op[1] = reg($2) * 16 + reg_indirect($4);
		op[2] = ($6 >= 0) ? msb($6) : msb(65536 + $6);
		op[3] = ($6 >= 0) ? lsb($6) : lsb(65536 + $6);
		RELOC_ABS_FFFF(MEM_POS+2, 0);
	}
  }
| stack_inst WORD {
	$$ = 3;
	size = find_size0(inst_size);
	op[0] = msb(stack_addr_opcode) + size * 8;
	op[1] = lsb(stack_addr_opcode) + msb(direct_addr($2));
	op[2] = lsb(direct_addr($2));
	RELOC_DIR_07FF(MEM_POS+1, 0);
  }
| stack_inst rlist {
	$$ = 2;
	if (inst_size != UNKNOWN && find_size0(inst_size) != rlist_size)
		error("inst specifies different size than registers used");
	op[0] = stack_reg_opcode + rlist_reg_bank * 64 + rlist_size * 8;
	op[1] = rlist_bitmask;
  }


| MUL REG ',' REG {
	$$ = 2;
	size = find_size2(inst_size, $2, $4);
	op[0] = 0xE6;
	op[1] = reg($2) * 16 + reg($4);
  }
| MULU REG ',' REG {
	$$ = 2;
	size = find_size2(inst_size, $2, $4);
	if (size == SIZE8) {
		op[0] = 0xE0;
		op[1] = reg($2) * 16 + reg($4);
	} else {
		op[0] = 0xE4;
		op[1] = reg($2) * 16 + reg($4);
	}
  }
| MUL REG ',' '#' expr {
	$$ = 2;
	size = find_size1(inst_size, $2);
	op[0] = 0xE9;
	op[1] = reg($2) + 8;
	op[2] = msb(imm_data16($5));
	op[3] = lsb(imm_data16($5));
	RELOC_ABS_FFFF(MEM_POS+2, 0);
  }
| MULU REG ',' '#' expr {
	size = find_size2(inst_size, $2, $4);
	if (size == SIZE8) {
		$$ = 3;
		op[0] = 0xE8;
		op[1] = reg($2) * 16;
		op[2] = imm_data8($5);
		RELOC_ABS_FF(MEM_POS+2, 0);
	} else {
		$$ = 4;
		op[0] = 0xE9;
		op[1] = reg($2) * 16;
		op[2] = msb(imm_data16($5));
		op[3] = lsb(imm_data16($5));
		RELOC_ABS_FFFF(MEM_POS+2, 0);
	}
  }
| DIV REG ',' REG {
	$$ = 2;
	size = find_size2(inst_size, $2, $4);
	switch (size) {
	case SIZE8:
		error("Signed DIV can't be 8 bit size"); break;
	case SIZE16:
		op[0] = 0xE7;
		op[1] = reg($2) * 16 + reg($4);
		break;
	case SIZE32:
		op[0] = 0xEF;
		op[1] = (reg($2) / 2) * 32 + reg($4);
		break;
	}
  }
| DIVU REG ',' REG {
	$$ = 2;
	size = find_size2(inst_size, $2, $4);
	switch (size) {
	case SIZE8:
		op[0] = 0xE1;
		op[1] = reg($2) * 16 + reg($4);
		break;
	case SIZE16:
		op[0] = 0xE5;
		op[1] = reg($2) * 16 + reg($4);
		break;
	case SIZE32:
		op[0] = 0xED;
		op[1] = (reg($2) / 2) * 32 + reg($4);
		break;
	}
  }
| DIV REG ',' '#' expr { 
	size = find_size1(inst_size, $2);
	switch (size) {
	case SIZE8:
		error("Singed DIV can't be 8 bit size"); break;
	case SIZE16:
		$$ = 3;
		op[0] = 0xE8;
		op[1] = reg($2) * 16 + 11;
		op[2] = imm_data8($5);
		RELOC_ABS_FF(MEM_POS+2, 0);
		break;
	case SIZE32:
		$$ = 4;
		op[0] = 0xE9;
		op[1] = (reg($2) / 2) * 32 + 9;
		op[2] = msb(imm_data16($5));
		op[3] = lsb(imm_data16($5));
		RELOC_ABS_FFFF(MEM_POS+2, 0);
		break;
	}
  }
| DIVU REG ',' '#' expr { 
	size = find_size1(inst_size, $2);
	switch (size) {
	case SIZE8:
		$$ = 3;
		op[0] = 0xE8;
		op[1] = reg($2) * 16 + 1;
		op[2] = imm_data8($5);
		RELOC_ABS_FF(MEM_POS+2, 0);
		break;
	case SIZE16:
		$$ = 3;
		op[0] = 0xE8;
		op[1] = reg($2) * 16 + 3;
		op[2] = imm_data8($5);
		RELOC_ABS_FF(MEM_POS+2, 0);
		break;
	case SIZE32:
		$$ = 4;
		op[0] = 0xE9;
		op[1] = (reg($2) / 2) * 32 + 1;
		op[2] = msb(imm_data16($5));
		op[3] = lsb(imm_data16($5));
		RELOC_ABS_FFFF(MEM_POS+2, 0);
		break;
	}
  }
| CALL '[' REG ']' {
	$$ = 2;
	op[0] = 0xC6;
	op[1] = reg($3);
  }
| FCALL jmpaddr {
	$$ = 4;
	op[0] = 0xC4;
	op[1] = ($2 >> 8) & 255;
	op[2] = $2 & 255;
	op[3] = ($2 >> 16) & 255;
  }
| FJMP jmpaddr {
	$$ = 4;
	op[0] = 0xD4;
	op[1] = ($2 >> 8) & 255;
	op[2] = $2 & 255;
	op[3] = ($2 >> 16) & 255;
  }
| JMP '[' REG ']' {
	$$ = 2;
	op[0] = 0xD6;
	op[1] = 0x70 + reg_indirect($3);
  }
| JMP '[' A '+' DPTR ']' {
	$$ = 2;
	op[0] = 0xD6;
	op[1] = 0x46;
  }
| JMP '[' '[' REG '+' ']' ']' {
	$$ = 2;
	op[0] = 0xD6;
	op[1] = 0x60 + reg_indirect($4);
  }

| JMP jmpaddr {
	$$ = 3;
	op[0] = 0xD5;
	op[1] = msb(rel16(MEM_POS + $$, $2));
	op[2] = lsb(rel16(MEM_POS + $$, $2));
	RELOC_FFFF(MEM_POS+1,MEM_POS+$$,0);
  }

| CALL jmpaddr {
        $$ = 3;
        op[0] = 0xC5;
        op[1] = msb(rel16(MEM_POS + $$, $2));
        op[2] = lsb(rel16(MEM_POS + $$, $2));
	RELOC_FFFF(MEM_POS+1, MEM_POS+$$, 0);
  }
| branch_inst jmpaddr {
	$$ = 2;
	op[0] = branch_opcode;
	op[1] = rel8(MEM_POS + $$, $2);
	RELOC_FF(MEM_POS+1,MEM_POS + $$, 0);
  }
| CJNE REG ',' expr ',' jmpaddr {
        $$ = 4;
	size = find_size1(inst_size, $2);
	op[0] = 0xE2 + size * 8;
	op[1] = reg($2) * 16 + msb(direct_addr($4));
	op[2] = lsb(direct_addr($4));
	op[3] = rel8(MEM_POS + $$, $6);
	RELOC_DIR_07FF(MEM_POS+1, 0);
	RELOC_FF(MEM_POS+3, MEM_POS + $$, 1);
  }
| CJNE REG ',' '#' expr ',' jmpaddr {
	size  = find_size1(inst_size, $2);
	if (size == SIZE8) {
		$$ = 4;
		op[0] = 0xE3;
		op[1] = reg($2) * 16;
		op[2] = rel8(MEM_POS + $$, $7);
		op[3] = imm_data8($5);
		RELOC_ABS_FF(MEM_POS+3, 0);
	} else {
		$$ = 5;
		op[0] = 0xEB;
		op[1] = reg($2) * 16;
		op[2] = rel8(MEM_POS + $$, $7);
		op[3] = msb(imm_data16($5));
		op[4] = lsb(imm_data16($5));
		RELOC_ABS_FFFF(MEM_POS+3, 0);
	}
  }
| CJNE '[' REG ']' ',' '#' expr ',' jmpaddr {
	size  = find_size0(inst_size);
	if (size == SIZE8) {
		$$ = 4;
		op[0] = 0xE3;
		op[1] = reg_indirect($3) * 16 + 8;
		op[2] = rel8(MEM_POS + $$, $9);
		op[3] = imm_data8($7);
		RELOC_ABS_FF(MEM_POS+3, 0);
	} else {
		$$ = 5;
		op[0] = 0xEB;
		op[1] = reg_indirect($3) * 16 + 8;
		op[2] = rel8(MEM_POS + $$, $9);
		op[3] = msb(imm_data16($7));
		op[4] = lsb(imm_data16($7));
		RELOC_ABS_FFFF(MEM_POS+3, 0);
	}
  }
| DJNZ REG ',' jmpaddr {
	$$ = 3;
	size  = find_size1(inst_size, $2);
	op[0] = 0x87 + size * 8;
	op[1] = reg($2) * 16 + 8;
	op[2] = rel8(MEM_POS + $$, $4);
	RELOC_FF(MEM_POS+2, MEM_POS+$$, 0);
  }


| DJNZ WORD ',' jmpaddr {
	$$ = 4;
	size  = find_size0(inst_size);
	op[0] = 0xE2 + size * 8;
	op[1] = msb(direct_addr($2)) + 8;
	op[2] = lsb(direct_addr($2));
	op[3] = rel8(MEM_POS + $$, $4);
	RELOC_DIR_07FF(MEM_POS+1, 0);
	RELOC_FF(MEM_POS+3, MEM_POS + $$, 1)
  }

| JB bit ',' jmpaddr {
	$$ = 4;
	op[0] = 0x97;
	op[1] = 0x80 + msb(bit_addr($2));
	op[2] = lsb(bit_addr($2));
	op[3] = rel8(MEM_POS + $$, $4);
	RELOC_BIT_03FF(MEM_POS+1, 0);
	RELOC_FF(MEM_POS+3, MEM_POS + $$, 1);
  }

| JBC bit ',' jmpaddr {
	$$ = 4;
	op[0] = 0x97;
	op[1] = 0xC0 + msb(bit_addr($2));
	op[2] = lsb(bit_addr($2));
	op[3] = rel8(MEM_POS + $$, $4);
	RELOC_BIT_03FF(MEM_POS+1, 0);
	RELOC_FF(MEM_POS+3, MEM_POS + $$, 1);
  }

| JNB bit ',' jmpaddr {
	$$ = 4;
	op[0] = 0x97;
	op[1] = 0xA0 + msb(bit_addr($2));
	op[2] = lsb(bit_addr($2));
	op[3] = rel8(MEM_POS + $$, $4);
	RELOC_BIT_03FF(MEM_POS+1, 0);
	RELOC_FF(MEM_POS+3, MEM_POS + $$, 1);
  }


arith_inst:
	  ADD	{arith_opcode = 0;}
	| ADDC	{arith_opcode = 1;}
	| AND	{arith_opcode = 5;}
	| CMP	{arith_opcode = 4;}
	| MOV	{arith_opcode = 8;}
	| OR	{arith_opcode = 6;}
	| SUB	{arith_opcode = 2;}
	| SUBB	{arith_opcode = 3;}
	| XOR	{arith_opcode = 7;}

short_data_inst:
	  ADDS {short_opcode = 0xA0;}
	| MOVS {short_opcode = 0xB0;}

logical_shift_inst:
	  ASL  {shift_reg_opcode = 0xC1; shift_imm_opcode = 0xD1;}
	| ASR  {shift_reg_opcode = 0xC2; shift_imm_opcode = 0xD2;}
	| LSR  {shift_reg_opcode = 0xC0; shift_imm_opcode = 0xD0;}
	| NORM {shift_reg_opcode = 0xC3; shift_imm_opcode = -1;}

rotate_inst:
	  RL	{rotate_opcode = 0xD3;}
	| RLC	{rotate_opcode = 0xD7;}
	| RR	{rotate_opcode = 0xD0;}
	| RRC	{rotate_opcode = 0xD7;}

stack_inst:
	  POP	{stack_addr_opcode = 0x8710; stack_reg_opcode = 0x27;}
	| POPU	{stack_addr_opcode = 0x8700; stack_reg_opcode = 0x37;}
	| PUSH	{stack_addr_opcode = 0x8730; stack_reg_opcode = 0x07;}
	| PUSHU	{stack_addr_opcode = 0x8720; stack_reg_opcode = 0x17;}

no_opperand_inst:
	  BKPT 	{num_op = 1; opcode0 = 255; opcode1 = 0;}
	| NOP	{num_op = 1; opcode0 = 0; opcode1 = 0;}
	| RESET	{num_op = 2; opcode0 = 0xD6; opcode1 = 0x10;}
	| RET	{num_op = 2; opcode0 = 0xD6; opcode1 = 0x80;}
	| RETI	{num_op = 2; opcode0 = 0xD6; opcode1 = 0x90;}

branch_inst:
	  BCC	{branch_opcode = 0xF0;}
	| BCS	{branch_opcode = 0xF1;}
	| BEQ	{branch_opcode = 0xF3;}
	| BG	{branch_opcode = 0xF8;}
	| BGE	{branch_opcode = 0xFA;}
	| BGT	{branch_opcode = 0xFC;}
	| BL	{branch_opcode = 0xF9;}
	| BLE	{branch_opcode = 0xFD;}
	| BLT	{branch_opcode = 0xFB;}
	| BMI	{branch_opcode = 0xF7;}
	| BNE	{branch_opcode = 0xF2;}
	| BNV	{branch_opcode = 0xF4;}
	| BOV	{branch_opcode = 0xF5;}
	| BPL	{branch_opcode = 0xF6;}
	| BR	{branch_opcode = 0xFE;}
	| JZ	{branch_opcode = 0xEC;}
	| JNZ	{branch_opcode = 0xEE;}



%%


int reg(int reg_spec)
{
	return reg_spec & 15;
}

int reg_indirect(int reg_spec)
{
	if (reg_spec & BYTE_REG)
		error("Indirect addressing may not use byte registers");
	if ((reg_spec & 15) > 7)
		error("Only R0 through R7 may be used for indirect addr");
	return reg_spec & 7;
}

int rel16(int pos, int dest)
{
	int rel;

	if (!p3) return 0;	/* don't bother unless writing code */
	if (dest & (BRANCH_SPACING - 1))
		error("Attempt to jump to unaligned location");
	pos &= ~(BRANCH_SPACING - 1);
	rel = (dest - pos) / BRANCH_SPACING;
	if (rel < -32768 || rel > 32767)
		error("Attempt to jump out of 16 bit relative range");
	if (rel < 0) rel += 65536;
	return rel;
}

int rel8(int pos, int dest)
{
	int rel;

	if (!p3) return 0;	/* don't bother unless writing code */
	if (dest & (BRANCH_SPACING - 1))
		error("Attempt to jump to unaligned location");
	pos &= ~(BRANCH_SPACING - 1);
	rel = (dest - pos) / BRANCH_SPACING;
	if (rel < -128 || rel > 127)
		error("Attempt to jump out of 16 bit relative range");
	if (rel < 0) rel += 256;
	return rel;
}

int msb(int value)
{
	return (value >> 8) & 255;
}

int lsb(int value)
{
	return value & 255;
}

int direct_addr(int value)
{
	char buf[250];

	if (value < 0 || value > 2047) {
		sprintf(buf, "illegal value (%d) for direct address", value);
		error(buf);
	}
	return value;
}

int imm_data4_signed(int value)
{
	if (value < -8 || value > 7)
		error("illegal 4 bit (signed) value");
	if (value >= 0) return value;
	else return (16 + value);
}

int imm_data4_unsigned(int value)
{
	if (value < 0 || value > 15)
		error("illegal 4 bit (unsigned) value");
	return value;
}

int imm_data5_unsigned(int value)
{
	if (value < 0 || value > 31)
		error("illegal 5 bit (unsigned) value");
	return value;
}

int imm_data8(int value)
{
	if (value < -128 || value > 255)
		error("illegal 8 bit value");
	if (value >= 0) return value;
	else return (256 + value);
}

int imm_data16(int value)
{
	if (value < -32728 || value > 65535)
		error("illegal 16 bit value");
	if (value >= 0) return value;
	else return (65536 + value);
}

int bit_addr(int value)
{
	if (value < 0 || value > 1023) {
		fprintf(stderr, "bad bit addr of 0x%04X (%d dec)\n",
			value, value);
		error("illegal bit address");
	}
	return value;
}


int find_size_reg(int op1spec)
{
	int op1size=UNKNOWN;

	if (op1spec & BYTE_REG) op1size = SIZE8;
	if (op1spec & WORD_REG) op1size = SIZE16;
	if (op1size == UNKNOWN)
		error("Register without implied size");
	return op1size;
}

int find_size0(int isize)
{
	if (isize == UNKNOWN)
		error("Can't determine data size from instruction");
	return isize;
}

int find_size1(int isize, int op1spec)
{
	int op1size=UNKNOWN;

	if (op1spec & BYTE_REG) op1size = SIZE8;
	if (op1spec & WORD_REG) op1size = SIZE16;
	if (op1size == UNKNOWN)
		error("Register without implied size");

	if (isize == SIZE32 && op1size == SIZE16) return SIZE32;
	if (isize == UNKNOWN) return op1size;
	else {
		if (isize != op1size)
			error("data size of register and inst don't agree");
		return isize;
	}
}

int find_size2(int isize, int op1spec, int op2spec)
{
	int op1size=UNKNOWN, op2size=UNKNOWN;

	if (op1spec & BYTE_REG) op1size = SIZE8;
	if (op1spec & WORD_REG) op1size = SIZE16;
	if (op1size == UNKNOWN)
		error("Register without implied size");
	if (op2spec & BYTE_REG) op2size = SIZE8;
	if (op2spec & WORD_REG) op2size = SIZE16;
	if (op1size == UNKNOWN)
		error("Register without implied size");

	if (op1size != op2size)
		error("data sizes of two registers don't agree");
	if (isize == UNKNOWN) return op1size;
	else {
		if (isize != op1size)
			error("data size of registers and inst don't agree");
		return isize;
	}
}

int yyerror(char *s)
{
	if (yytext[0] >= 32) {
		fprintf(stderr, "%s near '%s', line %d\n",
			s, yytext, lineno);
	} else {
		fprintf(stderr, "%s, line %d\n", s, lineno - 1);
	}
	return 0;
}

void error(char *s)
{
	yyerror(s);
	exit(1);
}

int yywrap()
{
	return 1;
}
