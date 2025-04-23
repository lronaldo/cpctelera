/*
 * Simulator of microcontrollers (osc.cc)
 *
 * Copyright (C) 2024 Drotos Daniel
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

#include "pdkcl.h"

#include "osccl.h"


cl_osc::cl_osc(class cl_uc *auc, const char *aname):
  cl_hw(auc, HW_TIMER, 0, aname)
{
  puc= (class cl_pdk *)auc;
  ihrc= 0;
  ilrc= 0;
  eosc= 0;
  sys = 0;
}

int
cl_osc::init(void)
{
  cl_hw::init();
  clkmd= register_cell(puc->sfr, 3);
  eoscr= register_cell(puc->sfr, 0xa);
  cl_hw::init();
  /*
  uc->reg_cell_var(cfg_cell(osc_freq_ihrc), &frh,
		   "freq_ihrc", cfg_help(osc_freq_ihrc));
  uc->reg_cell_var(cfg_cell(osc_freq_ilrc), &frh,
		   "freq_ilrc", cfg_help(osc_freq_ilrc));
  uc->reg_cell_var(cfg_cell(osc_freq_eosc), &frh,
		   "freq_eosc", cfg_help(osc_freq_eosc));
  */
  uc->mk_mvar(cfg, osc_freq_ihrc, "freq_ihrc",
	      cfg_help(osc_freq_ihrc));
  cfg_cell(osc_freq_ihrc)->decode(&frh);

  uc->mk_mvar(cfg, osc_freq_ilrc, "freq_ilrc", 
	      cfg_help(osc_freq_ilrc));
  cfg_cell(osc_freq_ilrc)->decode(&frl);

  uc->mk_mvar(cfg, osc_freq_eosc, "freq_eosc", 
	      cfg_help(osc_freq_eosc));
  cfg_cell(osc_freq_eosc)->decode(&fre);

  fre= puc->get_xtal();
  frh= 16000000;
  frl= 24000;
  reset();
  return 0;
}

void
cl_osc::recalc(void)
{
  t_mem v;
  v= clkmd->get();
  write(clkmd, &v);
  v= eoscr->get();
  write(eoscr, &v);
}

const char *
cl_osc::cfg_help(t_addr addr)
{
  switch ((enum osc_cfg)addr)
    {
    case osc_on: return "Turn ticking of osc on/off (bool, RW)";
    case osc_freq_ihrc: return "Frequ of IHRC oscillator (Hz, RW)";
    case osc_freq_ilrc: return "Frequ of ILRC oscillator (Hz, RW)";
    case osc_freq_eosc: return "Frequ of external oscillator (Hz, RW)";
    case osc_nuof: return "";
    }
  return "Not used";
}

void
cl_osc::reset(void)
{
  t_mem v;
  v= 0xf6;
  puc->sfr->write(3/*clkmd*/, v);
  v= 0x80;
  write(eoscr, &v);
}

void
cl_osc::write(class cl_memory_cell *cell, t_mem *val)
{
  u8_t v= *val;
  if (conf(cell, val))
    return;
  if (cell == eoscr)
    {
      rune= v & 0x80;
    }
  else if (cell == clkmd)
    {
      runh= v & 0x10;
      runl= v & 0x04;
      if (!(v & 0x08))
	{
	  // Type 0
	  switch ((v>>5) & 7)
	    {
	    case 0: setup(frh, 4); sys_source="ihrc/4"; break;
	    case 1: setup(frh, 2); sys_source="ihrc/2"; break;
	    case 2: break;
	    case 3: setup(fre, 4); sys_source="eosc/4"; break;
	    case 4: setup(fre, 2); sys_source="eosc/2"; break;
	    case 5: setup(fre, 1); sys_source="eosc"; break;
	    case 6: setup(frl, 4); sys_source="ilrc/4"; break;
	    case 7: setup(frl, 1); sys_source="ilrc"; break;
	    }
	}
      else
	{
	  // Type 1
	  switch ((v>>5) & 7)
	    {
	    case 0: setup(frh,16); sys_source="ihrc/16"; break;
	    case 1: setup(frh, 8); sys_source="ihrc/8"; break;
	    case 2: setup(frl,16); sys_source="ilrc/16"; break;
	    case 3: setup(frh,32); sys_source="ihrc/32"; break;
	    case 4: setup(frh,64); sys_source="ihrc/64"; break;
	    case 5: setup(fre, 8); sys_source="eosc/8"; break;
	    case 6: setup(frl, 2); sys_source="ilrc/2"; break;
	    case 7: break;
	    }
	}
    }
  cell->set(*val);
}

t_mem
cl_osc::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  switch (addr)
    {
    case osc_on: // turn this HW on/off
      if (val)
	on= *val;
      else
	cell->set(on?1:0);
      break;
    default:
      if (val)
	{
	  cell->set(*val);
	  recalc();
	}
      break;
    }
    return cell->get();
}

int
cl_osc::tick(int cycles)
{
  if (puc->mode == pm_run) sys+= cycles;
  if (puc->mode != pm_pd)
    {
      if (runh) ihrc+= cycles * mh;
      if (runl) ilrc+= cycles * ml;
      if (rune) eosc+= cycles * me;
    }
  return 0;
}

void
cl_osc::setup(t_mem src_fr, unsigned int div_by)
{
  frsys= (double)src_fr/div_by;
  mh= frh/frsys;
  ml= frl/frsys;
  me= fre/frsys;
  puc->set_xtal(frsys);
}

void
cl_osc::print_info(class cl_console_base *con)
{
  con->dd_printf("frsys=%f source=%s\n", frsys, sys_source.c_str());
  con->dd_printf("Hon=%d Lon=%d Eon=%d\n", runh?1:0, runl?1:0, rune?1:0);
  con->dd_printf("Th=%f Tl=%f Te=%f\n", ihrc, ilrc, eosc);
  con->dd_printf("frh=%u frl=%u fre=%u\n", MU(frh), MU(frl), MU(fre));
  con->dd_printf("mh=%f ml=%f me=%f\n", mh, ml, me);
}


/* End of pdk.src/osc.cc */
