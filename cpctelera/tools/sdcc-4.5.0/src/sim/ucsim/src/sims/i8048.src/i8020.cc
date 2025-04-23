/*
 * Simulator of microcontrollers (i8020.cc)
 *
 * Copyright (C) 2022 Drotos Daniel
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

#include <ctype.h>

#include "utils.h"

#include "dregcl.h"

#include "glob.h"

#include "i8020cl.h"

/*
 * Flags
 */

t_mem
cl_flag20_op::write(t_mem val)
{
  val|= 0x18;
  return val;
}


/*
 * CPU
 */

cl_i8020::cl_i8020(class cl_sim *asim):
  cl_uc(asim)
{
  PCmask= 0xfff;
  ram_size= 64;
  rom_size= 1024;
  info_ch= '1';
  timer= NULL;
  inner_rom= rom_size;
  bus= NULL;
}

int
cl_i8020::init(void)
{
  class cl_memory_operator *o;

  o= new cl_pc_op(&cPC);
  o->init();
  cPC.append_operator(o);

  cflagF1.init();
  cflagF1.decode(&flagF1);
  o= new cl_bool_op(&cflagF1);
  o->init();
  cflagF1.append_operator(o);
  cflagF1.set_mask(1);

  cmb.init();
  cmb.decode(&mb);
  o= new cl_bool_op(&cmb);
  o->init();
  cmb.append_operator(o);
  cmb.set_mask(1);

  cl_uc::init();

  mk_cvar(&cflagF1, "F1", "CPU flag F1");
  mk_cvar(&cmb, "DBF", "CPU code bank selector");
  mk_cvar(&cmb, "A11", "CPU code bank selector");
  reg_cell_var(&cA, &rA, "ACC", "Accumulator");

  // prepare tables
  u8_t not_inv[256];
  int i;
  struct dis_entry *dt= dis_tbl();
  for (i= 0; i < 256; i++) not_inv[i]= 0;
  i= 0;
  while (dt[i].mnemonic != NULL)
    {
      if (strchr((const char*)(dt[i].info), info_ch) != NULL)
	{
	  not_inv[dt[i].code]= 1;
	  tick_tab20[dt[i].code]= dt[i].ticks * 5;
	}
      i++;
    }
  for (i= 0; i < 256; i++)
    if (!not_inv[i])
      {
	uc_itab[i]= uc_itab[0x100];
      }
  //reset();
  return 0;
}

/*
const char *
cl_i8020::id_string(void)
{
  return get_name();
}
*/

void
cl_i8020::set_PC(t_addr addr)
{
  // do not change A11
  PC&= ~0x7ff;
  addr&= 0x7ff;
  PC|= addr;
}

t_mem
cl_i8020::fetch(void)
{
  u8_t c= rom->read(PC);
  set_PC((PC+1)&PCmask);
  if (PC >= inner_rom)
    bus->latch(PC);
  vc.fetch++;
  return c;
}

class cl_memory_operator *
cl_i8020::make_flagop(void)
{
  class cl_memory_operator *o;
  o= new cl_flag20_op(cpsw);
  o->init();
  o->set_name("MCS21 flag operator");
  return o;
}

void
cl_i8020::make_cpu_hw(void)
{
  cpu= new cl_i8020_cpu(this);
  add_hw(cpu);
  cpu->init();
}

void
cl_i8020::mk_hw_elements(void)
{
  cl_uc::mk_hw_elements();
  class cl_hw *h;

  timer= new cl_timer(this);
  timer->init();
  add_hw(timer);

  bus= new cl_bus(this);
  bus->init();

  p0= new cl_qport(this, 0, ports, 0, port_8bit);
  p0->init();
  add_hw(p0);
  p1= new cl_qport(this, 1, ports, 1, port_8bit);
  p1->init();
  add_hw(p1);
  p2= new cl_p2(this, 2, ports, 2, port_4bit);
  p2->init();
  add_hw(p2);

  pext= new cl_pext(this, 0, ports, 4, p2);
  pext->init();
  add_hw(pext);

  h= new cl_dreg(this, 0, "dreg");
  h->init();
  add_hw(h);
}

void
cl_i8020::make_memories(void)
{
  class cl_memory_operator *o;
  make_address_spaces();
  // setup psw
  cpsw= (cl_cell8*)aspsw->get_cell(0);
  cpsw->decode(&psw);
  o= make_flagop();
  cpsw->append_operator(o);
  reg_cell_var(cpsw, &psw, "psw", "CPU register PSW");
  // do others
  make_chips();
  decode_regs();
  decode_rom();
  decode_iram();
  // decode ports
  class cl_address_decoder *ad;
  ad= new cl_address_decoder(ports, ports_chip, 0, 7, 0);
  ad->init();
  ad->set_name("def_ports_decoder");
  ports->decoders->add(ad);
  ad->activate(0);
  // decode xram
  ad= new cl_address_decoder(xram, xram_chip, 0, 0xff, 0);
  ad->init();
  ad->set_name("def_xram_decoder");
  xram->decoders->add(ad);
  ad->activate(0);
}

void
cl_i8020::make_address_spaces(void)
{
  rom= new cl_address_space("rom", 0, 0x1000, 8);
  rom->init();
  address_spaces->add(rom);
  
  iram= new cl_address_space("iram", 0, 0x100, 8);
  iram->init();
  address_spaces->add(iram);

  regs= new cl_address_space("regs", 0, 8, 8);
  regs->init();
  address_spaces->add(regs);

  aspsw= new cl_address_space("aspsw", 0, 1, 8);
  aspsw->init();
  //address_spaces->add(aspsw);

  ports= new cl_address_space("ports", 0, 8, 8);
  ports->init();
  address_spaces->add(ports);

  xram= new cl_address_space("xram", 0, 256, 8);
  xram->init();
  address_spaces->add(xram);
}

void
cl_i8020::make_chips(void)
{
  rom_chip= new cl_chip8("rom_chip", rom_size, 8/*, 0xff*/);
  rom_chip->init();
  memchips->add(rom_chip);
  
  iram_chip= new cl_chip8("iram_chip", ram_size, 8);
  iram_chip->init();
  memchips->add(iram_chip);
  
  ports_chip= new cl_chip8("ports_chip", 8, 8);
  ports_chip->init();
  memchips->add(ports_chip);
  
  xram_chip= new cl_chip8("xram_chip", 0x100, 8);
  xram_chip->init();
  memchips->add(xram_chip);
}

void
cl_i8020::decode_rom(void)
{
  class cl_address_decoder *ad;
  ad= new cl_address_decoder(rom, rom_chip, 0, rom_size-1, 0);
  ad->init();
  ad->set_name("def_rom_decoder");
  rom->decoders->add(ad);
  ad->activate(0);
}

void
cl_i8020::decode_regs(void)
{
  class cl_address_decoder *ad;
  ad= new cl_address_decoder(regs, iram_chip, 0, 7, 0);
  ad->init();
  ad->set_name("def_regs_decoder");
  regs->decoders->add(ad);
  ad->activate(0);
  int i;
  for (i= 0; i < 8; i++)
    R[i]= (cl_cell8*)regs->get_cell(i);
}

void
cl_i8020::decode_iram(void)
{
  class cl_address_decoder *ad;
  
  ad= new cl_address_decoder(iram, iram_chip, 0, ram_size-1, 0);
  ad->init();
  ad->set_name("def_iram_decoder");
  iram->decoders->add(ad);
  ad->activate(0);
}

struct dis_entry *
cl_i8020::dis_tbl(void)
{
  return(dis_tab20);
}

struct dis_entry *
cl_i8020::get_dis_entry(t_addr addr)
{
  u8_t code= rom->get(addr);
  return dis_entry_of(code);
}

struct dis_entry *
cl_i8020::dis_entry_of(u8_t code)
{
  for (struct dis_entry *de = dis_tbl(); de && de->mnemonic; de++)
    {
      if ((code & de->mask) == de->code)
	{
	  if (strchr((const char*)(de->info), info_ch) != NULL)
	    return de;
	}
    }
  return NULL;
}

char *
cl_i8020::disassc(t_addr addr, chars *comment)
{
  chars work= chars(), temp= chars(), fmt= chars();
  const char *b;
  struct dis_entry *de;
  int i;
  bool first;
  //u8_t h, l, r;
  u8_t code;
  u16_t a;

  de= get_dis_entry(addr);
  code= rom->read(addr);
  
  if (!de || !de->mnemonic)
    return strdup("-- UNKNOWN/INVALID");

  b= de->mnemonic;

  first= true;
  work= "";
  for (i=0; b[i]; i++)
    {
      if ((b[i] == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (b[i] == '\'')
	{
	  fmt= "";
	  i++;
	  while (b[i] && (b[i]!='\''))
	    fmt.append(b[i++]);
	  if (!b[i]) i--;
	  if (fmt.empty())
	    work.append("'");
	  if (strcmp(fmt.c_str(), "i8") == 0)
	    {
	      work.appendf("#0x%02x", rom->read(addr+1));
	    }
	  if (strcmp(fmt.c_str(), "a8") == 0)
	    {
	      a= addr & 0xf00;
	      a|= rom->read(addr+1);
	      work.appendf("0x%04x", a);
	    }
	  if (strcmp(fmt.c_str(), "a11") == 0)
	    {
	      a= (code&0xe0)<<3;
	      a+= rom->read(addr+1);
	      work.appendf("P.0x%04x", a);
	      if (A11) a|= 0x800;
	      temp.format("; %04x", a);
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	  continue;
	}
      if (b[i] == '%')
	{
	  i++;
	  temp= "";
	  switch (b[i])
	    {
	    }
	  if (comment && temp.nempty())
	    comment->append(temp);
	}
      else
	work+= b[i];
    }

  return(strdup(work.c_str()));
}

int
cl_i8020::inst_length(t_addr addr)
{
  struct dis_entry *de= get_dis_entry(addr);
  return de?(de->length):1;
}

void
cl_i8020::print_regs(class cl_console_base *con)
{
  int start, stop, i;
  //t_mem data;
  u16_t dp;
  // show regs
  start= (psw & flagBS)?24:0;
  con->dd_color("answer");
  con->dd_printf("        R0 R1 R2 R3 R4 R5 R6 R7    PSW= CAF%c-SSS    ACC= ",
		 (type->type & CPU_MCS21)?'-':'B');
  con->dd_color("dump_number");
  con->dd_printf("0x%02x %+3d %c", ACC, ACC, (isprint(ACC)?ACC:'?'));
  con->dd_printf("\n");
  con->dd_cprintf("dump_address", "   0x%02x", start);
  for (int ii= 0; ii < 8; ii++)
    con->dd_cprintf("dump_number", " %02x", iram->get(start + ii));
  con->dd_cprintf("dump_number", "    0x%02x ", psw);
  con->dd_color("dump_number");
  con->print_bin(psw, 8);
  con->dd_printf("    DBF=%d F1=%d", mb, flagF1);
  con->dd_printf("\n");
  // show indirectly addressed IRAM and some basic regs
  start= R[0]->get();
  stop= start+7;
  con->dd_color("answer");
  con->dd_printf("R0=");
  class cl_dump_ads ads(start, stop);
  iram->dump(0, /*start, stop*/&ads, 8, con);
  start= R[1]->get();
  stop= start+7;
  con->dd_color("answer");
  con->dd_printf("R1=");
  ads._ss(start, stop);
  iram->dump(0, /*start, stop*/&ads, 8, con);

  con->dd_cprintf("answer", "SP=%d", psw&7);
  start= 8;
  stop= psw&7;
  for (i= 7; i>=0; i--)
    {
      dp= iram->read(start+2*i+1) * 256 + iram->read(start+2*i);
      con->dd_cprintf("dump_address", " %x", dp>>12);
      if (i<stop)
	con->dd_color("dump_address");
      else
	con->dd_color("dump_number");
      con->dd_printf(".%03x", dp&0xfff);
    }
  con->dd_printf("\n ");
  con->dd_color("answer");
  for (i= 7; i>=0; i--)
    {
      con->dd_printf("    %d", i);
      con->dd_printf("%c", (i==(psw&7))?'^':' ');
	
    }
  con->dd_printf("\n");

  print_disass(PC, con);
}

void
cl_i8020::push(void)
{
  u8_t spb= psw&7;
  u8_t spa= (spb+1)&7;
  u8_t ib= 8 + spb*2;
  iram->write(ib, PC);
  WR;
  u8_t h= (PC>>8) & 0xf;
  h|= (psw & 0xf0);
  iram->write(ib+1, h);
  WR;
  psw&= ~7;
  psw|= spa;
  cF.W(psw);
  stack_write((t_addr)spa);
}

u16_t
cl_i8020::pop(bool popf)
{
  u8_t spb= psw&7;
  u8_t spa= (spb-1)&7;
  u8_t ia= 8 + spa*2, vh;
  u16_t v= iram->read(ia);
  vh= iram->read(ia+1);
  v+= 256 * vh;
  vc.rd+= 2;
  if (popf)
    {
      psw&= 0xf;
      psw|= (vh&0xf0);
    }
  psw&= ~7;
  psw|= spa;
  cF.W(psw);
  return v & 0xfff;
}

void
cl_i8020::stack_check_overflow(t_addr sp_after)
{
  u8_t spa= sp_after, sp_before;
  spa&= 7;
  sp_before= (spa-1)&7;
  
  if (spa < sp_before)
    {
      u8_t ib, ia;
      ib= 8 + sp_before*2;
      ia= 8 + spa*2;
      class cl_error_stack_overflow *e=
	new cl_error_stack_overflow(instPC, ib, ia);
      e->init();
      error(e);
    }
}

void
cl_i8020::reset(void)
{
  cl_uc::reset();
  // 1) PC=0
  cPC.W(0);
  // 2) SP=0, 3) regbank=0
  cpsw->W(0);
  // 4) membank=0
  cmb.W(0);
  // 5) BUS=> HiZ
  // 6) Port1, Port2: input mode
  ports->write(0, 0xff);
  ports->write(1, 0xff);
  ports->write(2, 0xff);
  // 7) disbale irq
  ien= 0;
  // 8) stop timer
  // 9) clear timer flag
  // 10) Clear F0, F1
  cflagF1.W(0);
  // 11) disable clock output from T0
}

int
cl_i8020::exec_inst(void)
{
  return exec_inst_uctab();
}

class cl_memory_cell *
CL2::iram_ir(int regnr)
{
  u8_t a= R[regnr&1]->read();
  return iram->get_cell(a);
}

u8_t
CL2::read_ir(int regnr)
{
  u8_t a= R[regnr&1]->read();
  RD;
  return iram->read(a);
}


cl_i8021::cl_i8021(class cl_sim *asim):
  cl_i8020(asim)
{
  rom_size= 1024;
  ram_size= 64;
  info_ch= '1';
}


cl_i8022::cl_i8022(class cl_sim *asim):
  cl_i8021(asim)
{
  rom_size= 2048;
  ram_size= 128;
  info_ch= '2';
}

cl_i8020_cpu::cl_i8020_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
}

int
cl_i8020_cpu::init(void)
{
  cl_hw::init();

  cl_var *v;
  uc->vars->add(v= new cl_var("T0", cfg, i8020cpu_t0,
			      cfg_help(i8020cpu_t0)));
  v->init();
  uc->vars->add(v= new cl_var("T1", cfg, i8020cpu_t1,
			      cfg_help(i8020cpu_t1)));
  v->init();
  uc->vars->add(v= new cl_var("inner_rom", cfg, i8020cpu_inner,
			      cfg_help(i8020cpu_inner)));
  v->init();

  return 0;
}

const char *
cl_i8020_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case i8020cpu_t0: return "T0 input pin (bool, RW)";
    case i8020cpu_t1: return "T1 input pin (bool, RW)";
    case i8020cpu_inner: return "Size of inner code memory (int, RW)";
    }
  return "Not used";
}

t_mem
cl_i8020_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_i8020 *u= (class cl_i8020 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum i8020cpu_confs)addr)
    {
    case i8020cpu_t0:
      if (val)
	*val= (*val)?1:0;
      break;
    case i8020cpu_t1:
      if (val)
	{
	  *val= (*val)?1:0;
	  if ((cell->get() != 0) &&
	      (*val == 0) &&
	      (u->timer != NULL))
	    {
	      u->timer->do_counter(1);
	    }
	}
      break;
    case i8020cpu_inner:
      if (val)
	u->set_inner(*val);
      cell->set(u->get_inner());
      break;
    case i8020cpu_nuof: break;
    }
  return cell->get();
}


/* End of i8048.src/i8020.cc */
