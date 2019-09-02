%{
/* xa_rasm.l - This file is part of Paul's XA51 Assembler

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

#include <stdio.h>
#include <string.h>
#include "xa_main.h"
#include "xa_rasm.tab.h"

extern int inst_size, yylval;
char line_text[MAX_LINE]={'\0'}, last_line_text[MAX_LINE];
char lex_sym_name[MAX_SYMBOL];
extern char base_symbol_name[];
#define LIST if (p3) strcat(line_text, yytext)
%}

%%

add\.w		{LIST; inst_size=SIZE16; return ADD;}
add\.b		{LIST; inst_size=SIZE8; return ADD;}
add		{LIST; inst_size=UNKNOWN; return ADD;}
addc\.w		{LIST; inst_size=SIZE16; return ADDC;}
addc\.b		{LIST; inst_size=SIZE8; return ADDC;}
addc		{LIST; inst_size=UNKNOWN; return ADDC;}
adds\.w		{LIST; inst_size=SIZE16; return ADDS;}
adds\.b		{LIST; inst_size=SIZE8; return ADDS;}
adds		{LIST; inst_size=UNKNOWN; return ADDS;}
and\.w		{LIST; inst_size=SIZE16; return AND;}
and\.b		{LIST; inst_size=SIZE8; return AND;}
and		{LIST; inst_size=UNKNOWN; return AND;}
anl		{LIST; inst_size=UNKNOWN; return ANL;}
asl\.d		{LIST; inst_size=SIZE32; return ASL;}
asl\.w		{LIST; inst_size=SIZE16; return ASL;}
asl\.b		{LIST; inst_size=SIZE8; return ASL;}
asl		{LIST; inst_size=UNKNOWN; return ASL;}
asr\.d		{LIST; inst_size=SIZE32; return ASR;}
asr\.w		{LIST; inst_size=SIZE16; return ASR;}
asr\.b		{LIST; inst_size=SIZE8; return ASR;}
asr		{LIST; inst_size=UNKNOWN; return ASR;}
bcc		{LIST; inst_size=UNKNOWN; return BCC;}
bcs		{LIST; inst_size=UNKNOWN; return BCS;}
beq		{LIST; inst_size=UNKNOWN; return BEQ;}
bg		{LIST; inst_size=UNKNOWN; return BG;}
bge		{LIST; inst_size=UNKNOWN; return BGE;}
bgt		{LIST; inst_size=UNKNOWN; return BGT;}
bkpt		{LIST; inst_size=UNKNOWN; return BKPT;}
bl		{LIST; inst_size=UNKNOWN; return BL;}
ble		{LIST; inst_size=UNKNOWN; return BLE;}
blt		{LIST; inst_size=UNKNOWN; return BLT;}
bmi		{LIST; inst_size=UNKNOWN; return BMI;}
bne		{LIST; inst_size=UNKNOWN; return BNE;}
bnv		{LIST; inst_size=UNKNOWN; return BNV;}
bov		{LIST; inst_size=UNKNOWN; return BOV;}
bpl		{LIST; inst_size=UNKNOWN; return BPL;}
br		{LIST; inst_size=UNKNOWN; return BR;}
call		{LIST; inst_size=UNKNOWN; return CALL;}
cjne\.w		{LIST; inst_size=SIZE16; return CJNE;}
cjne\.b		{LIST; inst_size=SIZE8; return CJNE;}
cjne		{LIST; inst_size=UNKNOWN; return CJNE;}
clr		{LIST; inst_size=UNKNOWN; return CLR;}
cmp\.w		{LIST; inst_size=SIZE16; return CMP;}
cmp\.b		{LIST; inst_size=SIZE8; return CMP;}
cmp		{LIST; inst_size=UNKNOWN; return CMP;}
cpl\.w		{LIST; inst_size=SIZE16; return CPL;}
cpl\.b		{LIST; inst_size=SIZE8; return CPL;}
cpl		{LIST; inst_size=UNKNOWN; return CPL;}
da\.b		{LIST; inst_size=SIZE8; return DA;}
da		{LIST; inst_size=UNKNOWN; return DA;}
div\.d		{LIST; inst_size=SIZE32; return DIV;}
div\.w		{LIST; inst_size=SIZE16; return DIV;}
div\.b		{LIST; inst_size=SIZE8; return DIV;}
div		{LIST; inst_size=UNKNOWN; return DIV;}
divu\.d		{LIST; inst_size=SIZE32; return DIVU;}
divu\.w		{LIST; inst_size=SIZE16; return DIVU;}
divu\.b		{LIST; inst_size=SIZE8; return DIVU;}
divu		{LIST; inst_size=UNKNOWN; return DIVU;}
djnz\.w		{LIST; inst_size=SIZE16; return DJNZ;}
djnz\.b		{LIST; inst_size=SIZE8; return DJNZ;}
djnz		{LIST; inst_size=UNKNOWN; return DJNZ;}
fcall		{LIST; inst_size=UNKNOWN; return FCALL;}
fjmp		{LIST; inst_size=UNKNOWN; return FJMP;}
jb		{LIST; inst_size=UNKNOWN; return JB;}
jbc		{LIST; inst_size=UNKNOWN; return JBC;}
jmp		{LIST; inst_size=UNKNOWN; return JMP;}
jnb		{LIST; inst_size=UNKNOWN; return JNB;}
jnz		{LIST; inst_size=UNKNOWN; return JNZ;}
jz		{LIST; inst_size=UNKNOWN; return JZ;}
lea\.w		{LIST; inst_size=SIZE16; return LEA;}
lea		{LIST; inst_size=UNKNOWN; return LEA;}
lsr\.d		{LIST; inst_size=SIZE32; return LSR;}
lsr\.w		{LIST; inst_size=SIZE16; return LSR;}
lsr\.b		{LIST; inst_size=SIZE8; return LSR;}
lsr		{LIST; inst_size=UNKNOWN; return LSR;}
mov\.w		{LIST; inst_size=SIZE16; return MOV;}
mov\.b		{LIST; inst_size=SIZE8; return MOV;}
mov		{LIST; inst_size=UNKNOWN; return MOV;}
movc\.w		{LIST; inst_size=SIZE16; return MOVC;}
movc\.b		{LIST; inst_size=SIZE8; return MOVC;}
movc		{LIST; inst_size=UNKNOWN; return MOVC;}
movs\.w		{LIST; inst_size=SIZE16; return MOVS;}
movs\.b		{LIST; inst_size=SIZE8; return MOVS;}
movs		{LIST; inst_size=UNKNOWN; return MOVS;}
movx\.w		{LIST; inst_size=SIZE16; return MOVX;}
movx\.b		{LIST; inst_size=SIZE8; return MOVX;}
movx		{LIST; inst_size=UNKNOWN; return MOVX;}
mul\.d		{LIST; inst_size=SIZE32; return MUL;}
mul\.w		{LIST; inst_size=SIZE16; return MUL;}
mul\.b		{LIST; inst_size=SIZE8; return MUL;}
mul		{LIST; inst_size=UNKNOWN; return MUL;}
mulu\.d		{LIST; inst_size=SIZE32; return MULU;}
mulu\.w		{LIST; inst_size=SIZE16; return MULU;}
mulu\.b		{LIST; inst_size=SIZE8; return MULU;}
mulu		{LIST; inst_size=UNKNOWN; return MULU;}
neg\.w		{LIST; inst_size=SIZE16; return NEG;}
neg\.b		{LIST; inst_size=SIZE8; return NEG;}
neg		{LIST; inst_size=UNKNOWN; return NEG;}
nop		{LIST; inst_size=UNKNOWN; return NOP;}
norm\.d		{LIST; inst_size=SIZE32; return NORM;}
norm\.w		{LIST; inst_size=SIZE16; return NORM;}
norm\.b		{LIST; inst_size=SIZE8; return NORM;}
norm		{LIST; inst_size=UNKNOWN; return NORM;}
or\.w		{LIST; inst_size=SIZE16; return OR;}
or\.b		{LIST; inst_size=SIZE8; return OR;}
or		{LIST; inst_size=UNKNOWN; return OR;}
orl		{LIST; inst_size=UNKNOWN; return ORL;}
pop\.w		{LIST; inst_size=SIZE16; return POP;}
pop\.b		{LIST; inst_size=SIZE8; return POP;}
pop		{LIST; inst_size=UNKNOWN; return POP;}
popu\.w		{LIST; inst_size=SIZE16; return POPU;}
popu\.b		{LIST; inst_size=SIZE8; return POPU;}
popu		{LIST; inst_size=UNKNOWN; return POPU;}
push\.w		{LIST; inst_size=SIZE16; return PUSH;}
push\.b		{LIST; inst_size=SIZE8; return PUSH;}
push		{LIST; inst_size=UNKNOWN; return PUSH;}
pushu\.w	{LIST; inst_size=SIZE16; return PUSHU;}
pushu\.b	{LIST; inst_size=SIZE8; return PUSHU;}
pushu		{LIST; inst_size=UNKNOWN; return PUSHU;}
reset		{LIST; inst_size=UNKNOWN; return RESET;}
ret		{LIST; inst_size=UNKNOWN; return RET;}
reti		{LIST; inst_size=UNKNOWN; return RETI;}
rl\.w		{LIST; inst_size=SIZE16; return RL;}
rl\.b		{LIST; inst_size=SIZE8; return RL;}
rl		{LIST; inst_size=UNKNOWN; return RL;}
rlc\.w		{LIST; inst_size=SIZE16; return RLC;}
rlc\.b		{LIST; inst_size=SIZE8; return RLC;}
rlc		{LIST; inst_size=UNKNOWN; return RLC;}
rr\.w		{LIST; inst_size=SIZE16; return RR;}
rr\.b		{LIST; inst_size=SIZE8; return RR;}
rr		{LIST; inst_size=UNKNOWN; return RR;}
rrc\.w		{LIST; inst_size=SIZE16; return RRC;}
rrc\.b		{LIST; inst_size=SIZE8; return RRC;}
rrc		{LIST; inst_size=UNKNOWN; return RRC;}
setb		{LIST; inst_size=UNKNOWN; return SETB;}
sext\.w		{LIST; inst_size=SIZE16; return SEXT;}
sext\.b		{LIST; inst_size=SIZE8; return SEXT;}
sext		{LIST; inst_size=UNKNOWN; return SEXT;}
sub\.w		{LIST; inst_size=SIZE16; return SUB;}
sub\.b		{LIST; inst_size=SIZE8; return SUB;}
sub		{LIST; inst_size=UNKNOWN; return SUB;}
subb\.w		{LIST; inst_size=SIZE16; return SUBB;}
subb\.b		{LIST; inst_size=SIZE8; return SUBB;}
subb		{LIST; inst_size=UNKNOWN; return SUBB;}
trap		{LIST; inst_size=UNKNOWN; return TRAP;}
xch\.w		{LIST; inst_size=SIZE16; return XCH;}
xch\.b		{LIST; inst_size=SIZE8; return XCH;}
xch		{LIST; inst_size=UNKNOWN; return XCH;}
xor\.w		{LIST; inst_size=SIZE16; return XOR;}
xor\.b		{LIST; inst_size=SIZE8; return XOR;}
xor		{LIST; inst_size=UNKNOWN; return XOR;}


dptr		{LIST; return DPTR;}
pc		{LIST; return PC;}
a		{LIST; return A;}
c		{LIST; return C;}
usp		{LIST; return USP;}

org		{LIST; return ORG;}
equ		{LIST; return EQU;}
sfr		{LIST; return SFR;}
db		{LIST; return DB;}
dw		{LIST; return DW;}
byte		{LIST; return DB;}
bit		{LIST; return BITDEF;}
reg		{LIST; return REGDEF;}
area		{LIST; return AREA;}
ds		{LIST; return DS;}
DSEG		{LIST; yylval = AREA_DSEG; return AREA_NAME;}
BSEG		{LIST; yylval = AREA_BSEG; return AREA_NAME;}
XSEG		{LIST; yylval = AREA_XSEG; return AREA_NAME;}
XISEG		{LIST; yylval = AREA_XISEG; return AREA_NAME;}
XINIT		{LIST; yylval = AREA_XINIT; return AREA_NAME;}
GSINIT		{LIST; yylval = AREA_GSINIT; return AREA_NAME;}
GSFINAL		{LIST; yylval = AREA_GSFINAL; return AREA_NAME;}
HOME		{LIST; yylval = AREA_HOME; return AREA_NAME;}
SSEG            {LIST; yylval = AREA_SSEG; return AREA_NAME;}
CSEG		{LIST; yylval = AREA_CSEG; return AREA_NAME;}
module		{LIST; return MODULE;}
globl		{LIST; return GLOBL;}
\(DATA\)	{LIST; return AREA_DESC;}
\(OVR,XDATA\)	{LIST; return AREA_DESC;}
\(BIT\)		{LIST; return AREA_DESC;}
\(XDATA\)	{LIST; return AREA_DESC;}
\(CODE\)	{LIST; return AREA_DESC;}
low		{LIST; return LOW;}
high		{LIST; return HIGH;}
>>		{LIST; return RSHIFT;}
\<\<		{LIST; return LSHIFT;}

R[0-9]		{LIST; yylval = yytext[1] - '0' + WORD_REG; return REG;}
R1[0-5]		{LIST; yylval = yytext[2] - '0' + 10 + WORD_REG; return REG;}
R[0-7]L		{LIST; yylval = (yytext[1] - '0') * 2 + BYTE_REG; return REG;}
R[0-7]H		{LIST; yylval = (yytext[1] - '0') * 2 + 1 + BYTE_REG; return REG;}


[a-z_][a-z0-9_]*	{
				LIST;
				if (is_def(yytext)) {
					yylval = get_value(yytext);
				} else {
                                        if (p1) build_sym_list(yytext);
					yylval = 0;
                                        //if (p3) error("Symbol undefined");
				}
				/* keep name in lex_sym_name since yytext */
				/* could be overwritten if the parser does */
				/* a lookahead operation */
				strcpy(lex_sym_name, yytext);
				if (is_def(lex_sym_name)) {
					yylval = get_value(lex_sym_name);
					/* return correct type if special */
					if (is_bit(lex_sym_name)) return BIT;
					if (is_reg(lex_sym_name)) return REG;
				}
                                if (p3) {
                                 if (*operand[0]) {
                                   // first one in use
                                   strcpy (operand[1], yytext);
                                 } else {
                                   strcpy (operand[0], yytext);
                                 }
                                }
				return WORD;
			}
[0-9]+\$		{
				LIST;
				/* should print error if base_symbol_name */
				/* is not defined */
				sprintf(lex_sym_name, "%s:%s",
					base_symbol_name, yytext);
				if (is_def(lex_sym_name)) {
					yylval = get_value(lex_sym_name);
				} else {
					yylval = 0;
					if (p3) error("Symbol undefined");
				}
				return WORD;
			}
[01]+[bq]		{
				LIST;
				yylval = binary2int(yytext);
				return NUMBER;
			}
0x[0-9a-f]+		{
				LIST;
				sscanf(yytext, "%*c%*c%x", &yylval);
				return NUMBER;
			}
[0-9a-f]+[h]		{
				LIST;
				sscanf(yytext, "%x%*[hH]", &yylval);
				return NUMBER;
			}
\$[0-9a-f]+		{
				LIST;
				sscanf(yytext, "$%x", &yylval);
				return NUMBER;
			}
-?[0-9]+		{
				LIST;
				sscanf(yytext, "%d", &yylval);
				return NUMBER;
			}
\'.\'			{
				LIST;
				yylval = (int)yytext[1];
				return CHAR;
			}
\'\\.\'			{
				LIST;
				switch (yytext[1]) {
					case 'n':
					case 'N': yylval = 10; break;
					case 'r':
					case 'R': yylval = 13; break;
					case '0': yylval = 0; break;
					default:  yylval = (int)yytext[1];
					/* print a warning here */
				}
				return CHAR;
                	}
\"[^"\n]*["\n]		{
				LIST;
				return STRING;
			}

;[^\n]*		{LIST; /* comments */}
[ \t\r]		{LIST; /* whitespace */}

\n		{
			strcpy(last_line_text, line_text);
			line_text[0] = '\0';
			++lineno;
			return EOL;
		}

.		{LIST; return yytext[0];}

%%
