/*
 * Simulator of microcontrollers (serial.cc)
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

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>

// local
#include "uc390hwcl.h"
#include "regs51.h"
#include "uc51cl.h"


cl_uc390_hw::cl_uc390_hw (class cl_uc *auc):
  cl_hw (auc, HW_DUMMY, 0, "ds390hw")
{
  uc390 = (class cl_uc390 *) uc;
}

int
cl_uc390_hw::init(void)
{
  sfr = uc->address_space(MEM_SFR_ID);
  if (sfr)
    {
      /*cell_dps   = sfr->register_hw (DPS  , this, 0);
      cell_p4cnt = sfr->register_hw (P4CNT, this, 0);
      cell_exif  = sfr->register_hw (EXIF , this, 0);
      cell_acon  = sfr->register_hw (ACON , this, 0);
      cell_p5cnt = sfr->register_hw (P5CNT, this, 0);
      cell_c0c   = sfr->register_hw (C0C  , this, 0);
      cell_pmr   = sfr->register_hw (PMR  , this, 0);
      cell_mcon  = sfr->register_hw (MCON , this, 0);
      cell_ta    = sfr->register_hw (TA   , this, 0);
      cell_cor   = sfr->register_hw (COR  , this, 0);
      cell_mcnt0 = sfr->register_hw (MCNT0, this, 0);
      cell_mcnt1 = sfr->register_hw (MCNT1, this, 0);
      cell_ma    = sfr->register_hw (MA   , this, 0);
      cell_mb    = sfr->register_hw (MB   , this, 0);
      cell_mc    = sfr->register_hw (MC   , this, 0);
      cell_wdcon = sfr->register_hw (WDCON, this, 0);
      cell_c1c   = sfr->register_hw (C1C  , this, 0);*/
      register_cell (sfr, DPS  , &cell_dps  , wtd_restore);
      register_cell (sfr, P4CNT, &cell_p4cnt, wtd_restore);
      register_cell (sfr, EXIF , &cell_exif , wtd_restore);
      register_cell (sfr, ACON , &cell_acon , wtd_restore);
      register_cell (sfr, P5CNT, &cell_p5cnt, wtd_restore);
      register_cell (sfr, C0C  , &cell_c0c  , wtd_restore);
      register_cell (sfr, PMR  , &cell_pmr  , wtd_restore);
      register_cell (sfr, MCON , &cell_mcon , wtd_restore);
      register_cell (sfr, TA   , &cell_ta   , wtd_restore);
      register_cell (sfr, COR  , &cell_cor  , wtd_restore);
      register_cell (sfr, MCNT0, &cell_mcnt0, wtd_restore);
      register_cell (sfr, MCNT1, &cell_mcnt1, wtd_restore);
      register_cell (sfr, MA   , &cell_ma   , wtd_restore);
      register_cell (sfr, MB   , &cell_mb   , wtd_restore);
      register_cell (sfr, MC   , &cell_mc   , wtd_restore);
      register_cell (sfr, WDCON, &cell_wdcon, wtd_restore);
      register_cell (sfr, C1C  , &cell_c1c  , wtd_restore);
    }
  return 0;
}

t_mem
cl_uc390_hw::read (class cl_memory_cell *cell)
{
  if (cell == cell_exif)
    {
      if (ctm_ticks &&
          uc390->ticks->ticks >= ctm_ticks + 50 /*65535*/)
        {
          ctm_ticks = 0;
          cell->set (cell->get() | 0x08); /* set CKRDY */
        }
    }
  return cell->get();
}

void
cl_uc390_hw::write (class cl_memory_cell *cell, t_mem *val)
{
  if (cell == cell_dps)
    *val = (*val & 0xe5) | 0x04;
  else if (cell == cell_exif)
    {
      /* Bit 0 (BGS) is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = (*val & ~0x01) | (cell_exif->get() & 0x01);

      /* CKRDY and RGMD are read-only */
      *val = (*val & 0x0c) | (*val & ~0x0c);
    }
  else if (cell == cell_p4cnt)
    {
      /* P4CNT is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = cell_p4cnt->get();
      *val |= 0x80; /* always 1 */
    }
  else if (cell == cell_acon)
    {
      /* ACON is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = cell_acon->get();
      else
        {

          /* lockout: IDM1:IDM0 and SA can't be set at the same time */
            if ((cell_mcon->get() & 0xc0) == 0xc0) /* IDM1 and IDM0 set? */
              *val &= ~0x04; /* lockout SA */
        }
      *val |= 0xf8; /* always 1 */
    }
  else if (cell == cell_p5cnt)
    {
      /* Bits 0...2 are TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = (*val & ~0x07) | (cell_p5cnt->get() & 0x07);
    }
  else if (cell == cell_c0c)
    {
      /* Bit 3 (CRST) is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = (*val & ~0x08) | (cell_c0c->get() & 0x08);
    }
  else if (cell == cell_pmr)
    {
      /* fixme: check previous state */
      if ((*val & 0xd0) == 0x90) /* CD1:CD0 set to 10, CTM set */
        {
          ctm_ticks = uc390->ticks->ticks;
          cell_exif->set (cell_exif->get() & ~0x08); /* clear CKRDY */
        }
      else
        ctm_ticks = 0;
      *val |= 0x03; /* always 1 */
    }
  else if (cell == cell_mcon)
    {
      /* MCON is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = cell_mcon->get();
      else
        /* lockout: IDM1:IDM0 and SA can't be set at the same time */
        if ((cell_acon->get() & 0x04) == 0x04) /* SA set? */
          *val &= ~0xc0; /* lockout IDM1:IDM0 */
      *val |= 0x10; /* always 1 */
    }
  else if (cell == cell_ta)
    {
      if (*val == 0xAA)
        {
          timed_access_state = 1;
          timed_access_ticks = uc390->ticks->ticks;
        }
      else if (*val == 0x55 &&
               timed_access_state == 1 &&
               timed_access_ticks + 2*12 >= uc390->ticks->ticks) // fixme: 3 cycles
        {
          timed_access_state = 2;
          timed_access_ticks = uc390->ticks->ticks;
        }
      else
        timed_access_state = 0;
    }
  else if (cell == cell_cor)
    {
      /* COR is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = cell_cor->get();
    }
  else if (cell == cell_mcnt0)
    {
      ;
    }
  else if (cell == cell_mcnt1)
    {
      *val |= 0x0f; /* always 1 */
    }
  else if (cell == cell_ma)
    {
      ;
    }
  else if (cell == cell_mb)
    {
      ;
    }
  else if (cell == cell_mc)
    {
      ;
    }
  else if (cell == cell_wdcon)
    {
      /* Bits 0, 1, 3 and 6 are TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = (*val & ~0x4b) | (cell_wdcon->get() & 0x4b);
    }
  else if (cell == cell_c1c)
    {
      /* Bit 3 (CRST) is TA-protected */
      if (timed_access_state != 2 ||
          timed_access_ticks + 2*12 < uc390->ticks->ticks) // fixme: 3 cycles
        *val = (*val & ~0x08) | (cell_c1c->get() & 0x08);
    }
}

/*void
cl_uc390_hw::mem_cell_changed (class cl_m *mem, t_addr addr)
{
  class cl_m *sfr = uc->mem (MEM_SFR);

  if (mem && sfr && mem == sfr)
    switch (addr)
      {
        case DPS:   cell_dps   = sfr->get_cell (DPS);   break;
        case P4CNT: cell_p4cnt = sfr->get_cell (P4CNT); break;
        case EXIF:  cell_exif  = sfr->get_cell (EXIF);  break;
        case ACON:  cell_acon  = sfr->get_cell (ACON);  break;
        case P5CNT: cell_p5cnt = sfr->get_cell (P5CNT); break;
        case C0C:   cell_c0c   = sfr->get_cell (C0C);   break;
        case PMR:   cell_pmr   = sfr->get_cell (PMR);   break;
        case MCON:  cell_mcon  = sfr->get_cell (MCON);  break;
        case TA:    cell_ta    = sfr->get_cell (TA);    break;
        case COR:   cell_cor   = sfr->get_cell (COR);   break;
        case MCNT0: cell_mcnt0 = sfr->get_cell (MCNT0); break;
        case MCNT1: cell_mcnt1 = sfr->get_cell (MCNT1); break;
        case MA:    cell_ma    = sfr->get_cell (MA);    break;
        case MB:    cell_mb    = sfr->get_cell (MB);    break;
        case MC:    cell_mc    = sfr->get_cell (MC);    break;
        case WDCON: cell_wdcon = sfr->get_cell (WDCON); break;
        case C1C:   cell_c1c   = sfr->get_cell (C1C);   break;
      }
}*/

void
cl_uc390_hw::reset(void)
{
  ctm_ticks = 0;
  timed_access_state = 0;
}

void
cl_uc390_hw::print_info(class cl_console_base *con)
{
  int i;
  long l;

  i = sfr->get (EXIF);
  con->dd_printf ("%s"
                  " EXIF 0x%02x: IE5 %c IE4 %c IE3 %c IE2 %c CKRDY %c RGMD %c RGSL %c BGS %c\n",
                  id_string,
                  i,
                  (i & 0x80) ? '1' : '0',
                  (i & 0x40) ? '1' : '0',
                  (i & 0x20) ? '1' : '0',
                  (i & 0x10) ? '1' : '0',
                  (i & 0x08) ? '1' : '0',
                  (i & 0x04) ? '1' : '0',
                  (i & 0x02) ? '1' : '0',
                  (i & 0x01) ? '1' : '0');
  i = sfr->get (DPS);
  con->dd_printf ("\tDPS  0x%02x: ID1 %c ID0 %c TSL %c SEL %c\n",
                  i,
                  (i & 0x80) ? '1' : '0',
                  (i & 0x40) ? '1' : '0',
                  (i & 0x20) ? '1' : '0',
                  (i & 0x01) ? '1' : '0');
  l = sfr->get (DPX) * 256*256 +
      sfr->get (DPH) * 256 +
      sfr->get (DPL);
  con->dd_printf ("\tDPTR  0x%06x\n", l);
  l = sfr->get (DPX1) * 256*256 +
      sfr->get (DPH1) * 256 +
      sfr->get (DPL1);
  con->dd_printf ("\tDPTR1 0x%06x\n", l);
}

/* End of s51.src/uc390hw.cc */
