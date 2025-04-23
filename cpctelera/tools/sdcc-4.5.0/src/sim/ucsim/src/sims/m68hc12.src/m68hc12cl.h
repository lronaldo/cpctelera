/*
 * Simulator of microcontrollers (m68hc12cl.h)
 *
 * Copyright (C) 2020 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#ifndef M68HC12CL_HEADER
#define M68HC12CL_HEADER

#include "uccl.h"
#include "memcl.h"

#include "m68hc11cl.h"

#define CL12 cl_m68hc12

class CL12;
class cl_hc12_cpu;

typedef int (*hcwrapper_fn)(class CL12 *uc, t_mem code);

enum {
  flagStop	= 0x80,
  flagX		= 0x40
};

#define rTMP2 (TMP2)
#define rTMP3 (TMP3)

/*
 * Special handling of CCR
 */

class cl_ccr: public cl_cell8
{
public:
  virtual t_mem write(t_mem val);
};


/*
 * Base of M68HC12 processor
 *
 * uc -> m6800         -> m68hcbase   -> m68hc12
 *       A,B,CC,X,SP      D,Y
 */

class cl_m68hc12: public cl_m68hcbase
{
public:
  //i8_t post_inc_dec;
  //class cl_cell16 *post_idx_reg;
  class cl_wrap *hc12wrap;
  u16_t TMP2, TMP3;
  class cl_cell16 cTMP2, cTMP3;
  class cl_memory_cell *tex_cells[8], *loop_cells[8];
  const char *tex_names[8], *loop_names[8];
  u16_t XIRQ_AT, COP_AT, TRAP_AT, CMR_AT;
  class cl_hc12_cpu *cpu12;
  int extra_ticks, xb_tick_shift;
  bool block_irq;
  u8_t rev_st, rd_Rx, Rx, rd_Fy, Fy; // REV state
public:
  cl_m68hc12(class cl_sim *asim);
  virtual int init(void);
  virtual const char *id_string(void);
  virtual void reset(void);
  virtual void make_memories(void);
  virtual void setup_ccr(void) {}
  virtual void make_cpu_hw(void);

  virtual double def_xtal(void) { return 8000000; }
  
  virtual int proba(int,t_mem);
  virtual int prob1(int,t_mem) {return 1;}

  virtual struct dis_entry *dis_tbl(void);
  virtual struct dis_entry *get_dis_entry(t_addr addr);
  virtual char *disassc(t_addr addr, chars *comment=NULL);
  virtual void disass_xb(t_addr *addr, chars *work, chars *comment, int len, int corr= 0, u32_t use_PC=0);
  virtual void disass_b7(t_addr *addr, chars *work, chars *comment);
  virtual char *disass_loop(t_addr *addr, chars *work, chars *comment);
  
  virtual int inst_length(t_addr addr);
  virtual int longest_inst(void) { return 6; }

  virtual void pre_inst(void);
  virtual void pre_emu(void);
  virtual int exec_inst(void);
  virtual void post_inst(void);
  virtual void post_emu(void);
  virtual i16_t s8_16(u8_t op); // sex 8->16
  virtual int xb_type(u8_t p);
  virtual bool xb_indirect(u8_t p);
  virtual bool xb_PC(u8_t p);
  virtual t_addr naddr(t_addr *addr, u8_t *pg, u32_t use_PC= 0);
  virtual u8_t xbop8();
  virtual u16_t xbop16();
  virtual class cl_memory_cell &xb(void);
  virtual class cl_memory_cell &xbdst(void) { vc.wr++; return xb(); }
  virtual t_addr xbaddr(void) { return naddr(NULL, NULL); }
  virtual void print_regs(class cl_console_base *con);

  virtual void push_regs(bool inst_part);
  virtual void pull_regs(bool inst_part);
  virtual int exec_b7(void);
  virtual int trap(t_mem code);

  // ALU
  virtual int sub16(class cl_cell16 &dest, u16_t op);
  virtual int add16(class cl_cell16 &dest, u16_t op);
  virtual int cp16(u16_t op1, u16_t op2);
  virtual int lsr16(class cl_memory_cell &dest);
  virtual int asl16(class cl_memory_cell &dest);
  virtual int inxy(class cl_memory_cell &dest);
  virtual int dexy(class cl_memory_cell &dest);
  virtual int ediv(void);
  virtual int mul(void);
  virtual int emul(void);
  virtual int daa(void);
  virtual int idiv(void);
  virtual int fdiv(void);
  virtual int emacs(void);
  virtual int emuls(void);
  virtual int edivs(void);
  virtual int idivs(void);
  virtual int maxa(void);
  virtual int mina(void);
  virtual int emaxd(void);
  virtual int emind(void);
  virtual int maxm(void);
  virtual int minm(void);
  virtual int emaxm(void);
  virtual int eminm(void);
  virtual int tbl(void);
  virtual int etbl(void);
  virtual int mem(void);
  virtual int rev(void);
  
  // MOVE
#define ld16 ldsx
  virtual int i_psh8(u8_t op);
  virtual int i_pul8(class cl_memory_cell &dest);
  virtual int i_psh16(u16_t op);
  virtual int i_pul16(class cl_memory_cell &dest);
  virtual int movw_imid(void);
  virtual int movb_imid(void);
  virtual int movw_exid(void);
  virtual int movb_exid(void);
  virtual int movw_idid(void);
  virtual int movb_idid(void);
  virtual int movw_imex(void);
  virtual int movb_imex(void);
  virtual int movw_exex(void);
  virtual int movb_exex(void);
  virtual int movw_idex(void);
  virtual int movb_idex(void);
  
  // BRANCH
  virtual int call_e(void);
  virtual int call_id(void);
  virtual int brset(class cl_memory_cell &m);
  virtual int brset_d(void);
  virtual int brset_id(void);
  virtual int brset_e(void);
  virtual int brclr(class cl_memory_cell &m);
  virtual int brclr_d(void);
  virtual int brclr_id(void);
  virtual int brclr_e(void);
  virtual int branch(t_addr a, bool cond);
  virtual int jump(t_addr a);
  virtual int bsr(void);
  virtual int jsr(t_addr a);
  virtual int rtc(void);
  virtual int rts(void);
  virtual int swi(void);
  virtual int rti(void);
  virtual int lbranch(u8_t code);
  virtual int loop(u8_t code);
  
  // OTHER
  virtual int andcc(u8_t op);
  virtual int orcc(u8_t op);
  virtual int lea(class cl_memory_cell &dest);
};


enum hc12cpu_cfg {
  hc12_cpu_nuof	= 0
};

class cl_hc12_cpu: public cl_hw
{
protected:
  class cl_m68hc12 *muc;
  class cl_memory_cell *dpage, *ppage, *epage, *windef;
public:
  cl_hc12_cpu(class cl_uc *auc);
  virtual int init(void);
  virtual void reset(void);
  virtual void print_info(class cl_console_base *con);
  virtual t_mem ppage_write(u8_t val);
  virtual u8_t ppage_read(void) { return ppage->R(); }
};


#endif

/* End of m68hc12.src/m68hc12cl.h */
