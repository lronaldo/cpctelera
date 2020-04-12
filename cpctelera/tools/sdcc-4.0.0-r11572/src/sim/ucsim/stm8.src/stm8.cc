/*
 * Simulator of microcontrollers (stm8.cc)
 *
 * some stm8 code base from Karl Bongers karl@turbobit.com
 * and Valentin Dudouyt valentin.dudouyt@gmail.com
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
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

//#include "ddconfig.h"

//#include <stdarg.h> /* for va_list */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// prj
//#include "pobjcl.h"
#include "globals.h"

// sim
//#include "simcl.h"

// local
#include "stm8cl.h"
#include "glob.h"
//#include "regsstm8.h"
#include "stm8mac.h"
//#include "itccl.h"
#include "serialcl.h"
#include "rstcl.h"
#include "timercl.h"
#include "portcl.h"
#include "clkcl.h"
#include "uidcl.h"
#include "bl.h"
#include "flashcl.h"

/*******************************************************************/


/*
 * Base type of STM8 controllers
 */

cl_stm8::cl_stm8(struct cpu_entry *IType, class cl_sim *asim):
  cl_uc(asim)
{
  type= IType;
  flash_ctrl= NULL;
}

int
cl_stm8::init(void)
{
  cl_uc::init(); /* Memories now exist */
  sp_limit= 0x1500;

  xtal = 8000000;

  //rom = address_space(MEM_ROM_ID);
  //ram = mem(MEM_XRAM);
  //ram = rom;

  //printf("******************** leave the RAM dirty now \n");
  // zero out ram(this is assumed in regression tests)
  //for (int i=0x0; i<0x8000; i++) {
  //  ram->set((t_addr) i, 0);
  //}

  trap_src= new cl_it_src(this, -1,
			  (cl_memory_cell*)NULL, (t_mem)0,
			  (cl_memory_cell*)NULL, (t_mem)0,
			  (t_addr)0x8004,
			  false, false,
			  "trap", 0);
  trap_src->init();
  return(0);
}


void
cl_stm8::reset(void)
{
  cl_uc::reset();

  regs.SP = 0x17ff;
  regs.A = 0;
  regs.X = 0;
  regs.Y = 0;
  regs.CC = 0x28;
  //regs.VECTOR = 1;
  PC= 0x8000;
}


char *
cl_stm8::id_string(void)
{
  switch (type->type)
    {
    case CPU_STM8S:
      return((char*)"STM8 S,AF");
    case CPU_STM8L:
      return((char*)"STM8 AL,L");
    case CPU_STM8L101:
      return((char*)"STM8 L101");
    default:
      return((char*)"STM8");
    }
}


/*
 * Making elements of the controller
 */
/*
t_addr
cl_stm8::get_mem_size(enum mem_class type)
{
  switch(type)
    {
    case MEM_ROM: return(0x10000);
    case MEM_XRAM: return(0x10000);
    default: return(0);
    }
 return(cl_uc::get_mem_size(type));
}
*/

 /*
   L15x46 uid: 0x4926 00 5b 00 16 11 47 30 31 38 35 35 36
 */

static class cl_port_ui *d= NULL;
static int puix= 1;
static int puiy= 4;
static int puik= 0;
static int puis= 1;
static const char *puiks= keysets[puik];
static class cl_port_data pd;

void
cl_stm8::mk_port(t_addr base, chars n)
{
  class cl_port *p;
  add_hw(p= new cl_port(this, base, n));
  p->init();

  pd.set_name(n);
  pd.cell_p  = p->cell_p;
  pd.cell_in = p->cell_in;
  pd.cell_dir= p->cell_dir;
  pd.keyset  = chars(puiks);
  pd.basx    = puix;
  pd.basy    = puiy;
  d->add_port(&pd, puis++);
  
  if ((puix+= 20) > 80)
    {
      puix= 1;
      if ((puiy+= 7) > 20)
	;
    }
  if ((puik+= 1) > 7)
    puiks= NULL;
  else
    puiks= keysets[puik];
}

void
cl_stm8::mk_hw_elements(void)
{
  class cl_hw *h;
  cl_uc::mk_hw_elements();
  class cl_option *o;

  if ((o= application->options->get_option("serial1_in_file")) == NULL)
    {
      o= new cl_string_option(this, "serial1_in_file",
			      "Input file for serial line uart1 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  if ((o= application->options->get_option("serial1_out_file")) == NULL)
    {
      o= new cl_string_option(this, "serial1_out_file",
			      "Output file for serial line uart1 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  
  if ((o= application->options->get_option("serial2_in_file")) == NULL)
    {
      o= new cl_string_option(this, "serial2_in_file",
			      "Input file for serial line uart2 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  if ((o= application->options->get_option("serial2_out_file")) == NULL)
    {
      o= new cl_string_option(this, "serial2_out_file",
			      "Output file for serial line uart2 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  
  if ((o= application->options->get_option("serial3_in_file")) == NULL)
    {
      o= new cl_string_option(this, "serial3_in_file",
			      "Input file for serial line uart3 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  if ((o= application->options->get_option("serial3_out_file")) == NULL)
    {
      o= new cl_string_option(this, "serial3_out_file",
			      "Output file for serial line uart3 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  
  if ((o= application->options->get_option("serial4_in_file")) == NULL)
    {
      o= new cl_string_option(this, "serial4_in_file",
			      "Input file for serial line uart4 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  if ((o= application->options->get_option("serial4_out_file")) == NULL)
    {
      o= new cl_string_option(this, "serial4_out_file",
			      "Output file for serial line uart4 (-S)");
      application->options->new_option(o);
      o->init();
      o->hide();
    }
  
  add_hw(d= new cl_port_ui(this, 0, "dport"));
  d->init();
  pd.init();
  
  add_hw(h= new cl_stm8_cpu(this));
  h->init();
  
  if (type->type == CPU_STM8S)
    {
      add_hw(h= new cl_clk_saf(this));
      h->init();
      if (type->subtype & (DEV_STM8S003|
			   DEV_STM8S007|
			   DEV_STM8S103|
			   DEV_STM8S207|
			   DEV_STM8S208|
			   DEV_STM8S903|
			   DEV_STM8AF52))
	{
	  add_hw(h= new cl_serial(this, 0x5230, 1, 17, 18));
	  h->init();
	}
      if (type->subtype & (DEV_STM8S005|
			   DEV_STM8S105|
			   DEV_STM8AF62_46))
	{
	  add_hw(h= new cl_serial(this, 0x5240, 2, 20, 21));
	  h->init();
	}
      if (type->subtype & (DEV_STM8S007|
			   DEV_STM8S207|
			   DEV_STM8S208|
			   DEV_STM8AF52))
	{
	  add_hw(h= new cl_serial(this, 0x5240, 3, 20, 21));
	  h->init();
	}
      if (type->subtype & (DEV_STM8AF62_12))
	{
	  add_hw(h= new cl_serial(this, 0x5230, 4, 17, 18));
	  h->init();
	}
    }
  if (type->type == CPU_STM8L)
    {
      add_hw(h= new cl_clk_all(this));
      h->init();
      add_hw(h= new cl_serial(this, 0x5230, 1, 27, 28));
      h->init();
      if (type->subtype & (DEV_STM8AL3xE|
			   DEV_STM8AL3x8|
			   DEV_STM8L052R|
			   DEV_STM8L15x8|
			   DEV_STM8L162))
	{
	  add_hw(h= new cl_serial(this, 0x53e0, 2, 19, 20));
	  h->init();
	}
      if (type->subtype & (DEV_STM8AL3xE|
			   DEV_STM8AL3x8|
			   DEV_STM8L052R|
			   DEV_STM8L15x8|
			   DEV_STM8L162))
	{
	  add_hw(h= new cl_serial(this, 0x53f0, 3, 21, 22));
	  h->init();
	}
    }
  if (type->type == CPU_STM8L101)
    {
      add_hw(h= new cl_clk_l101(this));
      h->init();
      add_hw(h= new cl_serial(this, 0x5230, 1, 27, 28));
      h->init();
    }

  add_hw(itc= new cl_itc(this));
  itc->init();

  {
    mk_port(0x5000, "pa");
    mk_port(0x5005, "pb");
    mk_port(0x500a, "pc");
    mk_port(0x500f, "pd");
  }
  
  if (type->type == CPU_STM8S)
    {
      // all S and AF
      mk_port(0x5014, "pe");
      mk_port(0x5019, "pf");
      if (type->subtype & (DEV_STM8S005|
			   DEV_STM8S007|
			   DEV_STM8S105|
			   DEV_STM8S207|
			   DEV_STM8S208|
			   DEV_STM8AF52|
			   DEV_STM8AF62_46))
	{
	  mk_port(0x501e, "pg");
	  if (type->subtype != DEV_STM8AF62_46)
	    {
	      mk_port(0x5023, "ph");
	      mk_port(0x5028, "pi");
	    }
	}
      add_hw(h= new cl_rst(this, 0x50b3, 0x1f));
      h->init();
      add_hw(h= new cl_tim1_saf(this, 1, 0x5250));
      h->init();
      // some S, some AF
      if (type->subtype & (DEV_STM8S005|
			   DEV_STM8S007|
			   DEV_STM8S105|
			   DEV_STM8S207|
			   DEV_STM8S208|
			   DEV_STM8AF52|
			   DEV_STM8AF62_46))
	{
	  add_hw(h= new cl_tim2_saf_a(this, 2, 0x5300));
	  h->init();
	  add_hw(h= new cl_tim3_saf(this, 3, 0x5320));
	  h->init();
	  add_hw(h= new cl_tim4_saf_a(this, 4, 0x5340));
	  h->init();
	}
      if (type->subtype & (DEV_STM8S903|
			   DEV_STM8AF62_12))
	{
	  add_hw(h= new cl_tim5_saf(this, 5, 0x5300));
	  h->init();
	  add_hw(h= new cl_tim6_saf(this, 6, 0x5340));
	  h->init();
	}
      if (type->subtype & (DEV_STM8S003|
			   DEV_STM8S103))
	{
	  add_hw(h= new cl_tim2_saf_b(this, 2, 0x5300));
	  h->init();
	  // tim4 B
	  add_hw(h= new cl_tim4_saf_b(this, 4, 0x5340));
	  h->init();
	}
    }
  else if (type->type == CPU_STM8L)
    {
      if (type->subtype != DEV_STM8L051)
	{
	  mk_port(0x5014, "pe");
	  mk_port(0x5019, "pf");
	}
      if (type->subtype & (DEV_STM8AL3xE|
			   DEV_STM8AL3x8|
			   DEV_STM8L052R|
			   DEV_STM8L15x8|
			   DEV_STM8L162))
	{
	  mk_port(0x501e, "pg");
	  if (type->subtype != DEV_STM8L052R)
	    {
	      mk_port(0x5023, "ph");
	      mk_port(0x5028, "pi");
	    }
	}
      add_hw(h= new cl_rst(this, 0x50b0+1, 0x3f));
      h->init();
      add_hw(h= new cl_tim2_all(this, 2, 0x5250));
      h->init();
      add_hw(h= new cl_tim3_all(this, 3, 0x5280));
      h->init();
      add_hw(h= new cl_tim4_all(this, 4, 0x52E0));
      h->init();
      // all AL
      if (type->subtype & DEV_STM8AL)
	{
	  add_hw(h= new cl_tim1_all(this, 1, 0x52b0));
	  h->init();
	}
      // some L
      if (type->subtype & (DEV_STM8L052C |
			   DEV_STM8L052R |
			   DEV_STM8L15x46 |
			   DEV_STM8L15x8 |
			   DEV_STM8L162))
	{
	  add_hw(h= new cl_tim1_all(this, 1, 0x52b0));
	  h->init();
	}
      if (type->subtype & (DEV_STM8AL3xE |
			   DEV_STM8AL3x8 |
			   DEV_STM8L052R |
			   DEV_STM8L15x8 |
			   DEV_STM8L162))
	{
	  add_hw(h= new cl_tim5_all(this, 5, 0x5300));
	  h->init();
	}
    }
  else if (type->type == CPU_STM8L101)
    {
      add_hw(h= new cl_rst(this, 0x50b0+1, 0x0f));
      h->init();
      add_hw(h= new cl_tim2_l101(this, 2, 0x5250));
      h->init();
      add_hw(h= new cl_tim3_l101(this, 2, 0x5280));
      h->init();
      add_hw(h= new cl_tim4_l101(this, 4, 0x52E0));
      h->init();
    }

  // UID
  if (type->subtype & (DEV_STM8S103 |
		       DEV_STM8S903 |
		       DEV_STM8AF62_12))
    {
      add_hw(h= new cl_uid(this, 0x4865));
      h->init();
    }
  else if (type->subtype & (DEV_STM8AL |
			    DEV_STM8L151x23 |
			    DEV_STM8L15x46 |
			    DEV_STM8L15x8 |
			    DEV_STM8L162))
    {
      add_hw(h= new cl_uid(this, 0x4926));
      h->init();
    }
  else if (type->subtype & (DEV_STM8L101))
    {
      add_hw(h= new cl_uid(this, 0x4925));
      h->init();
    }

  // FLASH
  if (type->subtype & (DEV_STM8SAF))
    {
      add_hw(flash_ctrl= new cl_saf_flash(this, 0x505a));
      flash_ctrl->init();
    }
  else if (type->subtype & (DEV_STM8ALL |
			    DEV_STM8L101))
    {
      add_hw(flash_ctrl= new cl_l_flash(this, 0x5050));
      flash_ctrl->init();
    }
  //add_hw(h= new cl_tim235(this, 3, 0x5320));
  //h->init();
  //add_hw(h= new cl_tim46(this, 4, 0x5340));
  //h->init();
}

class cl_memory_chip *c;

void
cl_stm8::make_memories(void)
{
  class cl_address_space *as;

  rom= ram= as= new cl_flash_as/*address_space*/("rom", 0, 0x28000/*, 8*/);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip /* *chip,*/ *rom_chip;

  c= rom_chip= NULL;/*new cl_memory_chip("rom_chip", 0x20000, 8, 0);
  rom_chip->init();
  memchips->add(rom_chip);*/

  ram_chip= new cl_memory_chip("ram_chip", 0x1800, 8);
  ram_chip->init();
  memchips->add(ram_chip);
  eeprom_chip= new cl_memory_chip("eeprom_chip", 0x0800, 8, 0);
  eeprom_chip->init();
  memchips->add(eeprom_chip);
  option_chip= new cl_memory_chip("option_chip", 0x0800, 8, 0);
  option_chip->init();
  memchips->add(option_chip);
  io_chip= new cl_memory_chip("io_chip", 0x0800, 8);
  io_chip->init();
  memchips->add(io_chip);
  if (type->subtype & DEV_STM8S105)
    boot_chip= new cl_memory_chip("boot_chip_s105", bl_s105_length, 8, bl_s105);
  else if (type->subtype & DEV_STM8L15x46)
    boot_chip= new cl_memory_chip("boot_chip_l15x46", bl_l15x46_length, 8, bl_l15x46);
  /*else if (type->subtype & DEV_STM8L101)
    boot_chip= new cl_memory_chip("boot_chip_l101", bl_l15x46_length, 8, bl_l15x46);*/
  else
    boot_chip= new cl_memory_chip("boot_chip", 0x0800, 8);
  boot_chip->init();
  memchips->add(boot_chip);
  cpu_chip= new cl_memory_chip("cpu_chip", 0x0100, 8);
  cpu_chip->init();
  memchips->add(cpu_chip);
  flash_chip= new cl_memory_chip("flash_chip", 0x20000, 8, 0);
  flash_chip->init();
  memchips->add(flash_chip);
  /*
  ad= new cl_address_decoder(as= address_space("rom"), rom_chip, 0, 0x1ffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  */
  ad= new cl_address_decoder(as= rom, ram_chip, 0, 0x17ff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  ad= new cl_address_decoder(as= rom, eeprom_chip, 0x4000, 0x47ff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  ad= new cl_address_decoder(as= rom, option_chip, 0x4800, 0x4fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  ad= new cl_address_decoder(as= rom, io_chip, 0x5000, 0x57ff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  ad= new cl_address_decoder(as= rom, boot_chip, 0x6000, 0x67ff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  rom->set_cell_flag(0x6000, 0x67ff, true, CELL_READ_ONLY);

  ad= new cl_address_decoder(as= rom, cpu_chip, 0x7f00, 0x7fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  ad= new cl_address_decoder(as= rom, flash_chip, 0x8000, 0x27fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
  class cl_option *o= application->options->get_option("writable_flash");
  bool wv= false;
  if (o)
    o->get_value(&wv);
  if (!wv)
    rom->set_cell_flag(0x8000, 0x27fff, true, CELL_READ_ONLY);
  
  regs8= new cl_address_space("regs8", 0, 2, 8);
  regs8->init();
  regs8->get_cell(0)->decode((t_mem*)&regs.A);
  regs8->get_cell(1)->decode((t_mem*)&regs.CC);

  regs16= new cl_address_space("regs16", 0, 3, 16);
  regs16->init();

  regs16->get_cell(0)->decode((t_mem*)&regs.X);
  regs16->get_cell(1)->decode((t_mem*)&regs.Y);
  regs16->get_cell(2)->decode((t_mem*)&regs.SP);

  address_spaces->add(regs8);
  address_spaces->add(regs16);

  class cl_var *v;
  vars->add(v= new cl_var(cchars("A"), regs8, 0, ""));
  v->init();
  vars->add(v= new cl_var(cchars("CC"), regs8, 1, ""));
  v->init();
  
  vars->add(v= new cl_var(cchars("X"), regs16, 0, ""));
  v->init();
  vars->add(v= new cl_var(cchars("Y"), regs16, 1, ""));
  v->init();
  vars->add(v= new cl_var(cchars("SP"), regs16, 2, ""));
  v->init();
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_stm8::dis_tbl(void)
{
  return(disass_stm8);
}

/*struct name_entry *
cl_stm8::sfr_tbl(void)
{
  return(0);
}*/

/*struct name_entry *
cl_stm8::bit_tbl(void)
{
  //FIXME
  return(0);
}*/

int
cl_stm8::inst_length(t_addr addr)
{
  int len = 0;

  get_disasm_info(addr, &len, NULL, NULL, NULL);

  return len;
}
int
cl_stm8::inst_branch(t_addr addr)
{
  int b;

  get_disasm_info(addr, NULL, &b, NULL, NULL);

  return b;
}

bool
cl_stm8::is_call(t_addr addr)
{
  struct dis_entry *e;

  get_disasm_info(addr, NULL, NULL, NULL, &e);

  return e?(e->is_call):false;
}

int
cl_stm8::longest_inst(void)
{
  return 5;
}



const char *
cl_stm8::get_disasm_info(t_addr addr,
			 int *ret_len,
			 int *ret_branch,
			 int *immed_offset,
			 struct dis_entry **dentry)
{
  const char *b = NULL;
  uint code;
  int len = 0;
  int immed_n = 0;
  int i;
  int start_addr = addr;
  struct dis_entry *dis_e;

  code= rom->get(addr++);
  dis_e = NULL;

  switch(code) {
	/* here will be all the prefixes for STM8 */
	case 0x72 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_stm8_72[i].mask) != disass_stm8_72[i].code &&
        disass_stm8_72[i].mnemonic)
        i++;
      dis_e = &disass_stm8_72[i];
      b= disass_stm8_72[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_72[i].length + 1);
    break;

    	case 0x71 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_stm8_71[i].mask) != disass_stm8_71[i].code &&
        disass_stm8_71[i].mnemonic)
        i++;
      dis_e = &disass_stm8_71[i];
      b= disass_stm8_71[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_71[i].length + 1);
    break;

	case 0x90 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_stm8_90[i].mask) != disass_stm8_90[i].code &&
        disass_stm8_90[i].mnemonic)
        i++;
      dis_e = &disass_stm8_90[i];
      b= disass_stm8_90[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_90[i].length + 1);
    break;
	  
	case 0x91 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_stm8_91[i].mask) != disass_stm8_91[i].code &&
        disass_stm8_91[i].mnemonic)
        i++;
      dis_e = &disass_stm8_91[i];
      b= disass_stm8_91[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_91[i].length + 1);
    break;
	  
	case 0x92 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_stm8_92[i].mask) != disass_stm8_92[i].code &&
        disass_stm8_92[i].mnemonic)
        i++;
      dis_e = &disass_stm8_92[i];
      b= disass_stm8_92[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8_92[i].length + 1);
    break;
	  
	
    default:
      i= 0;
      while ((code & disass_stm8[i].mask) != disass_stm8[i].code &&
             disass_stm8[i].mnemonic)
        i++;
      dis_e = &disass_stm8[i];
      b= disass_stm8[i].mnemonic;
      if (b != NULL)
        len += (disass_stm8[i].length);
    break;
  }

  if (ret_branch) {
    *ret_branch = dis_e->branch;
  }

  if (immed_offset) {
    if (immed_n > 0)
         *immed_offset = immed_n;
    else *immed_offset = (addr - start_addr);
  }

  if (len == 0)
    len = 1;

  if (ret_len)
    *ret_len = len;

  if (dentry)
    *dentry= dis_e;
  
  return b;
}

char *
cl_stm8::disass(t_addr addr, const char *sep)
{
  char work[256], temp[20];
  const char *b;
  char *buf, *p, *t;
  int len = 0;
  int immed_offset = 0;


  p= work;

  b = get_disasm_info(addr, &len, NULL, &immed_offset, NULL);

  if (b == NULL) {
    buf= (char*)malloc(30);
    strcpy(buf, "UNKNOWN/INVALID");
    return(buf);
  }

  while (*b)
    {
      if (*b == '%')
        {
          b++;
          switch (*(b++))
            {
            case 's': // s    signed byte immediate
              sprintf(temp, "#%d", (char)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case 'e': // e    extended 24bit immediate operand
              sprintf(temp, "#0x%06lx",
                 (ulong)((rom->get(addr+immed_offset)<<16) |
                        (rom->get(addr+immed_offset+1)<<8) |
                        (rom->get(addr+immed_offset+2))) );
              ++immed_offset;
              ++immed_offset;
              ++immed_offset;
              break;
            case 'w': // w    word immediate operand
              sprintf(temp, "#0x%04x",
                 (uint)((rom->get(addr+immed_offset)<<8) |
                        (rom->get(addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'b': // b    byte immediate operand
              sprintf(temp, "#0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case 'x': // x    extended addressing
              sprintf(temp, "0x%04x",
                 (uint)((rom->get(addr+immed_offset)<<8) |
                        (rom->get(addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'd': // d    direct addressing
              sprintf(temp, "0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case '3': // 3    24bit index offset
              sprintf(temp, "0x%06lx",
                 (ulong)((rom->get(addr+immed_offset)<<16) |
                        (rom->get(addr+immed_offset+1)<<8) |
                        (rom->get(addr+immed_offset+2))) );
              ++immed_offset;
              ++immed_offset;
              ++immed_offset;
             break;
            case '2': // 2    word index offset
              sprintf(temp, "0x%04x",
                 (uint)((rom->get(addr+immed_offset)<<8) |
                        (rom->get(addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case '1': // b    byte index offset
              sprintf(temp, "0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case 'p': // b    byte index offset
	      {
		long int base;
		i8_t offs;
		base= addr+immed_offset+1;
		offs= rom->get(addr+immed_offset);
		long int res= base+offs;
		sprintf(temp, "0x%04lx",
			/*(long int)(addr+immed_offset+1
			  +(int)rom->get(addr+immed_offset))*/
			res
			);
		++immed_offset;
	      }
              break;
            default:
              strcpy(temp, "?");
              break;
            }
          t= temp;
          while (*t)
            *(p++)= *(t++);
        }
      else
        *(p++)= *(b++);
    }
  *p= '\0';

  p= strchr(work, ' ');
  if (!p)
    {
      buf= strdup(work);
      return(buf);
    }
  if (sep == NULL)
    buf= (char *)malloc(6+strlen(p)+1);
  else
    buf= (char *)malloc((p-work)+strlen(sep)+strlen(p)+1);
  for (p= work, t= buf; *p != ' '; p++, t++)
    *t= *p;
  p++;
  *t= '\0';
  if (sep == NULL)
    {
      while (strlen(buf) < 6)
        strcat(buf, " ");
    }
  else
    strcat(buf, sep);
  strcat(buf, p);
  return(buf);
}


void
cl_stm8::print_regs(class cl_console_base *con)
{
  con->dd_printf("V-IHINZC  Flags= 0x%02x %3d %c  ",
                 regs.CC, regs.CC, isprint(regs.CC)?regs.CC:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 regs.A, regs.A, isprint(regs.A)?regs.A:'.');
  con->dd_printf("%c-%c%c%c%c%c%c  ",
                 (regs.CC&BIT_V)?'1':'0',
                 (regs.CC&BIT_I1)?'1':'0',
                 (regs.CC&BIT_H)?'1':'0',
                 (regs.CC&BIT_I0)?'1':'0',
                 (regs.CC&BIT_N)?'1':'0',
                 (regs.CC&BIT_Z)?'1':'0',
                 (regs.CC&BIT_C)?'1':'0');
  con->dd_printf("X= 0x%04x %3d %c    ",
                 regs.X, regs.X, isprint(regs.X)?regs.X:'.');
  con->dd_printf("Y= 0x%04x %3d %c\n",
                 regs.Y, regs.Y, isprint(regs.Y)?regs.Y:'.');
  con->dd_printf("SP= 0x%04x [SP+1]= %02x %3d %c  Limit= 0x%04x\n",
                 regs.SP, ram->get(regs.SP+1), ram->get(regs.SP+1),
                 isprint(ram->get(regs.SP+1))?ram->get(regs.SP+1):'.',
		 AU(sp_limit));

  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_stm8::exec_inst(void)
{
  t_mem code;
  unsigned char cprefix; // prefix used for processing in functions
  /*
  if (regs.VECTOR) {
    PC = get1(0x8000);
	if (PC == 0x82) { // this is reserved opcode for vector table
		regs.VECTOR = 0;
		PC = get1(0x8001)*(1<<16);
		PC += get2(0x8002);
		return(resGO);
	} else {
		return( resERROR);
	}
  }
  */

  instPC= PC;

  if (fetch(&code)) {
    //printf("******************** break \n");
	  return(resBREAKPOINT);
  }
  tick(1);

  switch (code)
    { // get prefix
    case 0x72:
    case 0x90:
    case 0x91:
    case 0x92:
      cprefix = code;
      fetch(&code);
      break;
    case 0x82:
      {
	int ce= fetch();
	int ch= fetch();
	int cl= fetch();
	PC= ce*0x10000 + ch*0x100 + cl;
	return resGO;
      }
    case 0x8b: return resSTOP; // BREAK instruction
    default:
      cprefix = 0x00;
      break;
    }

   // exceptions
   if((cprefix==0x90)&&((code&0xf0)==0x10)) {
      return ( inst_bccmbcpl( code, cprefix));
   }
   if((cprefix==0x72)&&((code&0xf0)==0x10)) {
      return ( inst_bresbset( code, cprefix));
   }
   if((cprefix==0x72)&&((code&0xf0)==0x00)) {
      return ( inst_btjfbtjt( code, cprefix));
   }
   if ((code &0xf0) == 0x20) {
      return ( inst_jr( code, cprefix));
   }
   if (cprefix == 0x72) {
      switch (code) {
      	 // addw
     case 0xa9:
	 case 0xb9:
	 case 0xbb:
	 case 0xf9:
	 case 0xfb:
            return( inst_addw( code, cprefix));
	 // subw
	 case 0xa2:
	 case 0xb2:
	 case 0xb0:
	 case 0xf2:
	 case 0xf0:
            return( inst_addw( code, cprefix));
			//default is processing in the next switch statement
         default:
            break;
      }
   }

   // main switch
   switch (code & 0xf) {
   unsigned int tempi;
   int opaddr;
      case 0x0:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // neg
               return( inst_neg( code, cprefix));
               break;
            case 0x80: // IRET
               pop1( regs.CC);
               pop1( regs.A);
               pop2( regs.X);
               pop2( regs.Y);
               pop1( tempi);
               pop2( PC);
               PC += (tempi <<16); //Add PCE to PC
	       {
		 class it_level *il= (class it_level *)(it_levels->top());
		 if (il &&
		     il->level >= 0)
		   {
		     il= (class it_level *)(it_levels->pop());
		     delete il;
		   }
	       }
               return(resGO);
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // SUB
               return( inst_sub( code, cprefix));
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x1:
         switch ( code & 0xf0) {
            case 0x00: // RRWA
               if (cprefix == 0x00) { // rrwa X,A
                  tempi = regs.X;
                  regs.X >>= 8;
                  regs.X |= (regs.A << 8);
                  regs.A = tempi & 0xff;
                  FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
                  FLAG_ASSIGN (BIT_Z, regs.X == 0x0000);
               } else if (cprefix == 0x90) { // rrwa Y,A
                  tempi = regs.Y;
                  regs.Y >>= 8;
                  regs.Y |= (regs.A << 8);
                  regs.A = tempi & 0xff;
                  FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
                  FLAG_ASSIGN (BIT_Z, regs.Y == 0x0000);
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x30: // exg A,longmem
               opaddr = fetch2();
               tempi = get1(opaddr);
               store1( opaddr, regs.A);
               regs.A = tempi;
               return(resGO);
            case 0x40: // exg A,XL
               tempi = regs.X;
               regs.X = (regs.X &0xff00) | regs.A;
               regs.A = tempi & 0xff;
               return(resGO);
            case 0x50: // exgw X,Y
               tempi = regs.Y;
               regs.Y = regs.X;
               regs.X = tempi;
               return(resGO);
            case 0x60: // exg A,YL
               tempi = regs.Y;
               regs.Y = (regs.Y &0xff00) | regs.A;
               regs.A = tempi & 0xff;
               return(resGO);
            case 0x70: // special opcodes
               code = fetch();
               switch(code) {
                  case 0xEC: return(resHALT);
                  case 0xED: putchar(regs.A); fflush(stdout); return(resGO);
                  default:
		    //printf("************* bad code !!!!\n");
                     return(resINV_INST);
               }
            case 0x80: // ret
               pop2( PC);
               return(resGO);
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // CP
               return( inst_cp( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x2:
         switch ( code & 0xf0) {
            case 0x00: // RLWA
               if (cprefix == 0x00) { // rlwa X,A
                  tempi = regs.X;
                  regs.X <<= 8;
                  regs.X |= regs.A ;
                  regs.A = tempi >> 8;
                  FLAG_ASSIGN (BIT_N, 0x8000 & regs.X);
                  FLAG_ASSIGN (BIT_Z, regs.X == 0x0000);
               } else if (cprefix == 0x90) { // rlwa Y,A
                  tempi = regs.Y;
                  regs.Y <<= 8;
                  regs.Y |= regs.A ;
                  regs.A = tempi >> 8;
                  FLAG_ASSIGN (BIT_N, 0x8000 & regs.Y);
                  FLAG_ASSIGN (BIT_Z, regs.Y == 0x0000);
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x30: // POP longmem
               opaddr = fetch2();
               pop1(tempi);
               store1(opaddr, tempi);
               return(resGO);
            case 0x40: // mul
               tick(3);
               if(cprefix==0x90) {
                  regs.Y = (regs.Y&0xff) * regs.A;
               } else if(cprefix==0x00) {
                  regs.X = (regs.X&0xff) * regs.A;
               } else {
                  return(resHALT);
               }
               FLAG_CLEAR(BIT_H);
               FLAG_CLEAR(BIT_C);
               return(resGO);
               break;
            case 0x50: // sub sp,#val
               regs.SP -= fetch();
               return(resGO);
               break;            
            case 0x60: //div
               return(inst_div(code, cprefix));
               break;
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // SBC
               return( inst_sbc( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x3:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // CPL, CPLW
               return( inst_cpl( code, cprefix));
               break;
            case 0x80: // TRAP
	       {
		 class it_level *il= new it_level(3, 0x8004, PC, trap_src);
		 accept_it(il);
	       }
               return(/*resHALT*/resGO);
            case 0x90:
               if(cprefix==0x90) {
                  regs.Y = regs.X;
               } else if(cprefix==0x00) {
                  regs.X = regs.Y;
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // CPW
               return( inst_cpw( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x4:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // SRL
               return( inst_srl( code, cprefix));
               break;
            case 0x80: 
               pop1( regs.A);
               return(resGO);
            case 0x90:
               if(cprefix==0x90) {
                  regs.SP = regs.Y;
               } else if(cprefix==0x00) {
                  regs.SP = regs.X;
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // AND
               return( inst_and( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x5:
         switch ( code & 0xf0) {
            case 0x30: // MOV
               tempi = fetch1();
               opaddr = fetch2();
               store1(opaddr, tempi);
               return( resGO);
               break;
            case 0x40:
               tempi = get1(fetch1());
               opaddr = fetch1();
               store1(opaddr, tempi);
               return( resGO);
               break;
            case 0x50:
               tempi = get1(fetch2());
               opaddr = fetch2();
               store1(opaddr, tempi);
               return( resGO);
               break;
            case 0x60: // DIVW
               return( inst_div( code, cprefix));
               break;
            case 0x80:
               if(cprefix==0x90) {
                  pop2(regs.Y);
               } else if(cprefix==0x00) {
                  pop2(regs.X);
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x90:
               if(cprefix==0x90) {
                  regs.Y = (regs.Y & 0xff) | (regs.A<<8);
               } else if(cprefix==0x00) {
                  regs.X = (regs.X & 0xff) | (regs.A<<8);
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // BCP
               return( inst_bcp( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x6:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // RRC
               return( inst_rrc( code, cprefix));
               break;
            case 0x10:
               return(inst_ldxy( code, cprefix));
               break;
            case 0x80: 
               pop1( regs.CC);
               return(resGO);
            case 0x90:
               if(cprefix==0x90) {
                  regs.Y = regs.SP;
               } else if(cprefix==0x00) {
                  regs.X = regs.SP;
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // LD A,...
               return( inst_lda( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x7:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // SRA
               return( inst_sra( code, cprefix));
               break;
            case 0x10:
               opaddr = fetch1()+regs.SP;
               store2(opaddr, regs.Y);
               FLAG_ASSIGN (BIT_Z, (regs.Y & 0xffff) == 0x0000);
               FLAG_ASSIGN (BIT_N, regs.Y & 0x8000);
               return(resGO);
               break;
            case 0x80: // RETF
               pop1( tempi);
               pop2( PC);
               PC += (tempi <<16); //Add PCE to PC
               return(resGO);
            case 0x90:
               if(cprefix==0x90) {
                  regs.Y = (regs.Y & 0xff00) | regs.A;
               } else if(cprefix==0x00) {
                  regs.X = (regs.X & 0xff00) | regs.A;
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0xA0:
               opaddr = fetch2();
               if (cprefix == 0x92) {
                  store1(get3(opaddr)+regs.X,regs.A);
               } else if(cprefix==0x91) {
                  store1(get3(opaddr)+regs.Y,regs.A);
               } else if(cprefix==0x90) {
                  store1((opaddr << 8) + fetch() + regs.Y, regs.A);
               } else if(cprefix==0x00) {
                  store1((opaddr << 8) + fetch() + regs.X, regs.A);
               } else {
                  return(resHALT);
               }
               FLAG_NZ (regs.A);
               return(resGO);
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // LD dst,A
               return( inst_lddst( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x8:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // SLL
               return( inst_sll( code, cprefix));
               break;
            case 0x80: 
               push1( regs.A);
               return(resGO);
            case 0x90: // RCF
               FLAG_CLEAR(BIT_C);
               return(resGO);
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // XOR
               return( inst_xor( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0x9:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // RLC
               return( inst_rlc( code, cprefix));
               break;
            case 0x80: // PUSHW
               if(cprefix==0x90) {
                  push2(regs.Y);
               } else if(cprefix==0x00) {
                  push2(regs.X);
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x90: // SCF
               FLAG_SET(BIT_C);
               return(resGO);
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // ADC
               return( inst_adc( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0xa:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // DEC
               return( inst_dec( code, cprefix));
               break;
            case 0x80: 
               push1( regs.CC);
               return(resGO);
            case 0x90: // RIM
               FLAG_CLEAR(BIT_I0);
               FLAG_SET(BIT_I1);
               return(resGO);
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // OR
               return( inst_or( code, cprefix));
               break;
             default:
               //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0xb:
         switch ( code & 0xf0) {
            case 0x30: // push longmem
	      {
		t_addr a= fetch2();
		t_mem v= get1(a);
		push1( v /*get1(fetch2())*/);
		return(resGO);
	      }
            case 0x40: // push #byte
	      {
		t_mem v= fetch1();
		push1(v);
		return(resGO);
	      }
            case 0x50: // addw sp,#val
               regs.SP += fetch1();
               return(resGO);
               break;
            case 0x60: // ld (shortoff,SP),A
               store1(fetch1()+regs.SP, regs.A);
               FLAG_NZ(regs.A);
               return(resGO);
               break;
            case 0x70: // ld A,(shortoff,SP)
               regs.A = get1(fetch1()+regs.SP);
               FLAG_NZ(regs.A);
               return(resGO);
               break;
            case 0x80: // BREAK
               return(resSTOP);
            case 0x90: // SIM - disable INT
               FLAG_SET(BIT_I0);
               FLAG_SET(BIT_I1);
               return(resGO);
            case 0x10:
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // ADD
               return( inst_add( code, cprefix));
               break;
            default:
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0xc:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // INC
               return( inst_inc( code, cprefix));
               break;
            case 0x10: // ADDW X,#word
               return( inst_addw( code, cprefix));
               break;
            case 0x80: // CCF
               regs.CC ^= BIT_C;
               return(resGO);
               break;            
            case 0x90: // RVF
               FLAG_CLEAR(BIT_V);
               return(resGO);
            case 0xA0: // JPF
               opaddr = fetch2();
               if (cprefix == 0x92) {
                  PC = get3(opaddr);
               } else {
                  PC = (opaddr << 8) + fetch();
               }
               return(resGO);
               break;
            case 0xb0: // LDF
               opaddr = fetch2();
               if (cprefix == 0x92) {
                  regs.A = get1(get3(opaddr));
               } else {
                  regs.A = get1((opaddr << 8) + fetch());
               }
               FLAG_NZ (regs.A);
               return(resGO);
               break;
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // JP
               return( inst_jp( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0xd:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // tnz
               return( inst_tnz( code, cprefix));
               break;
            case 0x10: // SUBW X,#word
               return( inst_addw( code, cprefix));
               break;
            case 0x80: // CALLF
               opaddr = fetch2();
               if (cprefix == 0x92) {
                   push2(PC & 0xffff);
                   push1(PC >> 16);
                   PC = get3(opaddr);
               } else {
                   unsigned char c = fetch();
                   push2(PC & 0xffff);
                   push1(PC >> 16);
                   PC = (opaddr << 8) + c;
               }
               return(resGO);
               break;
            case 0x90: // NOP
               return(resGO);
               break;
            case 0xA0: // CALLR
             {
               signed char c = (signed char) fetch1();
               push2(PC);
               PC += c;
               return(resGO);
             }
               break;            
            case 0xb0: // LDF
               opaddr = fetch2();
               if (cprefix == 0x92) {
                  store1(get3(opaddr),regs.A);
               } else {
                  store1((opaddr << 8) + fetch(), regs.A);
               }
               FLAG_NZ (regs.A);
               return(resGO);
               break;
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // CALL
               return( inst_call( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0xe:
         switch ( code & 0xf0) {
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // swap
               return( inst_swap( code, cprefix));
               break;
            case 0x80: 
	      //printf("************* HALT instruction reached !!!!\n");
               return(resHALT);
            case 0x90: // LD A, YH / XH
               if(cprefix==0x90) {
                  regs.A = (regs.Y >> 8) & 0xff;
               } else if(cprefix==0x00) {
                  regs.A = (regs.X >> 8) & 0xff;
               } else {
                  return(resHALT);
               }
               return(resGO);
               break;
            case 0x10: 
            case 0xA0:
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // LDXY
               return( inst_ldxy( code, cprefix));
               break;
            default:
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      case 0xf:
         switch ( code & 0xf0) {
            case 0x10:
               // ldw   (offset,SP),X
               return( inst_ldxydst( code, cprefix ) );
               break;
            case 0x00: 
            case 0x30:
            case 0x40:
            case 0x50:
            case 0x60:
            case 0x70: // CLR
               return( inst_clr( code, cprefix));
               break;
            case 0x80: 
	      //printf("************* WFI/WFE instruction not implemented !!!!\n");
               return(resINV_INST);
            case 0x90:
               if(cprefix==0x90) {
                  regs.A = (regs.Y & 0xff);
               } else if(cprefix==0x00) {
                  regs.A = (regs.X & 0xff);
               } else {
                  return(resHALT);
               }
               return(resGO);
            case 0xA0: // LDF
               opaddr = fetch2();
               if (cprefix == 0x92) {
                  regs.A = get1(get3(opaddr)+regs.X);
               } else if(cprefix==0x91) {
                  regs.A = get1(get3(opaddr)+regs.Y);
               } else if(cprefix==0x90) {
                  regs.A = get1((opaddr << 8) + fetch() + regs.Y);
               } else if(cprefix==0x00) {
                  regs.A = get1((opaddr << 8) + fetch() + regs.X);
               } else {
                  return(resHALT);
               }
               FLAG_NZ (regs.A);
               return(resGO);
            case 0xB0:
            case 0xC0:
            case 0xD0:
            case 0xE0:
            case 0xF0: // LDXYDST
               return( inst_ldxydst( code, cprefix));
               break;
            default: 
	      //printf("************* bad code !!!!\n");
               return(resINV_INST);
         }
         break;
      default:
	//printf("************* bad code !!!!\n");
         return(resINV_INST);
      
   }

   //printf("************* bad code !!!!\n");
  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  //PC= rom->inc_address(PC, -1);

  //sim->stop(resINV_INST);
  return(resINV_INST);
}


/*
 * Checking for interrupt requests and accept one if needed
 */

int
cl_stm8::do_interrupt(void)
{
  return cl_uc::do_interrupt();
}

int
cl_stm8::priority_of(uchar nuof_it)
{
  t_addr ra= 0x7f70;
  int idx= nuof_it / 4;
  u8_t i1_mask, i0_mask, i0, i1;
  cl_memory_cell *c;
  t_mem cv;
  int levels[4]= { 2, 1, 0, 3 };

  if (nuof_it > 31)
    return 0;
  i1_mask= 2 << ((nuof_it % 4) * 2);
  i0_mask= 1 << ((nuof_it % 4) * 2);
  c= ram->get_cell(ra + idx);
  cv= c->read();
  i0= (cv & i0_mask)?1:0;
  i1= (cv & i1_mask)?2:0;
  return levels[i1+i0];
}

int
cl_stm8::priority_main(void)
{
  t_mem cv= regs.CC;
  u8_t i1, i0;
  int levels[4]= { 2, 1, 0, 3 };
  i0= (cv & BIT_I0)?1:0;
  i1= (cv & BIT_I1)?2:0;
  return levels[i1+i0];
}


/*
 * Accept an interrupt
 */

int
cl_stm8::accept_it(class it_level *il)
{
  //class cl_it_src *is= il->source;
  push2( PC & 0xffff);
  push1( PC >> 16); //extended PC
  push2( regs.Y);
  push2( regs.X);
  push1( regs.A);
  push1( regs.CC);
  // set I1 and I0 status bits
  if (il->level == 0)
    { FLAG_SET(BIT_I1) FLAG_CLEAR(BIT_I0) }
  else if (il->level == 1)
    { FLAG_CLEAR(BIT_I1) FLAG_SET(BIT_I0) }
  else if (il->level == 2)
    { FLAG_CLEAR(BIT_I1) FLAG_CLEAR(BIT_I0) }
  else // 3
    { FLAG_SET(BIT_I1) FLAG_SET(BIT_I0) }
  PC = il->addr;

  it_levels->push(il);
  return resGO;//resINTERRUPT;
}


/* check if interrupts are enabled (globally)
 */

bool
cl_stm8::it_enabled(void)
{
  return !(regs.CC & BIT_I0) || !(regs.CC & BIT_I1);
}

void
cl_stm8::stack_check_overflow(class cl_stack_op *op)
{
  if (op)
    {
      if (op->get_op() & stack_write_operation)
	{
	  t_addr a= op->get_after();
	  if (a < sp_limit)
	    {
	      class cl_error_stack_overflow *e=
		new cl_error_stack_overflow(op);
	      e->init();
	      error(e);
	    }
	}
    }
}

cl_stm8_cpu::cl_stm8_cpu(class cl_uc *auc):
  cl_hw(auc, HW_DUMMY, 0, "cpu")
{
}

int
cl_stm8_cpu::init(void)
{
  int i;
  cl_hw::init();
  for (i= 0; i < 11; i++)
    {
      regs[i]= register_cell(uc->rom, 0x7f00+i);
    }
  cl_var *v;
  uc->vars->add(v= new cl_var(chars("sp_limit"), cfg, cpuconf_sp_limit,
			      cfg_help(cpuconf_sp_limit)));
  v->init();
  
  return 0;
}

void
cl_stm8_cpu::write(class cl_memory_cell *cell, t_mem *val)
{
  t_addr a;
  cl_stm8 *u= (cl_stm8*)uc;

  if (conf(cell, val))
    return;
  
  if (conf(cell, NULL))
    return;

  *val&= 0xff;
  if (!uc->rom->is_owned(cell, &a))
    return;  
  if ((a < 0x7f00) ||
      (a > 0x7f0a))
    return;

  a-= 0x7f00;
  switch (a)
    {
    case 0:
      u->regs.A= *val;
      break;
    case 1:
      u->PC= (u->PC & 0xffff) + (*val << 16);
      break;
    case 2:
      u->PC= (u->PC & 0xff00ff) | (*val << 8);
      break;
    case 3:
      u->PC= (u->PC & 0xffff00) | (*val);
      break;
    case 4:
      u->regs.X= (u->regs.X & 0xff) | (*val << 8);
      break;
    case 5:
      u->regs.X= (u->regs.X & 0xff00) | (*val);
      break;
    case 6:
      u->regs.Y= (u->regs.Y & 0xff) | (*val << 8);
      break;
    case 7:
      u->regs.Y= (u->regs.Y & 0xff00) | (*val);
      break;
    case 8:
      u->regs.SP= (u->regs.SP & 0xff) | (*val << 8);
      break;
    case 9:
      u->regs.SP= (u->regs.SP & 0xff00) | (*val);
      break;
    case 0xa:
      u->regs.CC= (u->regs.CC & 0xff00) | (*val);
      break;
    }
}

t_mem
cl_stm8_cpu::read(class cl_memory_cell *cell)
{
  t_mem v= cell->get();
  t_addr a;
  cl_stm8 *u= (cl_stm8*)uc;
  
  if (conf(cell, NULL))
    return v;
  if (!uc->rom->is_owned(cell, &a))
    return v;
  if ((a < 0x7f00) ||
      (a > 0x7f0a))
    return v;

  a-= 0x7f00;
  switch (a)
    {
    case 0:
      v= u->regs.A;
      break;
    case 1:
      v= (u->PC >> 16) & 0xff;
      break;
    case 2:
      v= (u->PC >> 8) & 0xff;
      break;
    case 3:
      v= u->PC & 0xff;
      break;
    case 4:
      v= (u->regs.X >> 8) & 0xff;
      break;
    case 5:
      v= u->regs.X & 0xff;
      break;
    case 6:
      v= (u->regs.Y >> 8) & 0xff;
      break;
    case 7:
      v= u->regs.Y & 0xff;
      break;
    case 8:
      v= (u->regs.SP >> 8) & 0xff;
      break;
    case 9:
      v= u->regs.SP & 0xff;
      break;
    case 0xa:
      v= u->regs.CC;
      break;
    }
  return v;
}

t_mem
cl_stm8_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_stm8 *u= (class cl_stm8 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum stm8_cpu_cfg)addr)
    {
    case cpuconf_sp_limit:
      if (val)
	u->sp_limit= *val & 0xffff;
      else
	cell->set(u->sp_limit);
      break;
    }
  return cell->get();
}

char *
cl_stm8_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case cpuconf_sp_limit:
      return (char*)"Stack overflows when SP is below this limit";
    }
  return (char*)"Not used";
}

/* End of stm8.src/stm8.cc */
