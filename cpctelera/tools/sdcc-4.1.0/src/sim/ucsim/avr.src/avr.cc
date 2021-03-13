/*
 * Simulator of microcontrollers (avr.cc)
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//#include "i_string.h"

// prj
//#include "pobjcl.h"

// sim
#include "simcl.h"
//#include "memcl.h"

// local
#include "portcl.h"
#include "avrcl.h"
#include "glob.h"
#include "regsavr.h"


// Addresses are IRAM addresses!
static struct name_entry sfr_tabl[]= {
  { CPU_ALL_AVR, 0x001a, "XL"},
  { CPU_ALL_AVR, 0x001a, "XL" },
  { CPU_ALL_AVR, 0x001b, "XH" },
  { CPU_ALL_AVR, 0x001c, "YL" },
  { CPU_ALL_AVR, 0x001d, "YH" },
  { CPU_ALL_AVR, 0x001e, "ZL" },
  { CPU_ALL_AVR, 0x001f, "ZH" },
  { CPU_ALL_AVR, 0x0024, "ADCL" },
  { CPU_ALL_AVR, 0x0025, "ADCH" },
  { CPU_ALL_AVR, 0x0026, "ADCSR" },
  { CPU_ALL_AVR, 0x0027, "ADMUX" },
  { CPU_ALL_AVR, 0x0028, "ACSR" },
  { CPU_ALL_AVR, 0x0029, "UBRR" },
  { CPU_ALL_AVR, 0x002A, "UCR" },
  { CPU_ALL_AVR, 0x002B, "USR" },
  { CPU_ALL_AVR, 0x002C, "UDR" },
  { CPU_ALL_AVR, 0x002D, "SPCR" },
  { CPU_ALL_AVR, 0x002E, "SPSR" },
  { CPU_ALL_AVR, 0x002F, "SPDR" },
  { CPU_ALL_AVR, 0x0030, "PIND" },
  { CPU_ALL_AVR, 0x0031, "DDRD" },
  { CPU_ALL_AVR, 0x0032, "PORTD" },
  { CPU_ALL_AVR, 0x0033, "PINC" },
  { CPU_ALL_AVR, 0x0034, "DDRC" },
  { CPU_ALL_AVR, 0x0035, "PORTC" },
  { CPU_ALL_AVR, 0x0036, "PINB" },
  { CPU_ALL_AVR, 0x0037, "DDRB" },
  { CPU_ALL_AVR, 0x0038, "PORTB" },
  { CPU_ALL_AVR, 0x0039, "PINA" },
  { CPU_ALL_AVR, 0x003A, "DDRA" },
  { CPU_ALL_AVR, 0x003B, "PORTA" },
  { CPU_ALL_AVR, 0x003C, "EECR" },
  { CPU_ALL_AVR, 0x003D, "EEDR" },
  { CPU_ALL_AVR, 0x003E, "EEARL" },
  { CPU_ALL_AVR, 0x003E, "EEARH" },
  { CPU_ALL_AVR, 0x0041, "WDTCR" },
  { CPU_ALL_AVR, 0x0042, "ASSR" },
  { CPU_ALL_AVR, 0x0043, "OCR2" },
  { CPU_ALL_AVR, 0x0044, "TCNT2" },
  { CPU_ALL_AVR, 0x0045, "TCCR2" },
  { CPU_ALL_AVR, 0x0046, "ICR1L" },
  { CPU_ALL_AVR, 0x0047, "ICR1H" },
  { CPU_ALL_AVR, 0x0048, "OCR1BL" },
  { CPU_ALL_AVR, 0x0049, "OCR1BH" },
  { CPU_ALL_AVR, 0x004A, "OCR1AL" },
  { CPU_ALL_AVR, 0x004B, "OCR1AH" },
  { CPU_ALL_AVR, 0x004C, "TCNT1L" },
  { CPU_ALL_AVR, 0x004D, "TCNT1H" },
  { CPU_ALL_AVR, 0x004E, "TCCR1B" },
  { CPU_ALL_AVR, 0x004F, "TCCR1A" },
  { CPU_ALL_AVR, 0x0052, "TCNT0" },
  { CPU_ALL_AVR, 0x0053, "TCCR0" },
  { CPU_ALL_AVR, 0x0054, "MCUSR" },
  { CPU_ALL_AVR, 0x0055, "MCUCR" },
  { CPU_ALL_AVR, 0x0058, "TIFR" },
  { CPU_ALL_AVR, 0x0059, "TIMSK" },
  { CPU_ALL_AVR, 0x005A, "GIFR" },
  { CPU_ALL_AVR, 0x005B, "GIMSK" },
  { CPU_ALL_AVR, 0x005D, "SPL" },
  { CPU_ALL_AVR, 0x005E, "SPH" },
  { CPU_ALL_AVR, 0x005F, "SREG" },
  {0, 0, NULL}
};

/*
 * Base type of AVR microcontrollers
 */

cl_avr::cl_avr(class cl_sim *asim):
  cl_uc(asim)
{
  type= (struct cpu_entry *)malloc(sizeof(struct cpu_entry));
  type->type= CPU_AVR;
  sleep_executed= 0;
}

int
cl_avr::init(void)
{
  cl_uc::init(); /* Memories now exist */
  int i;
  for (i= 0; sfr_tabl[i].name != NULL; i++)
    {
      if (type->type & sfr_tabl[i].cpu_type)
	{
	  class cl_var *v;
	  vars->add(v= new cl_var(chars(sfr_tabl[i].name),
				  ram,
				  sfr_tabl[i].addr, ""));
	  v->init();
	}
    }
  return(0);
}

const char *
cl_avr::id_string(void)
{
  return("unspecified AVR");
}


/*
 * Making elements of the controller
 */

void
cl_avr::mk_hw_elements(void)
{
  class cl_hw *h;
  cl_uc::mk_hw_elements();
  add_hw(h= new cl_port(this));
  h->init();
}


void
cl_avr::make_memories(void)
{
  class cl_address_space *as;

  rom= as= new cl_address_space("rom"/*MEM_ROM_ID*/, 0, 0x10000, 16);
  as->init();
  address_spaces->add(as);
  ram= as= new cl_address_space(MEM_IRAM_ID, 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_memory_chip("rom_chip", 0x10000, 16);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= rom/*address_space(MEM_ROM_ID)*/,
			     chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("iram_chip", 0x80, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= ram/*address_space(MEM_IRAM_ID)*/,
			     chip, 0, 0x7f, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_avr::dis_tbl(void)
{
  return(disass_avr);
}

char *
cl_avr::disass(t_addr addr, const char *sep)
{
  char work[256], temp[20];
  char *buf, *p, *t, *s;
  const char *b;
  uint code, data= 0;
  int i;

  p= work;

  code= rom/*get_mem*/->get(/*MEM_ROM_ID,*/ addr);
  i= 0;
  while ((code & dis_tbl()[i].mask) != dis_tbl()[i].code &&
	 dis_tbl()[i].mnemonic)
    i++;
  if (dis_tbl()[i].mnemonic == NULL)
    {
      buf= (char*)malloc(30);
      strcpy(buf, "UNKNOWN/INVALID");
      return(buf);
    }
  b= dis_tbl()[i].mnemonic;

  while (*b)
    {
      if (*b == '%')
	{
	  b++;
	  switch (*(b++))
	    {
	    case 'd': // Rd   .... ...d dddd ....  0<=d<=31
	      if (!/*get*/addr_name(data= (code&0x01f0)>>4, /*sfr_tbl()*/ram, temp))
		sprintf(temp, "r%d", data);
	      break;
	    case 'D': // Rd   .... .... dddd ....  16<=d<=31
	      if (!/*get*/addr_name(data= 16+((code&0xf0)>>4), /*sfr_tbl()*/ram, temp))
		sprintf(temp, "r%d", data);
	      break;
	    case 'K': // K    .... KKKK .... KKKK  0<=K<=255
	      sprintf(temp, "%d", ((code&0xf00)>>4)|(code&0xf));
	      break;
	    case 'r': // Rr   .... ..r. .... rrrr  0<=r<=31
	      if (!/*get*/addr_name(data= ((code&0x0200)>>5)|(code&0x000f),
				    /*sfr_tbl()*/ram, temp))
		sprintf(temp, "r%d", data);
	      break;
	    case '2': // Rdl  .... .... ..dd ....  dl= {24,26,28,30}
	      if (!/*get*/addr_name(data= 24+(2*((code&0x0030)>>4)),
				    /*sfr_tbl()*/ram, temp))
		sprintf(temp, "r%d", data);
	      break;
	    case '6': // K    .... .... KK.. KKKK  0<=K<=63
	      sprintf(temp, "%d", ((code&0xc0)>>2)|(code&0xf));
	      break;
	    case 's': // s    .... .... .sss ....  0<=s<=7
	      sprintf(temp, "%d", (code&0x70)>>4);
	      break;
	    case 'b': // b    .... .... .... .bbb  0<=b<=7
	      sprintf(temp, "%d", code&0x7);
	      break;
	    case 'k': // k    .... ..kk kkkk k...  -64<=k<=+63
	      {
		int k= (code&0x3f8)>>3;
		if (code&0x200)
		  k|= -128;
		sprintf(temp, "0x%06x", k+1+(signed int)addr);
		break;
	      }
	    case 'A': // k    .... ...k kkkk ...k  0<=k<=64K
	              //      kkkk kkkk kkkk kkkk  0<=k<=4M
	      sprintf(temp, "0x%06x",
		      (((code&0x1f0)>>3)|(code&1))*0x10000+
		      (uint)rom->get/*_mem*/(/*MEM_ROM_ID,*/ addr+1));
	      break;
	    case 'P': // P    .... .... pppp p...  0<=P<=31
	      data= (code&0xf8)>>3;
	      if (!/*get*/addr_name(data+0x20, /*sfr_tbl()*/ram, temp))
		sprintf(temp, "%d", data);
	      break;
	    case 'p': // P    .... .PP. .... PPPP  0<=P<=63
	      data= ((code&0x600)>>5)|(code&0xf);
	      if (!/*get*/addr_name(data+0x20, /*sfr_tbl()*/ram, temp))
		sprintf(temp, "%d", data);
	      break;
	    case 'q': // q    ..q. qq.. .... .qqq  0<=q<=63
	      sprintf(temp, "%d",
		      ((code&0x2000)>>8)|((code&0xc00)>>7)|(code&7));
	      break;
	    case 'R': // k    SRAM address on second word 0<=k<=65535
	      sprintf(temp, "0x%06x", (uint)rom->get/*_mem*/(/*MEM_ROM_ID,*/ addr+1));
	      break;
	    case 'a': // k    .... kkkk kkkk kkkk  -2k<=k<=2k
	      {
		int k= code&0xfff;
		if (code&0x800)
		  k|= -4096;
		sprintf(temp, "0x%06x",
			(int)rom->validate_address(k+1+(signed int)addr));
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
  for (p= work, s= buf; *p != ' '; p++, s++)
    *s= *p;
  p++;
  *s= '\0';
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
cl_avr::print_regs(class cl_console_base *con)
{
  uchar data, sreg= ram->get(SREG);
  uint x, y, z;

  ram->dump(0, 31, 16, con/*->get_fout()*/);

  con->dd_color("answer");
  con->dd_printf("ITHSVNZC  SREG= 0x%02x %3d %c\n",
		 sreg, sreg, isprint(sreg)?sreg:'.');
  con->dd_printf("%c%c%c%c%c%c%c%c  ",
		 (sreg&BIT_I)?'1':'0',
		 (sreg&BIT_T)?'1':'0',
		 (sreg&BIT_H)?'1':'0',
		 (sreg&BIT_S)?'1':'0',
		 (sreg&BIT_V)?'1':'0',
		 (sreg&BIT_N)?'1':'0',
		 (sreg&BIT_Z)?'1':'0',
		 (sreg&BIT_C)?'1':'0');
  con->dd_printf("SP  = 0x%06x\n", ram->get(SPH)*256+ram->get(SPL));

  x= ram->get(XH)*256 + ram->get(XL);
  data= ram->get(x);
  con->dd_printf("X= 0x%04x [X]= 0x%02x %3d %c  ", x,
		 data, data, isprint(data)?data:'.');
  y= ram->get(YH)*256 + ram->get(YL);
  data= ram->get(y);
  con->dd_printf("Y= 0x%04x [Y]= 0x%02x %3d %c  ", y,
		 data, data, isprint(data)?data:'.');
  z= ram->get(ZH)*256 + ram->get(ZL);
  data= ram->get(z);
  con->dd_printf("Z= 0x%04x [Z]= 0x%02x %3d %c\n", z,
		 data, data, isprint(data)?data:'.');

  print_disass(PC, con);
}


/*
 * Execution
 */

int
cl_avr::exec_inst(void)
{
  t_mem code;

  instPC= PC;
  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  switch (code)
    {
    case 0x9419:
      return(eijmp(code));
    case 0x9519:
      return(eicall(code));
    case 0x9508: case 0x9528: case 0x9548: case 0x9568:
      return(ret(code));
    case 0x9518: case 0x9538: case 0x9558: case 0x9578:
      return(reti(code));
    case 0x95c8:
      return(lpm(code));
    case 0x95d8:
      return(elpm(code)); // in some devices equal to lpm
    case 0x95e8:
      return(spm(code));
    case 0x95f8:
      return(espm(code));
    case 0x9408:
      return(sec(code));
    case 0x9488:
      return(clc(code));
    case 0x9428:
      return(sen(code));
    case 0x94a8:
      return(cln(code));
    case 0x9418:
      return(sez(code));
    case 0x9498:
      return(clz(code));
    case 0x9478:
     return(sei(code));
    case 0x94f8:
      return(cli(code));
    case 0x9448:
      return(ses(code));
    case 0x94c8:
      return(cls(code));
    case 0x9438:
      return(sev(code));
    case 0x94b8:
      return(clv(code));
    case 0x9468:
      return(set(code));
    case 0x94e8:
      return(clt(code));
    case 0x9458:
      return(seh(code));
    case 0x94d8:
      return(clh(code));
    case 0x0000:
      return(nop(code));
    case 0x9588: case 0x9598:
      return(sleep(code));
    case 0x95a8: case 0x95b8:
      return(wdr(code));
    }
  switch (code & 0xf000)
    {
    case 0x3000: return(cpi_Rd_K(code));
    case 0x4000: return(sbci_Rd_K(code));
    case 0x5000: return(subi_Rd_K(code));
    case 0x6000: return(ori_Rd_K(code));
    case 0x7000: return(andi_Rd_K(code));
    case 0xc000: return(rjmp_k(code));
    case 0xd000: return(rcall_k(code));
    case 0xe000: return(ldi_Rd_K(code));
    }
  switch (code & 0xf000)
    {
    case 0x0000:
      {
	// 0x0...
	switch (code & 0xfc00)
	  {
	  case 0x0000:
	    {
	      switch (code & 0xff00)
		{
		case 0x0100: return(movw_Rd_Rr(code));
		case 0x0200: return(muls_Rd_Rr(code));
		case 0x0300:
		  {
		    switch (code & 0xff88)
		      {
		      case 0x0300: return(mulsu_Rd_Rr(code));
		      case 0x0308: return(fmul_Rd_Rr(code));
		      case 0x0380: return(fmuls_Rd_Rr(code));
		      case 0x0388: return(fmulsu_Rd_Rr(code));
		      }
		    break;
		  }
		  break;
		}
	      break;
	    }
	  case 0x0400: return(cpc_Rd_Rr(code));
	  case 0x0800: return(sbc_Rd_Rr(code));
	  case 0x0c00: return(add_Rd_Rr(code));
	  }
	break;
      }
    case 0x1000:
      {
	// 0x1...
	switch (code & 0xfc00)
	  {
	  case 0x1000: return(cpse_Rd_Rr(code));
	  case 0x1400: return(cp_Rd_Rr(code));
	  case 0x1800: return(sub_Rd_Rr(code));
	  case 0x1c00: return(adc_Rd_Rr(code));
	  }
	break;
      }
    case 0x2000:
      {
	// 0x2...
	switch (code & 0xfc00)
	  {
	  case 0x2000: return(and_Rd_Rr(code));
	  case 0x2400: return(eor_Rd_Rr(code));
	  case 0x2800: return(or_Rd_Rr(code));
	  case 0x2c00: return(mov_Rd_Rr(code));
	}
	break;
      }
    case 0x8000:
      {
	// 0x8...
	switch (code &0xf208)
	  {
	  case 0x8000: return(ldd_Rd_Z_q(code));
	  case 0x8008: return(ldd_Rd_Y_q(code));
	  case 0x8200: return(std_Z_q_Rr(code));
	  case 0x8208: return(std_Y_q_Rr(code));
	  }
	break;
      }
    case 0x9000:
      {
	// 0x9...
	if ((code & 0xff0f) == 0x9509)
	  return(icall(code));
	if ((code & 0xff0f) == 0x9409)
	  return(ijmp(code));
	if ((code & 0xff00) == 0x9600)
	  return(adiw_Rdl_K(code));
	if ((code & 0xff00) == 0x9700)
	  return(sbiw_Rdl_K(code));
	switch (code & 0xfc00)
	  {
	  case 0x9000:
	    {
	      switch (code & 0xfe0f)
		{
		case 0x9000: return(lds_Rd_k(code));
		case 0x9001: return(ld_Rd_ZS(code));
		case 0x9002: return(ld_Rd_SZ(code));
		case 0x9004: return(lpm_Rd_Z(code));
		case 0x9005: return(lpm_Rd_ZS(code));
		case 0x9006: return(elpm_Rd_Z(code));
		case 0x9007: return(elpm_Rd_ZS(code));
		case 0x9009: return(ld_Rd_YS(code));
		case 0x900a: return(ld_Rd_SY(code));
		case 0x900c: return(ld_Rd_X(code));
		case 0x900d: return(ld_Rd_XS(code));
		case 0x900e: return(ld_Rd_SX(code));
		case 0x900f: return(pop_Rd(code));
		case 0x9200: return(sts_k_Rr(code));
		case 0x9201: return(st_ZS_Rr(code));
		case 0x9202: return(st_SZ_Rr(code));
		case 0x9209: return(st_YS_Rr(code));
		case 0x920a: return(st_SY_Rr(code));
		case 0x920c: return(st_X_Rr(code));
		case 0x920d: return(st_XS_Rr(code));
		case 0x920e: return(st_SX_Rr(code));
		case 0x920f: return(push_Rr(code));
		}
	      break;
	    }
	  case 0x9400:
	    {
	      switch (code & 0xfe0f)
		{
		case 0x9400: return(com_Rd(code));
		case 0x9401: return(neg_Rd(code));
		case 0x9402: return(swap_Rd(code));
		case 0x9403: return(inc_Rd(code));
		case 0x9405: return(asr_Rd(code));
		case 0x9406: return(lsr_Rd(code));
		case 0x9407: return(ror_Rd(code));
		case 0x940a: return(dec_Rd(code));
		case 0x940c: case 0x940d: return(jmp_k(code));
		case 0x940e: case 0x940f: return(call_k(code));
		}
	      break;
	    }
	  case 0x9800:
	    {
	      switch (code & 0xff00)
		{
		case 0x9800: return(cbi_A_b(code));
		case 0x9900: return(sbic_P_b(code));
		case 0x9a00: return(sbi_A_b(code));
		case 0x9b00: return(sbis_P_b(code));
		}
	      break;
	    }
	  case 0x9c00: return(mul_Rd_Rr(code));
	  }
	break;
      }
    case 0xa000:
      {
	// 0xa...
	switch (code &0xf208)
	  {
	  case 0xa000: return(ldd_Rd_Z_q(code));
	  case 0xa008: return(ldd_Rd_Y_q(code));
	  case 0xa200: return(std_Z_q_Rr(code));
	  case 0xa208: return(std_Y_q_Rr(code));
	  }
	break;
      }
    case 0xb000:
      {
	// 0xb...
	switch (code & 0xf800)
	  {
	  case 0xb000: return(in_Rd_A(code));
	  case 0xb800: return(out_A_Rr(code));
	  }
	break;
      }
    case 0xe000:
      {
	// 0xe...
	switch (code & 0xff0f)
	  {
	  case 0xef0f: return(ser_Rd(code));
	  }
	break;
      }
    case 0xf000:
      {
	// 0xf...
	switch (code & 0xfc00)
	  {
	  case 0xf000: return(brbs_s_k(code));
	  case 0xf400: return(brbc_s_k(code));
	  case 0xf800: case 0xfc00:
	    {
	      switch (code & 0xfe08)
		{
		case 0xf800: return(bld_Rd_b(code));
		case 0xfa00: return(bst_Rd_b(code));
		case 0xfc00: case 0xfc08: return(sbrc_Rr_b(code));
		case 0xfe00: case 0xfe08: return(sbrs_Rr_b(code));
		}
	      break;
	    }
	  }
	break;
      }
    }
  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  class cl_error_unknown_code *e= new cl_error_unknown_code(this);
  error(e);
  return(resGO);
  PC= rom->inc_address(PC, -1);
  //tick(-clock_per_cycle());
  sim->stop(resINV_INST);
  return(resINV_INST);
}


/*
 */

int
cl_avr::push_data(t_mem data)
{
  t_addr sp;
  t_mem spl, sph;

  spl= ram->read(SPL);
  sph= ram->read(SPH);
  sp= 0xffff & (256*sph + spl);
  data= ram->write(sp, data);
  vc.wr++;
  sp= 0xffff & (sp-1);
  spl= sp & 0xff;
  sph= (sp>>8) & 0xff;
  ram->write(SPL, spl);
  ram->write(SPH, sph);
  return(resGO);
}

int
cl_avr::push_addr(t_addr addr)
{
  t_addr sp;
  t_mem spl, sph, al, ah;

  spl= ram->read(SPL);
  sph= ram->read(SPH);
  sp= 0xffff & (256*sph + spl);
  al= addr & 0xff;
  ah= (addr>>8) & 0xff;
  ram->write(sp, ah);
  sp= 0xffff & (sp-1);
  ram->write(sp, al);
  vc.wr+= 2;
  sp= 0xffff & (sp-1);
  spl= sp & 0xff;
  sph= (sp>>8) & 0xff;
  ram->write(SPL, spl);
  ram->write(SPH, sph);
  return(resGO);
}

int
cl_avr::pop_data(t_mem *data)
{
  t_addr sp;
  t_mem spl, sph;

  spl= ram->read(SPL);
  sph= ram->read(SPH);
  sp= 256*sph + spl;
  sp= 0xffff & (sp+1);
  *data= ram->read(sp);
  vc.rd++;
  spl= sp & 0xff;
  sph= (sp>>8) & 0xff;
  ram->write(SPL, spl);
  ram->write(SPH, sph);

  return(resGO);
}

int
cl_avr::pop_addr(t_addr *addr)
{
  t_addr sp;
  t_mem spl, sph, al, ah;

  spl= ram->read(SPL);
  sph= ram->read(SPH);
  sp= 256*sph + spl;
  sp= 0xffff & (sp+1);
  al= ram->read(sp);
  sp= 0xffff & (sp+1);
  ah= ram->read(sp);
  vc.rd+= 2;
  *addr= ah*256 + al;
  spl= sp & 0xff;
  sph= (sp>>8) & 0xff;
  ram->write(SPL, spl);
  ram->write(SPH, sph);

  return(resGO);
}


/*
 * Set Z, N, V, S bits of SREG after logic instructions and some others
 */

void
cl_avr::set_zn0s(t_mem data)
{
  t_mem sreg= ram->get(SREG) & ~BIT_V;
  data= data&0xff;
  if (!data)
    sreg|= BIT_Z;
  else
    sreg&= ~BIT_Z;
  if (data & 0x80)
    sreg|= (BIT_N|BIT_S);
  else
    sreg&= ~(BIT_N|BIT_S);
  ram->set(SREG, sreg);
}


/* End of avr.src/avr.cc */
