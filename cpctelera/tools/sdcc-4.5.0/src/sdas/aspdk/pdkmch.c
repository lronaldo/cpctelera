/* pdkmch.c */

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
 */

#include "asxxxx.h"
#include "pdk.h"

static VOID outpdkaw(struct inst inst, struct expr e) {
        outaw(inst.op | (e.e_addr & inst.mask));
}

static VOID outpdkrm(struct inst inst, struct expr e) {
        /* Don't generate relocatable data if everything is constant. */
        if (is_abs(&e)) {
                outpdkaw(inst, e);
        } else {
                outrwp(&e, inst.op, inst.mask, /*jump=*/0);
        }
}

static VOID outpdka(struct inst inst) {
        struct expr e;
        clrexpr(&e);
        outpdkaw(inst, e);
}

VOID emov(a_uint op,
          struct inst def,
          struct inst ioa,
          struct inst aio,
          struct inst ma,
          struct inst am) {
        bool ioAdr = op & PDK_OPCODE_ADR_IO;
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, ioAdr);
        comma(1);
        int t1 = addr(&e1, ioAdr);

        if (t == S_IO && t1 == S_A) {
                outpdkaw(ioa, e);
        } else
        if (t == S_A && t1 == S_IO) {
                outpdkaw(aio, e1);
        } else
        if (t == S_M && t1 == S_A) {
                outpdkrm(ma, e);
        } else
        if (t == S_A && t1 == S_M) {
                outpdkrm(am, e1);
        } else
        if (t == S_A && t1 == S_K) {
                outpdkrm(def, e1);
        } else
                aerr();
}

VOID eidxm(struct inst am, struct inst ma) {
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, false);
        comma(1);
        int t1 = addr(&e1, false);

        if (t == S_A && t1 == S_M) {
                outpdkrm(am, e1);
        } else
        if (t == S_M && t1 == S_A) {
                outpdkrm(ma, e);
        } else
                aerr();
}

VOID earith(struct inst def,
            struct inst ma,
            struct inst am) {
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, false);
        comma(1);
        int t1 = addr(&e1, false);

        if (t == S_M && t1 == S_A) {
                outpdkrm(ma, e);
        } else
        if (t == S_A && t1 == S_M) {
                outpdkrm(am, e1);
        } else
        if (t == S_A && t1 == S_K) {
                outpdkrm(def, e1);
        } else
                aerr();
}

VOID earithc(struct inst ma,
             struct inst am,
             struct inst m,
             struct inst a) {
        struct expr e;
        clrexpr(&e);
        int t = more() ? addr(&e, false) : S_A;

        if (comma(0)) {
                struct expr e1;
                clrexpr(&e1);
                int t1 = addr(&e1, false);
                if (t == S_M && t1 == S_A) {
                        outpdkrm(ma, e);
                } else
                if (t == S_A && t1 == S_M) {
                        outpdkrm(am, e1);
                } else
                        aerr();
        } else
        if (t == S_M) {
                outpdkrm(m, e);
        } else
        if (t == S_A) {
                outpdka(a);
        } else
                aerr();
}

VOID eshift(struct inst a,
            struct inst m) {
        struct expr e;
        clrexpr(&e);
        int t = more() ? addr(&e, false) : S_A;

        if (t == S_A) {
                outpdka(a);
        } else
        if (t == S_M) {
                outpdkrm(m, e);
        } else
                aerr();
}

VOID ebit(a_uint op,
          struct inst def,
          struct inst ma,
          struct inst am,
          struct inst *ioa) {
        bool ioAdr = op & PDK_OPCODE_ADR_IO;
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, ioAdr), t1 = 0;

        if (!more()) {
                if (t == S_K) {
                        t = S_A;
                        t1 = S_K;
                        e1 = e;
                } else if (t == S_IO) {
                        t1 = S_A;
                } else
                        aerr();
        } else {
                comma(1);
                t1 = addr(&e1, ioAdr);
        }

        if (t == S_M && t1 == S_A) {
                outpdkrm(ma, e);
        } else
        if (t == S_A && t1 == S_M) {
                outpdkrm(am, e1);
        } else
        if (t == S_A && t1 == S_K) {
                outpdkrm(def, e1);
        } else
        if (t == S_IO && t1 == S_A && ioa) {
                outpdkaw(*ioa, e);
        } else 
                aerr();
}

VOID enot(struct inst def, struct inst m) {
        struct expr e;
        clrexpr(&e);
        int t = more() ? addr(&e, false) : S_A;

        if (t == S_M) {
                outpdkrm(m, e);
        } else if (t == S_A) {
                outpdka(def);
        } else
                aerr();
}

VOID ebitn(a_uint op, struct inst io, struct inst m, int offset) {
        bool ioAdr = op & PDK_OPCODE_ADR_IO;
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, ioAdr);
        comma(1);
        if (pdkbit(&e1) != S_K)
                aerr();

        const a_uint bitn = (e1.e_addr & 0x7) << offset;
        if (t == S_IO) {
                io.op |= bitn;
                outpdkaw(io, e);
        } else
        if (t == S_M) {
                m.op |= bitn;
                outpdkrm(m, e);
        } else
                aerr();
}

VOID eskip(struct inst def, struct inst m) {
        struct expr e;
        clrexpr(&e);
        int t = addr(&e, false);
        if (t == S_A) {
                comma(1);
                t = addr(&e, false);
        }

        if (t == S_M) {
                outpdkrm(m, e);
        } else
        if (t == S_K) {
                outpdkrm(def, e);
        } else
                aerr();
}

VOID ezsn(struct inst def, struct inst m) {
        /* IZSN and DZSN insts have the same params as NOT. */
        enot(def, m);
}

VOID eret(struct inst def, struct inst k) {
        if (more()) {
                struct expr e;
                clrexpr(&e);
                if (addr(&e, false) != S_K)
                        aerr();
                outpdkrm(k, e);
        } else
                outpdka(def);
}

VOID eone(struct inst m) {
        struct expr e;
        clrexpr(&e);
        if (addr(&e, false) != S_M)
                aerr();

        outpdkrm(m, e);
}

VOID exch(struct inst m) {
        struct expr e;
        clrexpr(&e);

        int t = addr(&e, false);
        if (t == S_A) {
                comma(1);
                t = addr(&e, false);
        }

        if (t != S_M)
                aerr();
        outpdkrm(m, e);
}

VOID epupo(struct inst def) {
        if (more() && (def.op & 0x2000)) {
                int t = getnb();
                if (t != 'a')
                        aerr();
                if (!more() || ((t = getnb()) != 'f'))
                        aerr();
                def.op &= 0x1FFF;
        }
        outpdka(def);
}

VOID eopta(struct inst def) {
        if (more()) {
                struct expr e;
                clrexpr(&e);
                if (addr(&e, false) != S_A)
                        aerr();
        }
        outpdka(def);
}

VOID eswapc(a_uint op, struct inst iok, int offset) {
        bool ioAdr = op & PDK_OPCODE_ADR_IO;
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, ioAdr);
        comma(1);
        int t1 = pdkbit(&e1);

        if (t != S_IO || t1 != S_K)
                aerr();

        iok.op |= (e1.e_addr & 0x7) << offset;
        outpdkaw(iok, e);
}

VOID espec(struct inst am, struct inst ma) {
        struct expr e, e1;
        clrexpr(&e);
        clrexpr(&e1);

        int t = addr(&e, false);
        comma(1);
        int t1 = addr(&e1, false);

        if (t == S_A && t1 == S_M) {
                outpdkrm(am, e1);
        } else
        if (t == S_M && t1 == S_A) {
                outpdkrm(ma, e);
        } else
                aerr();
}

