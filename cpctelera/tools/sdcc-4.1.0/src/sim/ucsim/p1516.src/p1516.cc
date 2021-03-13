/*
 * Simulator of microcontrollers (p1516.cc)
 *
 * Copyright (C) 2020,20 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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

#include "glob.h"
#include "portcl.h"

#include "p1516cl.h"


cl_p1516::cl_p1516(class cl_sim *asim):
  cl_uc(asim)
{
}

int
cl_p1516::init(void)
{
  int i;
  cl_uc::init();
  F= 0;
  for (i=0; i<16; i++)
    R[i]= 0;
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
  PC= R[15]= 0;
}
  
void
cl_p1516::set_PC(t_addr addr)
{
  PC= R[15]= addr;
}

void
cl_p1516::mk_hw_elements(void)
{
  cl_uc::mk_hw_elements();
  add_hw(pa= new cl_porto(this, 0xf000, "pa"));
  pa->init();
  add_hw(pb= new cl_porto(this, 0xf001, "pb"));
  pb->init();
  add_hw(pc= new cl_porto(this, 0xf002, "pc"));
  pc->init();
  add_hw(pd= new cl_porto(this, 0xf003, "pd"));
  pd->init();

  add_hw(pi= new cl_porti(this, 0xe000, "pi"));
  pi->init();
  add_hw(pj= new cl_porti(this, 0xd000, "pj"));
  pj->init();

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
  uo->add_port(&d, 2);

  d.set_name("PD");
  d.cell_p = pd->dr;
  d.cell_in= pd->dr;
  d.keyset = NULL;
  d.basx   = 15;
  d.basy   = 19;
  uo->add_port(&d, 3);

  d.set_name("PI");
  d.cell_p = pi->dr;
  d.cell_in= pi->cfg_cell(port_pin);
  d.keyset = chars("                qwertyui12345678");
  d.basx   = 15;
  d.basy   = 15;
  u->add_port(&d, 2);
  d.basy   = 4;
  ui->add_port(&d, 0);

  d.set_name("PJ");
  d.cell_p = pj->dr;
  d.cell_in= pj->cfg_cell(port_pin);
  d.keyset = chars("                asdfghjkzxcvbnm,");
  d.basx   = 15;
  d.basy   = 20;
  u->add_port(&d, 3);
  d.basy   = 10;
  ui->add_port(&d, 1);
}

void
cl_p1516::make_memories(void)
{
  class cl_address_space *as;
  int i;
  
  rom= as= new cl_address_space("rom"/*MEM_ROM_ID*/, 0, 0x10000, 32);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_memory_chip("rom_chip", 0x4000, 32);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= rom/*address_space(MEM_ROM_ID)*/,
			     chip, 0, 0x3fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  regs= new cl_address_space("regs", 0, 16, 32);
  regs->init();
  for (i= 0; i<16; i++)
    {
      RC[i]= regs->get_cell(i);
      RC[i]->decode((t_mem*)&R[i]);
    }
  address_spaces->add(regs);

  class cl_var *v;
  for (i=0; i<16; i++)
    {
      v= new cl_var(chars("", "R%d", i), regs, i, chars("", "CPU register %d",i));
      v->init();
      vars->add(v);
    }
}


struct dis_entry *
cl_p1516::dis_tbl(void)
{
  return(disass_p1516);
}

char *
cl_p1516::disass(t_addr addr, const char *sep)
{
  chars work= chars(), temp= chars();
  const char *b;
  t_mem code, data= 0;
  int i;

  //work= "";
  //p= (char*)work;

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
	      temp.format("r%d", data);
	      break;
	    case 'a': // Ra
	      data= (code & 0x000f0000)>>16;
	      temp.format("r%d", data);
	      break;
	    case 'b': // Rb
	      data= (code & 0x0000f000)>>12;
	      temp.format("r%d", data);
	      break;
	    case '0': // LDL0
	      data= (code & 0x0000ffff);
	      temp.format("0x0000%04x", data);
	      break;
	    case 'O': // LDL0
	      data= (code & 0x0000ffff);
	      temp.format("0x%04x", data);
	      break;
	    case 'l': // LDL
	      data= (code & 0x0000ffff);
	      temp.format("0x....%04x", data);
	      break;
	    case 'h': // LDH
	      data= (code & 0x0000ffff);
	      temp.format("0x%04x....", data);
	      break;
	    case 'A': // CALL
	      data= (code & 0x07ffffff);
	      temp.format("0x%x", data);
	      break;
	    default:
	      temp= "?";
	      break;
	    }
	  //t= temp;
	  //while (*t) *(p++)= *(t++);
	  work+= temp;
	}
      else
	work.append(*(b++));
    }
  //*p= '\0';

  return strdup(work.c_str());
}

void
cl_p1516::print_regs(class cl_console_base *con)
{
  int i;
  con->dd_color("answer");
  con->dd_printf("  F= 0x%x  ", F);
  con->dd_printf("S=%c ", (F&S)?'1':'0');
  con->dd_printf("C=%c ", (F&C)?'1':'0');
  con->dd_printf("Z=%c ", (F&Z)?'1':'0');
  con->dd_printf("O=%c ", (F&O)?'1':'0');
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


t_mem
cl_p1516::inst_ad(t_mem ra, t_mem rb, u32_t c)
{
  u64_t big;
  u32_t rd;
  
  big= ra + rb + c;
  F= 0;
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
      RC[d]->W(inst_ad(RC[a]->R(), RC[b]->R(), 0));
      break;
    case 1: // ADC
      RC[d]->W(inst_ad(RC[a]->R(), RC[b]->R(), (F&C)?1:0));
      break;
    case 2: // SUB
      RC[d]->W(inst_ad(RC[a]->R(), ~(RC[b]->R()), 1));
      break;
    case 3: // SBB
      RC[d]->W(inst_ad(RC[a]->R(), ~(RC[b]->R()), (F&C)?1:0));
      break;

    case 4: // INC
      RC[d]->W(inst_ad(RC[a]->R(), 1, 0));
      break;
    case 5: // DEC
      RC[d]->W(inst_ad(RC[a]->R(), ~(1), 1));
      break;

    case 6: // AND
      RC[d]->W(RC[a]->R() & RC[b]->R());
      SET_Z(R[d]);
      break;
    case 7: // OR
      RC[d]->W(RC[a]->R() | RC[b]->R());
      SET_Z(R[d]);
      break;
    case 8: // XOR
      RC[d]->W(RC[a]->R() ^ RC[b]->R());
      SET_Z(R[d]);
      break;

    case 9: // SHL
      SET_C(R[a] & 0x80000000);
      RC[d]->W(RC[a]->R() << 1);
      SET_Z(R[d]);
      break;
    case 10: // SHR
      SET_C(R[a] & 1);
      RC[d]->W(RC[a]->R() >> 1);
      SET_Z(R[d]);
      break;
    case 16: // SHA
      SET_C(R[a] & 1);
      RC[d]->W(((i32_t)(RC[a]->R())) >> 1);
      SET_Z(R[d]);
      break;
    case 11: // ROL
      c1= (F&C)?1:0;
      c2= (R[a] & 0x80000000)?1:0;
      RC[d]->W((RC[a]->R()<<1) + c1);
      SET_C(c2);
      SET_Z(R[d]);
      break;
    case 12: // ROR
      c1= (F&C)?1:0;
      c2= R[a] & 1;
      RC[d]->W(RC[a]->R() >> 1);
      if (c1)
	R[d]|= 0x80000000;
      SET_C(c2);
      SET_Z(R[d]);
      break;

    case 13: // MUL
      RC[d]->W(RC[a]->R() * RC[b]->R());
      SET_Z(R[d]);
      SET_S(R[d] & 0x80000000);
      break;
    case 19: // MUH
      big= RC[a]->R() * RC[b]->R();
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
      inst_ad(RC[a]->R(), ~(RC[b]->R()), 1);
      break;
    }
  
  return resGO;
}

int
cl_p1516::exec_inst(void)
{
  t_mem code;
  u8_t inst;
  u8_t cond;
  bool fe;
  
  PC= R[15];
  instPC= PC;
  fe= fetch(&code);
  tick(1);
  R[15]= PC;
  if (fe)
    return(resBREAKPOINT);

  cond= (code & 0xf0000000) >> 28;
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
	return resGO;
    }
  
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
      R[d]= R[a];
      break;
    case 4: // LDL0 Rd,data
      R[d]= code & 0x0000ffff;
      break;
    case 5: // LDL Rd,data
      R[d]= (R[d] & 0xffff0000) | (code & 0x0000ffff);
      break;
    case 6: // LDH Rd,data
      R[d]= (R[d] & 0x0000ffff) | (code << 16);
      break;
    case 7: // ALU
      inst_alu(code);
      break;
    }
  PC= R[15];
  
  return resGO;
}


/* End of p1516.src/p1516.cc */
