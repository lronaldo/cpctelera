/*
 * Simulator of microcontrollers (inst_gen.cc)
 * this code pulled into various parts
   of inst.cc with FUNC1 and FUNC2 defined as
   various operations to implement ADD, ADDC, ...
 *
 * Copyright (C) 1999,2002 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 * Other contributors include:
 *   Karl Bongers karl@turbobit.com,
 *   Johan Knol 
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

  switch (operands) {
    case REG_REG:
      if (code & 0x0800) {  /* word op */
        set_reg2( RI_F0,
                  FUNC2( reg2(RI_F0), reg2(RI_0F) )
                );
      } else {
        set_reg1( RI_F0,
                  FUNC1( reg1(RI_F0), reg1(RI_0F) )
                );
      }
    break;
    case REG_IREGINC :
    case REG_IREG:
    {
      short srcreg = reg2(RI_07);
      if (code & 0x0800) {  /* word op */
        set_reg2( RI_F0,
                  FUNC2( reg2(RI_F0),
                        get2(srcreg)
                      )
                );
      } else {
        set_reg1( RI_F0,
                  FUNC1( reg1(RI_F0),
                        get1(srcreg)
                      )
                );
      }
      if (operands == REG_IREGINC) {
        set_reg2(RI_07,  srcreg+1);
      }
    }
    break;
    case IREGINC_REG :
    case IREG_REG :
    {
      short addr = reg2(RI_07);
      if (code & 0x0800) {  /* word op */
        unsigned short wtmp, wtotal;
        wtmp = get2(addr);
        wtotal = FUNC2( wtmp, reg2(RI_F0) );
        store2(addr, wtotal);
      } else {
        unsigned char total;
        total = FUNC1( get1(addr), reg1(RI_F0) );
        store1(addr, total);
      }
      if (operands == IREGINC_REG) {
        set_reg2(RI_07, addr+1);
      }
    }
    break;

    case IREGOFF8_REG :
    case IREGOFF16_REG :
    {
      int offset;
      if (operands == IREGOFF8_REG) {
        offset = (int)((char) fetch());
      } else {
        offset = (int)((short)fetch2());
      }
      if (code & 0x0800) {  /* word op */
        t_mem addr = reg2(RI_07) + offset;
        unsigned short wtotal;
        wtotal = FUNC2( get2(addr), reg2(RI_F0) );
        store2(addr, wtotal);
      } else {
        t_mem addr = reg2(RI_07) + offset;
        unsigned char total;
        total = FUNC1( get1(addr), reg1(RI_F0) );
        store1(addr, total);
      }
    }
    break;

    case REG_IREGOFF8 :
    case REG_IREGOFF16 :
    {
      int offset;
      if (operands == REG_IREGOFF8) {
        offset = (int)((char) fetch());
      } else {
        offset = (int)((short)fetch2());
      }

      if (code & 0x0800) {  /* word op */
        set_reg2( RI_F0,
                  FUNC2( reg2(RI_F0),
                        get2(reg2(RI_07)+offset)
                      )
                );
      } else {
        set_reg1( RI_F0,
                  FUNC1( reg1(RI_F0),
                        get1(reg2(RI_07)+offset)
                      )
                );
      }
    }
    break;

    case DIRECT_REG :
    {
      int addr = ((code & 0x7) << 8) | fetch();
      if (code & 0x0800) {  /* word op */
        unsigned short wtmp = get_word_direct(addr);
        set_word_direct( addr,
                  FUNC2( wtmp, reg2(RI_F0) )
                );
      } else {
        unsigned char tmp = get_byte_direct(addr);
        set_byte_direct( addr,
                  FUNC1( tmp, reg1(RI_F0) )
                );
      }
    }
    break;

    case REG_DIRECT :
    {
      int addr = ((code & 0x7) << 8) | fetch();
      if (code & 0x0800) {  /* word op */
        set_reg2( RI_F0,
                  FUNC2( reg2(RI_F0),
                        get_word_direct(addr)
                      )
                );
      } else {
        set_reg1( RI_F0,
                  FUNC1( reg1(RI_F0),
                        get_byte_direct(addr)
                      )
                );
      }
    }
    break;

    case REG_DATA8 :
#if 0
      {
        unsigned char dat = fetch();
        unsigned char res;
        res = FUNC1( reg1(RI_F0), dat);
        set_reg1( RI_F0, res );
   printf("reg_data8 code=%x dat=%x, res=%x r=%x\n", code, dat, res, reg1( RI_F0) );
      }
#endif
      set_reg1( RI_F0, FUNC1( reg1(RI_F0), fetch()) );
    break;

    case REG_DATA16 :
      {
        unsigned short dat = fetch2();
        set_reg2( RI_F0, FUNC2( reg2(RI_F0), dat) );
      }
    break;

    case IREGINC_DATA8 :
    case IREG_DATA8 :
    {
      unsigned char total;
      unsigned char tmp;
      t_mem addr = reg2(RI_70);
      tmp = get1(addr);
      total = FUNC1(tmp, fetch() );
      store1(addr, total);
      if (operands == IREGINC_DATA8) {
        set_reg2(RI_70, addr+1);
      }
    }
    break;

    case IREGINC_DATA16 :
    case IREG_DATA16 :
    {
      unsigned short total;
      unsigned short tmp;
      t_mem addr = reg2(RI_70);
      tmp = get2(addr);
      total = FUNC2(tmp, fetch2() );
      store2(addr, total);
      if (operands == IREGINC_DATA16) {
        set_reg2(RI_70, addr+1);
      }
    }
    break;

    case IREGOFF8_DATA8 :
    case IREGOFF16_DATA8 :
    {
      unsigned short addr;
      int offset;
      unsigned char tmp;
      if (operands == IREGOFF8_DATA8) {
        offset = (int)((char) fetch());
      } else {
        offset = (int)((short)fetch2());
      }
      tmp = fetch();
      addr = reg2(RI_70);

      store1( addr+offset,
                  FUNC1( tmp,
                        get1(addr+offset)
                      )
                );
    }
    break;

    case IREGOFF8_DATA16 :
    case IREGOFF16_DATA16 :
    {
      unsigned short addr;
      int offset;
      unsigned short tmp;
      if (operands == IREGOFF8_DATA16) {
        offset = (int)((char) fetch());
      } else {
        offset = (int)((short)fetch2());
      }
      tmp = fetch2();
      addr = reg2(RI_70);

      store2( addr+offset,
                  FUNC2( tmp,
                        get2(addr+offset)
                      )
                );
    }
    break;

    case DIRECT_DATA8 :
    {
      int addr = ((code & 0x70) << 4) | fetch();
      unsigned char bdir = get_byte_direct(addr);
      unsigned char bdat = fetch();
      set_byte_direct( addr,  FUNC1( bdir, bdat) );
    }
    break;

    case DIRECT_DATA16 :
    {
      int addr = ((code & 0x70) << 4) | fetch();
      unsigned short wdir = get_word_direct(addr);
      unsigned short wdat = fetch2();
      set_word_direct( addr,  FUNC2( wdir, wdat) );
    }
    break;
  }

