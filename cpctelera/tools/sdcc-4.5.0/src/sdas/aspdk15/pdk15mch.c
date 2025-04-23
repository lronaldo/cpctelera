/* pdk15mch.c */

/*
 *  Copyright (C) 1998-2011  Alan R. Baldwin
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
 *  Benny Kim (2011/07/21)
 *  bennykim at coreriver dot com
 *  Fixed bugs in relative address with "."
 */

#include "sdas.h"
#include "asxxxx.h"
#include "pdk.h"

char    *cpu    = "Padauk 15";
char    *dsft   = "asm";

/*
 * Process machine ops.
 */
VOID
machine(struct mne *mp)
{
        a_uint op, opWithFlags;
        int combine;

        opWithFlags = mp->m_valu;
        op = opWithFlags & PDK_OPCODE_MASK;
        combine = 0;

        /* Default instructions are only used for A -> K instructions.
         * Although they may be (ab)used for other types.
         */
        struct inst def = {op, 0xFF};
        switch (mp->m_type) {

        case S_MOV: {
                struct inst ioa = {0x0100, 0x7F};
                struct inst aio = {0x0180, 0x7F};
                struct inst ma = {0x1700, 0xFF};
                struct inst am = {0x1F00, 0xFF};
                emov(opWithFlags, def, ioa, aio, ma, am);
                break;
        }

        case S_IDXM: {
                struct inst am = {op | 1, 0xFE};
                struct inst ma = {op, 0xFE};
                eidxm(am, ma);
                break;
        }

        case S_SUB:
                combine = 0x100;
                /* fallthrough */
        case S_ADD: {
                struct inst ma = {0x1000 | combine, 0xFF};
                struct inst am = {0x1800 | combine, 0xFF};
                earith(def, ma, am);
                break;
        }

        case S_SUBC:
                combine = 0x100;
                /* fallthrough */
        case S_ADDC: {
                struct inst ma = {0x1200 | combine, 0xFF};
                struct inst am = {0x1A00 | combine, 0xFF};
                struct inst m = {0x2000 | combine, 0xFF};
                struct inst a = {0x0060 + (combine ? 1 : 0), 0x00};
                earithc(ma, am, m, a);
                break;
        }

        case S_SLC:
        case S_SRC:
        case S_SL:
        case S_SR: {
                if (mp->m_type == S_SRC || mp->m_type == S_SLC)
                        combine = 2;
                if (mp->m_type == S_SL || mp->m_type == S_SLC)
                        combine += 1;

                struct inst a = {0x006A + combine, 0x00};
                struct inst m = {0x2A00 + (combine << 8), 0xFF};
                eshift(a, m);
                break;
        }

        case S_OR:
        case S_XOR:
        case S_AND: {
                if (mp->m_type == S_OR) {
                        combine = 0x100;
                } else
                if (mp->m_type == S_XOR) {
                        combine = 0x200;
                }

                struct inst ma = {0x1400 | combine, 0xFF};
                struct inst am = {0x1C00 | combine, 0xFF};
                struct inst ioa = {0x0080, 0x7F};
                ebit(opWithFlags, def, ma, am, mp->m_type == S_XOR ? &ioa : NULL);
                break;
        }

        case S_NEG:
                combine = 0x100;
                /* fallthrough */
        case S_NOT: {
                struct inst m = {0x2800 | combine, 0xFF};
                enot(def, m);
                break;
        }
        case S_SWAP: {
                struct inst m = {0x0a00, 0xFF};
                enot(def, m);
                break;
        }

        case S_SET1:
                combine = 0x400;
                /* fallthrough */
        case S_SET0: {
                struct inst io = {0x3800 | combine, 0x7F};
                struct inst m = {0x4800 | combine, 0x7F};
                ebitn(opWithFlags, io, m, /*N offset*/7);
                break;
        }

        case S_CNEQSN:
                combine = 0x100;
                /* fallthrough */
        case S_CEQSN: {
                struct inst m = {0x2E00 | combine, 0xFF};
                eskip(def, m);
                break;
        }

        case S_T1SN:
                combine = 0x400;
                /* fallthrough */
        case S_T0SN: {
                struct inst io = {0x3000 | combine, 0x7F};
                struct inst m = {0x4000 | combine, 0x7F};
                ebitn(opWithFlags, io, m, /*N offset*/7);
                break;
        }

        case S_DZSN:
                combine = 0x100;
                /* fallthrough */
        case S_IZSN: {
                struct inst m = {0x2200 | combine, 0xFF};
                ezsn(def, m);
                break;
        }

        case S_RET: {
                struct inst k = {0x0200, 0xFF};
                eret(def, k);
                break;
        }

        case S_LDTABL:
        case S_LDTABH:
        case S_LDT16:
        case S_STT16:
                def.mask = 0xFE;
                /* fallthrough */
        case S_INC:
        case S_DEC:
        case S_CLEAR:
                eone(def);
                break;

        case S_CALL:
        case S_GOTO: {
                struct expr e;
                clrexpr(&e);
                waddrmode = 1;
                expr(&e, 0);
                waddrmode = 0;
                outrwp(&e, op, 0xFFF, /*jump=*/1);
                break;
        }

        case S_XCH:
                exch(def);
                break;

        case S_PUSHAF:
        case S_POPAF:
                epupo(def);
                break;

        case S_PCADD:
              eopta(def);
              break;

        case S_SWAPC:
              def.mask = 0x7F;
              eswapc(opWithFlags, def, /*N offset*/7);
              break;

        case S_COMP:
        case S_NADD: {
              struct inst am = {op, 0xFF};
              struct inst ma = {op | 0x100, 0xFF};
              espec(am, ma);
              break;
        }

        /* Simple instructions consisting of only one opcode and no args */
        case S_RETI:
        case S_NOP:
        case S_ENGINT:
        case S_DISGINT:
        case S_STOPSYS:
        case S_STOPEXE:
        case S_RESET:
        case S_WDRESET:
        case S_MUL:
        case S_LDSPTL: /* undocumented */
        case S_LDSPTH: /* undocumented */
                outaw(op);
                break;
        }
}

/*
 * Machine specific initialization
 */

VOID
minit(void)
{
        /*
         * Byte Order
         */
        hilo = 0;

        /*
         * Address Space
         */
        exprmasks(3);

        /* Set the target in case it was not automatically
         * configured from the executable filename.
         */
        set_sdas_target(TARGET_ID_PDK15);
}

