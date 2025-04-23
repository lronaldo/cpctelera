/* pdk.h */

/*
 *  Copyright (C) 1998-2009  Alan R. Baldwin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 *
 *   This Assember Ported by
 *      John L. Hartman (JLH)
 *      jhartman at compuserve dot com
 *      noice at noicedebugger dot com
 *
 */

/* Contains common functionality for the pdk architecture. */

#include <stdbool.h>
/*
 * Instructions.
 */
#define S_MOV     50
#define S_LDT16   51
#define S_STT16   52
#define S_IDXM    53
#define S_XCH     54
#define S_PUSHAF  55
#define S_POPAF   56
#define S_ADD     57
#define S_ADDC    58
#define S_SUB     59
#define S_SUBC    60
#define S_INC     61
#define S_DEC     62
#define S_CLEAR   63
#define S_SR      64
#define S_SRC     65
#define S_SL      66
#define S_SLC     67
#define S_SWAP    68
#define S_AND     69
#define S_OR      70
#define S_XOR     71
#define S_NOT     72
#define S_NEG     73
#define S_SET0    74
#define S_SET1    75
#define S_CEQSN   76
#define S_T0SN    77
#define S_T1SN    78
#define S_IZSN    79
#define S_DZSN    80
#define S_CALL    81
#define S_GOTO    82
#define S_RET     83
#define S_RETI    84
#define S_NOP     85
#define S_PCADD   86
#define S_ENGINT  87
#define S_DISGINT 88
#define S_STOPSYS 89
#define S_STOPEXE 90
#define S_RESET   91
#define S_WDRESET 92
#define S_SWAPC   93  /* not on pdk13 */
#define S_CNEQSN  94 
#define S_LDSPTL  95
#define S_LDSPTH  96
#define S_NADD    97  /* not on pdk13 */
#define S_COMP    98  /* not on pdk13 */
#define S_MUL     99
#define S_DELAY   100 /* not on pdk15 */
#define S_PMODE   101 /* not on pdk15 */
#define S_POPWPC  102 /* not on pdk15 */
#define S_PUSHWPC 103 /* not on pdk15 */
#define S_PUSHW   104 /* not on pdk15 */
#define S_POPW    105 /* not on pdk15 */
#define S_IGOTO   106 /* not on pdk15 */
#define S_ICALL   107 /* not on pdk15 */
#define S_LDTABL  108 /* not on pdk14 */
#define S_LDTABH  109 /* not on pdk14 */
#define S_TOG     110 /* not on pdk15 */
#define S_WAIT0   111 /* not on pdk15 */
#define S_WAIT1   112 /* not on pdk15 */
#define S_NMOV    113 /* not on pdk15 */

/*
 * Addressing modes.
 */
#define S_K    31
#define S_A    32
#define S_M    33
#define S_IO   34

#define PDK_OPCODE_ADR_IO 0x80000000
#define PDK_OPCODE_MASK     0xFFFFFF

struct inst {
        a_uint op;    /* opcode of instruction */
        a_uint mask;  /* mask of parameter for instruction */
};

#ifdef OTHERSYSTEM

/* Codegen functions to emit instructions. */
extern VOID emov(a_uint op,
          struct inst def,
          struct inst ioa,
          struct inst aio,
          struct inst ma,
          struct inst am);
extern VOID eidxm(struct inst am, struct inst ma);
extern VOID earith(struct inst def, struct inst ma, struct inst am);
extern VOID earithc(struct inst ma, struct inst am, struct inst m, struct inst a);
extern VOID eshift(struct inst a, struct inst m);
extern VOID ebit(a_uint op, struct inst def, struct inst ma, struct inst am, struct inst *ioa);
extern VOID enot(struct inst def, struct inst m);
extern VOID ebitn(a_uint op, struct inst io, struct inst m, int offset);
extern VOID eskip(struct inst def, struct inst m);
extern VOID ezsn(struct inst def, struct inst m);
extern VOID eret(struct inst def, struct inst k);
extern VOID eone(struct inst m);
extern VOID exch(struct inst m);
extern VOID epupo(struct inst def);
extern VOID eopta(struct inst def);
extern VOID eswapc(a_uint op, struct inst iok, int offset);
extern VOID espec(struct inst am, struct inst ma);

extern int addr(struct expr *esp, bool ioAdr);
extern int pdkbit(struct expr *esp);

/* Addressing parsing */

#else

/* Codegen functions to emit instructions. */
extern VOID emov();
extern VOID eidxm();
extern VOID earith();
extern VOID earithc();
extern VOID eshift();
extern VOID ebit();
extern VOID enot();
extern VOID ebitn();
extern VOID eskip();
extern VOID ezsn();
extern VOID eret();
extern VOID eone();
extern VOID exch();
extern VOID epupo();
extern VOID eopta();
extern VOID eswapc();
extern VOID espec();

extern int addr();
extern int pdkbit();

#endif
