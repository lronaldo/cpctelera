/*
 * Simulator of microcontrollers (p1516.cc)
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

#include <stdlib.h>

#include "dregcl.h"

#include "glob.h"
#include "portcl.h"
#include "uartcl.h"
#include "clockcl.h"
#include "timercl.h"
#include "fpgacl.h"
#include "brd_ctrlcl.h"

#include "p1516cl.h"


cl_pc_write::cl_pc_write(class cl_memory_cell *acell, class cl_uc *the_uc):
  cl_memory_operator(acell)
{
  uc= the_uc;
}

t_mem
cl_pc_write::write(t_mem val)
{
  uc->set_PC(val);
  return uc->PC;
}


cl_p1516::cl_p1516(class cl_sim *asim):
  cl_uc(asim)
{
  PCmask= 0xffffffff;
  r2b_state= r2b_none;
}

int
cl_p1516::init(void)
{
  int i;
  cl_uc::init();
  for (i=0; i<16; i++)
    R[i]= 0;
  reg_cell_var(&cF, &F, "F", "Flag register");
  class cl_pc_write *pcw= new cl_pc_write(&cPC, this);
  pcw->init();
  cPC.append_operator(pcw);
  return 0;
}

const char *
cl_p1516::id_string(void)
{
  return "P1516";
}

void
cl_p1516::reset(void)
{
  cl_uc::reset();
  
  PC= R[15]= 0;
  F= 0;//F&= ~(U|P);
}
  
void
cl_p1516::set_PC(t_addr addr)
{
  PC= R[15]= addr;
}

void
cl_p1516::mk_hw_elements(void)
{
  class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();
  
  add_hw(pa= new cl_porto(this, 0xff00, "pa"));
  pa->init();
  add_hw(pb= new cl_porto(this, 0xff01, "pb"));
  pb->init();
  add_hw(pc= new cl_porto(this, 0xff02, "pc"));
  pc->init();
  add_hw(pd= new cl_porto(this, 0xff03, "pd"));
  pd->init();

  add_hw(pi= new cl_porti(this, 0xff20, "pi"));
  pi->init();
  add_hw(pj= new cl_porti(this, 0xff10, "pj"));
  pj->init();

  add_hw(h= new cl_timer(this, 0xff30, "timer"));
  h->init();
  
  add_hw(h= new cl_uart(this, 0, 0xff40));
  h->init();

  add_hw(h= new cl_clock(this, 0xff50, "clock"));
  h->init();
  
  class cl_port_ui *u= new cl_port_ui(this, 0, "dport");
  u->init();
  add_hw(u);
  class cl_port_ui *uo= new cl_port_ui(this, 0, "oports");
  uo->init();
  add_hw(uo);
  class cl_port_ui *ui= new cl_port_ui(this, 0, "iports");
  ui->init();
  add_hw(ui);

  class cl_port_data d;
  d.init();
  d.cell_dir= NULL;
  d.width= 32;
  
  d.set_name("PA");
  d.cell_p = pa->dr;
  d.cell_in= pa->dr;
  d.keyset = NULL;
  d.basx   = 15;
  d.basy   = 4;
  u->add_port(&d, 0);
  uo->add_port(&d, 0);
  
  d.set_name("PB");
  d.cell_p = pb->dr;
  d.cell_in= pb->dr;
  d.keyset = NULL;
  d.basx   = 15;
  d.basy   = 9;
  u->add_port(&d, 1);
  uo->add_port(&d, 1);

  d.set_name("PC");
  d.cell_p = pc->dr;
  d.cell_in= pc->dr;
  d.keyset = NULL;
  d.basx   = 15;
  d.basy   = 14;
  u->add_port(&d, 2);
  uo->add_port(&d, 2);

  d.set_name("PD");
  d.cell_p = pd->dr;
  d.cell_in= pd->dr;
  d.keyset = NULL;
  d.basx   = 15;
  d.basy   = 19;
  u->add_port(&d, 3);
  uo->add_port(&d, 3);

  d.set_name("PI");
  d.cell_p = pi->dr;
  d.cell_in= pi->cfg_cell(port_pin);
  d.keyset = chars("                qwertyui12345678");
  d.basx   = 15;
  d.basy   = 24;
  u->add_port(&d, 4);
  d.basy   = 4;
  ui->add_port(&d, 0);

  d.set_name("PJ");
  d.cell_p = pj->dr;
  d.cell_in= pj->cfg_cell(port_pin);
  d.keyset = chars("                asdfghjkzxcvbnm,");
  d.basx   = 15;
  d.basy   = 29;
  u->add_port(&d, 5);
  d.basy   = 10;
  ui->add_port(&d, 1);

  add_hw(h= new cl_n4(this, 0, "n4ddr"));
  h->init();
  add_hw(h= new cl_bool(this, 0, "boolean"));
  h->init();
  add_hw(h= new cl_logsys(this, 0, "logsys"));
  h->init();

  add_hw(bc= new cl_brd_ctrl(this, 0, "brd_ctrl"));
  h->init();
}

void
cl_p1516::make_memories(void)
{
  class cl_address_space *as;
  int i;

  //double st= dnow();
  rom= as= new cl_address_space("rom"/*MEM_ROM_ID*/, 0, 0x20000, 32);
  as->init();
  //printf("%f\n", dnow()-st);
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_chip32("rom_chip", 0x20000, 32, 0);
  chip->init();
  memchips->add(chip);
  rom_chip= chip;
  ad= new cl_address_decoder(as= rom, chip, 0, /*0xfeff*/0x1ffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  /*ad= new cl_address_decoder(as= rom, chip, 0x10000, 0x1ffff, 0x10000);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);*/

  regs= new cl_address_space("regs", 0, 16, 32);
  regs->init();
  for (i= 0; i<16; i++)
    {
      RC[i]= regs->get_cell(i);
      RC[i]->decode((t_mem*)&R[i]);
    }
  address_spaces->add(regs);

  chars n, d;
  for (i=0; i<16; i++)
    {
      n.format("R%d", i);
      d.format("CPU register %d", i);
      vars->add(n, regs, i, d);
    }
}


struct dis_entry *
cl_p1516::dis_tbl(void)
{
  return(disass_p1516);
}

char *
cl_p1516::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars();
  const char *b;
  t_mem code, data= 0;
  int i;

  code= rom->get(addr);
  
  i= 0;
  while ((code & dis_tbl()[i].mask) != dis_tbl()[i].code &&
	 dis_tbl()[i].mnemonic)
    i++;
  if (dis_tbl()[i].mnemonic == NULL)
    {
      return strdup("-- UNKNOWN/INVALID");
    }
  b= dis_tbl()[i].mnemonic;

  data= (code&0xf0000000)>>28;
  if (((data & 1) == 0) || (dis_tbl()[i].branch == 'M'))
    work.append("   ");
  else
    {
      switch (data>>2)
	{
	case 0: work.append("S"); break;
	case 1: work.append("C"); break;
	case 2: work.append("Z"); break;
	case 3: work.append("O"); break;
	}
      if (data&2)
	work.append("1 ");
      else
	work+= "0 ";
    }

  while (*b)
    {
      if (*b == '%')
	{
	  b++;
	  switch (*(b++))
	    {
	    case 'd': // Rd
	      data= (code & 0x00f00000)>>20;
	      work.appendf("r%d", data);
	      break;
	    case 'a': // Ra
	      data= (code & 0x000f0000)>>16;
	      work.appendf("r%d", data);
	      break;
	    case 'R': // Ra in LD, ST
	      data= (code & 0x000f0000)>>16;
	      work.appendf("r%d", data);
	      if (comment)
		{
		  chars n= "";
		  addr_name(R[data], rom, &n);
		  comment->format("; [0x%08x%s]= 0x%08x",
				  R[data],
				  n.c_str(),
				  rom->get(R[data]));
		}
	      break;
	    case 'b': // Rb
	      data= (code & 0x0000f000)>>12;
	      work.appendf("r%d", data);
	      break;
	    case '0': // LDL0
	      data= (code & 0x0000ffff);
	      work.appendf("0x0000%04x", data);
	      addr_name(data, rom, &work);
	      break;
	    case 'O': // LDL0 -> jump
	      data= (code & 0x0000ffff);
	      work.appendf("0x%04x", data);
	      addr_name(data, rom, &work);
	      break;
	    case 'l': // LDL
	      data= (code & 0x0000ffff);
	      work.appendf("0x....%04x", data);
	      break;
	    case 'h': // LDH
	      data= (code & 0x0000ffff);
	      work.appendf("0x%04x....", data);
	      break;
	    case 'A': // CALL
	      data= (code & 0x07ffffff);
	      work.appendf("0x%x", data);
	      addr_name(data, rom, &work);
	      break;
	    default:
	      temp= "?";
	      break;
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work.append(*(b++));
    }

  return strdup(work.c_str());
}

void
cl_p1516::analyze_start(void)
{
  class cl_var *v= new cl_var(".reset", rom, 0, chars("Auto-generated by analyze"), -1, -1);
  v->init();
  vars->add(v);
  
  analyze(0);
}

void
cl_p1516::analyze(t_addr addr)
{
  struct dis_entry *de;
  int i;
  
  while (!inst_at(addr))
    {
      t_mem code= rom->read(addr);
      i= 0;
      while ((code & dis_tbl()[i].mask) != dis_tbl()[i].code &&
	     dis_tbl()[i].mnemonic)
	i++;
      de= &dis_tbl()[i];
      if (de->mnemonic == NULL)
	return;
      set_inst_at(addr);
      if (de->branch!=' ')
	switch (de->branch)
	  {
	  case 'x': case '_': // non-followable
	    return;
	  case 'M': // LDL0 r15,#imm
	    {
	      t_addr target= rom->read(addr) & 0xffff;
	      analyze_jump(addr, target, de->branch);
	      break;
	    }
	  }
      addr= rom->validate_address(addr+1);
    }
}

void
cl_p1516::print_regs(class cl_console_base *con)
{
  int i;
  con->dd_color("answer");
  con->dd_printf("  F= 0x%08x  ", F);
  con->dd_printf("O=%c ", (F&O)?'1':'0');
  con->dd_printf("C=%c ", (F&C)?'1':'0');
  con->dd_printf("Z=%c ", (F&Z)?'1':'0');
  con->dd_printf("S=%c ", (F&S)?'1':'0');
  con->dd_printf("\n");
  for (i= 0; i<16; i++)
    {
      if (i<10) con->dd_printf(" ");
      con->dd_printf("R%d= 0x%08x ", i, R[i]);
      if (i<10) con->dd_printf(" ");
      con->dd_printf("[R%d]= 0x%08x", i, rom->get(R[i]));
      if (i%2)
	con->dd_printf("\n");
      else
	con->dd_printf(" ");
    }
  print_disass(PC, con);
}


bool
cl_p1516::cond(t_mem code)
{
  t_mem cond= (code & 0xf0000000) >> 28;
  if ((cond&1) == 1)
    {
      u8_t flag= 0, fv, v;
      switch (cond>>2)
	{
	case 0: flag= F&S; break;
	case 1: flag= F&C; break;
	case 2: flag= F&Z; break;
	case 3: flag= F&O; break;
	}
      fv= flag?1:0;
      v= (cond&2)?1:0;
      if (fv != v)
	return false;
    }
  return true;
}

t_mem
cl_p1516::inst_ad(t_mem ra, t_mem rb, u32_t c)
{
  u64_t big;
  u32_t rd;
  
  big= (u64_t)ra + (u64_t)rb + (u64_t)c;
  F&= ~(C|S|Z|O);
  if (big > 0xffffffff)
    F|= C;
  rd= big;
  if (rd == 0)
    F|= Z;
  if (rd & 0x80000000)
    F|= S;
  big= (ra & 0x7fffffff) + (rb & 0x7fffffff) + c;
  if ((big & 0x80000000) && !(F&C))
    F|= O;
  if (!(big & 0x80000000) && (F&C))
    F|= O;
  cF.W(F);
  return rd;
}

int
cl_p1516::inst_alu(t_mem code)
{
  u8_t d, a, b, Op;
  u8_t c1, c2;
  u64_t big;
  
  d= (code & 0x00f00000) >> 20;
  a= (code & 0x000f0000) >> 16;
  b= (code & 0x0000f000) >> 12;
  Op=(code & 0x00000f80) >> 7;

  switch (Op)
    {
    case 0: // ADD
      RC[d]->W(inst_ad(R[a], R[b], 0));
      break;
    case 1: // ADC
      RC[d]->W(inst_ad(R[a], R[b], (F&C)?1:0));
      break;
    case 2: // SUB
      RC[d]->W(inst_ad(R[a], ~(R[b]), 1));
      break;
    case 3: // SBB
      RC[d]->W(inst_ad(R[a], ~(R[b]), (F&C)?1:0));
      break;

    case 4: // INC
      RC[d]->W(inst_ad(R[a], 1, 0));
      break;
    case 5: // DEC
      RC[d]->W(inst_ad(R[a], ~(1), 1));
      break;

    case 6: // AND
      RC[d]->W(R[a] & R[b]);
      SET_Z(R[d]);
      break;
    case 7: // OR
      RC[d]->W(R[a] | R[b]);
      SET_Z(R[d]);
      break;
    case 8: // XOR
      RC[d]->W(R[a] ^ R[b]);
      SET_Z(R[d]);
      break;

    case 9: // SHL
      SET_C(R[a] & 0x80000000);
      RC[d]->W(R[a] << 1);
      SET_Z(R[d]);
      break;
    case 10: // SHR
      SET_C(R[a] & 1);
      RC[d]->W(R[a] >> 1);
      SET_Z(R[d]);
      break;
    case 16: // SHA
      SET_C(R[a] & 1);
      RC[d]->W(((i32_t)(R[a])) >> 1);
      SET_Z(R[d]);
      break;
    case 11: // ROL
      c1= (F&C)?1:0;
      c2= (R[a] & 0x80000000)?1:0;
      RC[d]->W((R[a]<<1) + c1);
      SET_C(c2);
      SET_Z(R[d]);
      break;
    case 12: // ROR
      c1= (F&C)?1:0;
      c2= R[a] & 1;
      R[d]= R[a] >> 1;
      if (c1)
	R[d]|= 0x80000000;
      RC[d]->W(R[d]);
      SET_C(c2);
      SET_Z(R[d]);
      break;

    case 13: // MUL
      RC[d]->W(R[a] * R[b]);
      SET_Z(R[d]);
      SET_S(R[d] & 0x80000000);
      break;
    case 19: // MUH
      big= (u64_t)R[a] * (u64_t)R[b];
      RC[d]->W(big >> 32);
      SET_Z(R[d]);
      SET_S(R[d] & 0x80000000);
      break;
    case 14: // DIV
      break;

    case 17: // SETC
      SET_C(1);
      break;
    case 18: // CLRC
      SET_C(0);
      break;
      
    case 15: // CMP
      inst_ad(R[a], ~(R[b]), 1);
      break;
    }
  
  return resGO;
}

int
cl_p1516::exec_inst(void)
{
  t_mem code;
  u8_t inst;
  bool fe;
  
  PC= R[15];
  instPC= PC;
  fe= fetch(&code);
  tick(1);
  R[15]= PC;
  if (fe)
    return(resBREAKPOINT);

  if (!cond(code))
    return resGO;
  
  inst= (code & 0x0f000000) >> 24;
  if (code & 0x08000000)
    {
      // CALL
      t_addr data= (code & 0x07ffffff);
      RC[14]->W(R[15]);
      RC[15]->W(PC= data);
      return resGO;
    }

  u8_t d, a;
  d= (code & 0x00f00000) >> 20;
  a= (code & 0x000f0000) >> 16;
  switch (inst)
    {
    case 0: // nop
      break;
    case 1: // LD Rd,Ra
      R[d]= rom->read(R[a]);
      vc.rd++;
      break;
    case 2: // ST Rd,Ra
      rom->write(R[a], R[d]);
      vc.wr++;
      break;
    case 3: // MOV Rd,Ra
      RC[d]->W(R[a]);
      break;
    case 4: // LDL0 Rd,data
      RC[d]->W(code & 0x0000ffff);
      break;
    case 5: // LDL Rd,data
      RC[d]->W((R[d] & 0xffff0000) | (code & 0x0000ffff));
      break;
    case 6: // LDH Rd,data
      RC[d]->W((R[d] & 0x0000ffff) | (code << 16));
      break;
    case 7: // ALU
      inst_alu(code);
      break;
    }
  PC= R[15];
  
  return resGO;
}


void
cl_p1516::btn_edge(int btn, bool press)
{
  switch (r2b_state)
    {
    case r2b_none:
      if ((btn==1) && press)
	r2b_state= r2b_alarmed;
      break;
    case r2b_alarmed:
      if ((btn==1) && !press)
	r2b_state= r2b_none;
      if ((btn==0) && press)
	{
	  reset();
	  r2b_state= r2b_activated;
	}
      break;
    case r2b_activated:
      if ((btn==1) && !press)
	r2b_state= r2b_none;
      break;
    }
}


/* End of p1516.src/p1516.cc */
