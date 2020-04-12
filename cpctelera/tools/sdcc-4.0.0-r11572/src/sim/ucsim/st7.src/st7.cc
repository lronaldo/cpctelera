/*
 * Simulator of microcontrollers (st7.cc)
 *
 * some st7 code base from Karl Bongers karl@turbobit.com
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

// sim
#include "simcl.h"

// local
#include "st7cl.h"
#include "glob.h"
//#include "regsst7.h"
#include "st7mac.h"

#define uint32 t_addr
#define uint8 unsigned char

/*******************************************************************/


/*
 * Base type of ST7 controllers
 */

cl_st7::cl_st7(class cl_sim *asim):
  cl_uc(asim)
{
  type= (struct cpu_entry *)malloc(sizeof(struct cpu_entry));
  type->type= CPU_ST7;
}

int
cl_st7::init(void)
{
  cl_uc::init(); /* Memories now exist */

  xtal = 8000000;

  //rom = address_space(MEM_ROM_ID);
//  ram = mem(MEM_XRAM);
  //ram = rom;

  // zero out ram(this is assumed in regression tests)
  //printf("******************** leave the RAM dirty now \n");
//  for (int i=0x0; i<0x8000; i++) {
//    ram->set((t_addr) i, 0);
//  }

  return(0);
}


void
cl_st7::reset(void)
{
  cl_uc::reset();

  regs.SP = 0x17f;
  regs.A = 0;
  regs.X = 0;
  regs.Y = 0;
  regs.CC = 0x00;
  regs.VECTOR = 1;

}


char *
cl_st7::id_string(void)
{
  return((char*)"unspecified ST7");
}


/*
 * Making elements of the controller
 */
/*
t_addr
cl_st7::get_mem_size(enum mem_class type)
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
void
cl_st7::mk_hw_elements(void)
{
  //class cl_base *o;
  class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_st7_cpu(this));
  h->init();
}

void
cl_st7::make_memories(void)
{
  class cl_address_space *as;

  rom= ram= as= new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_memory_chip("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

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
cl_st7::dis_tbl(void)
{
  return(disass_st7);
}

/*struct name_entry *
cl_st7::sfr_tbl(void)
{
  return(0);
}*/

/*struct name_entry *
cl_st7::bit_tbl(void)
{
  //FIXME
  return(0);
}*/

int
cl_st7::inst_length(t_addr addr)
{
  int len = 0;

  get_disasm_info(addr, &len, NULL, NULL, NULL);

  return len;
}
int
cl_st7::inst_branch(t_addr addr)
{
  int b;

  get_disasm_info(addr, NULL, &b, NULL, NULL);

  return b;
}

bool
cl_st7::is_call(t_addr addr)
{
  struct dis_entry *e;

  get_disasm_info(addr, NULL, NULL, NULL, &e);

  return e?(e->is_call):false;
}

int
cl_st7::longest_inst(void)
{
  return 5;
}


const char *
cl_st7::get_disasm_info(t_addr addr,
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
	/* here will be all the prefixes for ST7 */
	case 0x90 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_st7_90[i].mask) != disass_st7_90[i].code &&
        disass_st7_90[i].mnemonic)
        i++;
      dis_e = &disass_st7_90[i];
      b= disass_st7_90[i].mnemonic;
      if (b != NULL)
        len += (disass_st7_90[i].length + 1);
    break;
	  
	case 0x91 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_st7_91[i].mask) != disass_st7_91[i].code &&
        disass_st7_91[i].mnemonic)
        i++;
      dis_e = &disass_st7_91[i];
      b= disass_st7_91[i].mnemonic;
      if (b != NULL)
        len += (disass_st7_91[i].length + 1);
    break;
	  
	case 0x92 :
	  code= rom->get(addr++);
      i= 0;
      while ((code & disass_st7_92[i].mask) != disass_st7_92[i].code &&
        disass_st7_92[i].mnemonic)
        i++;
      dis_e = &disass_st7_92[i];
      b= disass_st7_92[i].mnemonic;
      if (b != NULL)
        len += (disass_st7_92[i].length + 1);
    break;
	  
	
    default:
      i= 0;
      while ((code & disass_st7[i].mask) != disass_st7[i].code &&
             disass_st7[i].mnemonic)
        i++;
      dis_e = &disass_st7[i];
      b= disass_st7[i].mnemonic;
      if (b != NULL)
        len += (disass_st7[i].length);
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
cl_st7::disass(t_addr addr, const char *sep)
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
            //case 's': // s    signed byte immediate
            //  sprintf(temp, "#%d", (char)rom->get(addr+immed_offset));
            //  ++immed_offset;
            //  break;
            //case 'e': // e    extended 24bit immediate operand
            //  sprintf(temp, "#0x%06lx",
            //     (ulong)((rom->get(addr+immed_offset)<<16) |
            //            (rom->get(addr+immed_offset+1)<<8) |
            //            (rom->get(addr+immed_offset+2))) );
            //  ++immed_offset;
            //  ++immed_offset;
            //  ++immed_offset;
            //  break;
            //case 'w': // w    word immediate operand
            //  sprintf(temp, "#0x%04x",
            //     (uint)((rom->get(addr+immed_offset)<<8) |
            //            (rom->get(addr+immed_offset+1))) );
            //  ++immed_offset;
            //  ++immed_offset;
            //  break;
            case 'b': // b    byte immediate operand
              sprintf(temp, "#0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case 'd': // d    short direct addressing
              sprintf(temp, "$0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case 'x': // x    long direct
              sprintf(temp, "$0x%04x",
                 (uint)((rom->get(addr+immed_offset)<<8) |
                        (rom->get(addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            //case '3': // 3    24bit index offset
            //  sprintf(temp, "0x%06lx",
            //     (ulong)((rom->get(addr+immed_offset)<<16) |
            //            (rom->get(addr+immed_offset+1)<<8) |
            //            (rom->get(addr+immed_offset+2))) );
            //  ++immed_offset;
            //  ++immed_offset;
            //  ++immed_offset;
            // break;
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
		int i= (int)(addr+immed_offset+1
			     +(char)rom->get(addr+immed_offset)); 
		sprintf(temp, "0x%04x", i&0xffff);
		++immed_offset;
		break;
	      }
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
cl_st7::print_regs(class cl_console_base *con)
{
  con->dd_printf("---HINZC  Flags= 0x%02x %3d %c  ",
                 regs.CC, regs.CC, isprint(regs.CC)?regs.CC:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 regs.A, regs.A, isprint(regs.A)?regs.A:'.');
  con->dd_printf("---%c%c%c%c%c  ",
                 (regs.CC&BIT_H)?'1':'0',
                 (regs.CC&BIT_I)?'1':'0',
                 (regs.CC&BIT_N)?'1':'0',
                 (regs.CC&BIT_Z)?'1':'0',
                 (regs.CC&BIT_C)?'1':'0');
  con->dd_printf("X= 0x%02x %3d     %c  ",
                 regs.X, regs.X, isprint(regs.X)?regs.X:'.');
  con->dd_printf("Y= 0x%02x %3d %c\n",
                 regs.Y, regs.Y, isprint(regs.Y)?regs.Y:'.');
  con->dd_printf("SP= 0x%04x [SP+1]= %02x %3d %c",
                 regs.SP, ram->get(regs.SP+1), ram->get(regs.SP+1),
                 isprint(ram->get(regs.SP+1))?ram->get(regs.SP+1):'.');
  con->dd_printf("  Limit= 0x%04x\n", AU(sp_limit));
  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_st7::exec_inst(void)
{
  t_mem code;
  unsigned char cprefix; // prefix used for processing in functions

  if (regs.VECTOR) {
		regs.VECTOR = 0;
		PC = get2(0xFFFE);
		return(resGO);
	}

  instPC= PC;

  if (fetch(&code)) {
    //printf("******************** break \n");
	  return(resBREAKPOINT);
  }
  tick(1);

  switch (code) { // get prefix
	case 0x90:
	case 0x91:
	case 0x92:
		cprefix = code;
		fetch(&code);
		break;
	default:
		cprefix = 0x00;
		break;
  }
	
  //printf("********************  switch; pc=0x%lx, prefix = 0x%x, code = 0x%x\n",(long int)(PC), cprefix, code);
  switch (code & 0xf) {
   int mulres;
   
    case 0x0: 
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // NEG
            return( inst_neg( code, cprefix));
            break;
         case 0x80: // iret
            pop1( regs.CC);
            pop1( regs.A);
            pop1( regs.X);
            pop2( PC);
            return(resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // SUB
            return( inst_sub( code, cprefix));
            break;
         case 0x90: // this is prefix, do not be mad ???
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}
		
    case 0x1:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x80: // ret
            pop2( PC);
            return(resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // CP
            return( inst_cp( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // 
         case 0x90: // this is prefix, do not be mad ???
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0x2:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x40: // mul
            if(cprefix==0) {
               mulres = regs.X * regs.A;
               regs.A = mulres & 0xff;
               regs.X = mulres >> 8;
            }else if (cprefix == 0x90) {
               mulres = regs.Y * regs.A;
               regs.A = mulres & 0xff;
               regs.Y = mulres >> 8;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // SBC
            return( inst_sbc( code, cprefix));
            break;
         case 0x30:
         case 0x50:
         case 0x60:
         case 0x70: // 
         case 0x80: // 
         case 0x90: // this is prefix, do not be mad ???
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0x3:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // CPL
            return( inst_cpl( code, cprefix));
            break;
         case 0x80: // trap
	   //printf("************* TRAP instruction unimplemented !!!!\n");
            return(resINV_INST);
            break;
         case 0x90: // 
            if(cprefix==0) {
               regs.X = regs.Y;
            }else if (cprefix == 0x90) {
               regs.Y = regs.X;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // 
            return( inst_cpxy( code, cprefix));
            break;
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0x4:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // SRL
            return( inst_srl( code, cprefix));
            break;
         case 0x80: // pop A
            pop1( regs.A);
            return (resGO);
            break;
         case 0x90: // 
            if(cprefix==0) {
               regs.SP = regs.X;
            }else if (cprefix == 0x90) {
               regs.SP = regs.Y;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
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

    case 0x5:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x80: // 
            if(cprefix==0) {
               pop1(regs.X);
            }else if (cprefix == 0x90) {
               pop1(regs.Y);
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0x90: // 
            if(cprefix==0) {
               regs.SP = regs.A;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // BCP
            return( inst_bcp( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // 
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0x6:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // 
            return( inst_rrc( code, cprefix));
            break;
         case 0x80: // 
            pop1( regs.CC);
            return (resGO);
            break;
         case 0x90: // 
            if(cprefix==0) {
               regs.X = regs.SP;
            }else if (cprefix == 0x90) {
               regs.Y = regs.SP;
            } else {
               return (resHALT);
            }
            return (resGO);
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

    case 0x7:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // SRA
            return( inst_sra( code, cprefix));
            break;
         case 0x90: // 
            if(cprefix==0) {
               regs.X = regs.A;
            }else if (cprefix == 0x90) {
               regs.Y = regs.A;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0xA0:
            break;
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // 
            return( inst_lddst( code, cprefix));
            break;
         case 0x80: // 
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0x8:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // SLA, SLL
            return( inst_sll( code, cprefix));
            break;
         case 0x80: // push A
            push1(regs.A);
            return (resGO);
            break;
         case 0x90: // rcf
            FLAG_CLEAR (BIT_C);
            return (resGO);
            break;
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

    case 0x9:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // RLC
            return( inst_rlc( code, cprefix));
            break;
         case 0x80: // push X; push Y
            if(cprefix==0) {
               push1(regs.X);
            }else if (cprefix == 0x90) {
               push1(regs.Y);
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0x90: // scf
            FLAG_SET (BIT_C);
            return (resGO);
            break;
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

    case 0xa:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // DEC
            return( inst_dec( code, cprefix));
            break;
         case 0x80: // 
            push1(regs.CC);
            return (resGO);
            break;
         case 0x90: // RIM
            FLAG_CLEAR (BIT_I);
            return (resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // 
            return( inst_or( code, cprefix));
            break;
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0xb:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x90: // SIM
            FLAG_SET (BIT_I);
            return (resGO);
            break;
         case 0xA0:
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // ADD
            return( inst_add( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // 
         case 0x80: // 
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0xc:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // INC
            return( inst_inc( code, cprefix));
            break;
         case 0x90: // RSP
            regs.SP = 0x17f;
            return (resGO);
            break;
         case 0xA0:
            break;
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // JP
            PC = OPERAND(code, cprefix);
            return( resGO);
            break;
         case 0x80: // 
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0xd:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // TNZ
            return( inst_tnz( code, cprefix));
            break;
         case 0x90: // nop
            return(resGO);
            break;
         case 0xA0: // callr
            return( inst_callr( code, cprefix));
            break;
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // call
            return( inst_call( code, cprefix));
            break;
         case 0x80: // 
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}

    case 0xe:      
      switch ( code & 0xf0) {
          case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // SWAP
            return( inst_swap( code, cprefix));
            break;
         case 0x80: // HALT
            FLAG_CLEAR (BIT_I);
            PC--;
            return(resHALT);
            break;
         case 0x90: // LD A,SP
            if(cprefix==0) {
               regs.A = regs.SP;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
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

    case 0xf:      
      switch ( code & 0xf0) {
         case 0x00: // btjf, btjt
            return(inst_btjfbtjt(code, cprefix)); 
            break;
         case 0x10: // bres,btst
            return(inst_bresbset(code, cprefix));
            break;
         case 0x20:  //JR
            return( inst_jr( code, cprefix));
            break;
         case 0x30:
         case 0x40:
         case 0x50:
         case 0x60:
         case 0x70: // 
            return( inst_clr( code, cprefix));
            break;
         case 0x80: // WFI
            FLAG_CLEAR (BIT_I);
            PC--;
            return(resHALT);
            break;
         case 0x90: // ld A,X LD A,Y
            if(cprefix==0) {
               regs.A = regs.X;
            }else if (cprefix == 0x90) {
               regs.A = regs.Y;
            } else {
               return (resHALT);
            }
            return (resGO);
            break;
         case 0xB0:
         case 0xC0:
         case 0xD0:
         case 0xE0:
         case 0xF0: // 
            return( inst_ldxydst( code, cprefix));
            break;
         case 0xA0:
         default: 
	   //printf("************* bad code !!!!\n");
            return(resINV_INST);
		}


    default:
      //printf("************* bad code !!!!\n");
		return(resINV_INST);
	}
	  
  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);

  sim->stop(resINV_INST);
  return(resINV_INST);
}


void
cl_st7::stack_check_overflow(class cl_stack_op *op)
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

t_mem
cl_st7::get_1(t_addr addr)
{
  vc.rd++;
  return ram->read(addr);
}

t_mem
cl_st7::get_2(t_addr addr)
{
  vc.rd+= 2;
  return (ram->read(addr) << 8) | ram->read(addr+1);
}

t_mem
cl_st7::get_3(t_addr addr)
{
  vc.rd+= 3;
  return (ram->read(addr) << 16) |
    (ram->read(addr+1) << 8) |
    (ram->read(addr+2));
}


cl_st7_cpu::cl_st7_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
}

int
cl_st7_cpu::init(void)
{
  cl_hw::init();

  cl_var *v;
  uc->vars->add(v= new cl_var(cchars("sp_limit"), cfg, st7cpu_sp_limit,
			      cfg_help(st7cpu_sp_limit)));
  v->init();

  return 0;
}

char *
cl_st7_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case st7cpu_sp_limit:
      return (char*)"Stack overflows when SP is below this limit";
    }
  return (char*)"Not used";
}

t_mem
cl_st7_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_st7 *u= (class cl_st7 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum st7cpu_confs)addr)
    {
    case st7cpu_sp_limit:
      if (val)
	u->sp_limit= *val & 0xffff;
      else
	cell->set(u->sp_limit);
      break;
    case st7cpu_nuof: break;
    }
  return cell->get();
}


/* End of st7.src/st7.cc */
