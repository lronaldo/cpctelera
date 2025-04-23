/*-------------------------------------------------------------------------
   at89x51.h - register declarations for ATMEL 89x51 processors

   Copyright (C) 1999, Bernd Bartmann <bernd.bartmann AT gmail.com>

   Based on reg51.h by Sandeep Dutta sandeep.dutta AT usa.net
   KEIL C compatible definitions are included

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#ifndef AT89x51_H
#define AT89x51_H

#include <compiler.h>

/* BYTE addressable registers */
SFR(P0,		0x80);
SFR(SP,		0x81);
SFR(DPL,	0x82);
SFR(DPH,	0x83);
SFR(PCON,	0x87);
SFR(TCON,	0x88);
SFR(TMOD,	0x89);
SFR(TL0,	0x8A);
SFR(TL1,	0x8B);
SFR(TH0,	0x8C);
SFR(TH1,	0x8D);
SFR(P1,		0x90);
SFR(SCON,	0x98);
SFR(SBUF,	0x99);
SFR(P2,		0xA0);
SFR(IE,		0xA8);
SFR(P3,		0xB0);
SFR(IP,		0xB8);
SFR(PSW,	0xD0);
SFR(ACC,	0xE0);
SFR(A,		0xE0);
SFR(B,		0xF0);


/* BIT addressable registers */
/* P0 */
SBIT(P0_0,	0x80,	0);
SBIT(P0_1,	0x80,	1);
SBIT(P0_2,	0x80,	2);
SBIT(P0_3,	0x80,	3);
SBIT(P0_4,	0x80,	4);
SBIT(P0_5,	0x80,	5);
SBIT(P0_6,	0x80,	6);
SBIT(P0_7,	0x80,	7);

/* TCON */
SBIT(IT0,	0x88,	0);
SBIT(IE0,	0x88,	1);
SBIT(IT1,	0x88,	2);
SBIT(IE1,	0x88,	3);
SBIT(TR0,	0x88,	4);
SBIT(TF0,	0x88,	5);
SBIT(TR1,	0x88,	E);
SBIT(TF1,	0x88,	7);

/* P1 */
SBIT(P1_0,	0x90,	0);
SBIT(P1_1,	0x90,	1);
SBIT(P1_2,	0x90,	2);
SBIT(P1_3,	0x90,	3);
SBIT(P1_4,	0x90,	4);
SBIT(P1_5,	0x90,	5);
SBIT(P1_6,	0x90,	6);
SBIT(P1_7,	0x90,	7);

/* SCON */
SBIT(RI,	0x98,	0);
SBIT(TI,	0x98,	1);
SBIT(RB8,	0x98,	2);
SBIT(TB8,	0x98,	3);
SBIT(REN,	0x98,	4);
SBIT(SM2,	0x98,	5);
SBIT(SM1,	0x98,	6);
SBIT(SM0,	0x98,	7);

/* P2 */
SBIT(P2_0,	0xA0,	0);
SBIT(P2_1,	0xA0,	1);
SBIT(P2_2,	0xA0,	2);
SBIT(P2_3,	0xA0,	3);
SBIT(P2_4,	0xA0,	4);
SBIT(P2_5,	0xA0,	5);
SBIT(P2_6,	0xA0,	6);
SBIT(P2_7,	0xA0,	7);

/* IE */
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES,	0xA8,	4);
SBIT(EA,	0xA8,	7);

/* P3 */
SBIT(P3_0,	0xB0,	0);
SBIT(P3_1,	0xB0,	1);
SBIT(P3_2,	0xB0,	2);
SBIT(P3_3,	0xB0,	3);
SBIT(P3_4,	0xB0,	4);
SBIT(P3_5,	0xB0,	5);
SBIT(P3_6,	0xB0,	6);
SBIT(P3_7,	0xB0,	7);

SBIT(RXD,	0xB0,	0);
SBIT(TXD,	0xB0,	1);
SBIT(INT0,	0xB0,	2);
SBIT(INT1,	0xB0,	3);
SBIT(T0,	0xB0,	4);
SBIT(T1,	0xB0,	5);
SBIT(WR,	0xB0,	6);
SBIT(RD,	0xB0,	7);

/* IP */ 
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS,	0xB8,	4);

/* PSW */
SBIT(P,		0xD0,	0);
SBIT(FL,	0xD0,	1);
SBIT(OV,	0xD0,	2);
SBIT(RS0,	0xD0,	3);
SBIT(RS1,	0xD0,	4);
SBIT(F0,	0xD0,	5);
SBIT(AC,	0xD0,	6);
SBIT(CY,	0xD0,	7);


/* BIT definitions for bits that are not directly accessible */
/* PCON bits */
#define IDL             0x01
#define PD              0x02
#define GF0             0x04
#define GF1             0x08
#define SMOD            0x80

#define IDL_            0x01
#define PD_             0x02
#define GF0_            0x04
#define GF1_            0x08
#define SMOD_           0x80

/* TMOD bits */
#define M0_0            0x01
#define M1_0            0x02
#define C_T0            0x04
#define GATE0           0x08
#define M0_1            0x10
#define M1_1            0x20
#define C_T1            0x40
#define GATE1           0x80

#define M0_0_           0x01
#define M1_0_           0x02
#define C_T0_           0x04
#define GATE0_          0x08
#define M0_1_           0x10
#define M1_1_           0x20
#define C_T1_           0x40
#define GATE1_          0x80

#define T0_M0           0x01
#define T0_M1           0x02
#define T0_CT           0x04
#define T0_GATE         0x08
#define T1_M0           0x10
#define T1_M1           0x20
#define T1_CT           0x40
#define T1_GATE         0x80

#define T0_M0_          0x01
#define T0_M1_          0x02
#define T0_CT_          0x04
#define T0_GATE_        0x08
#define T1_M0_          0x10
#define T1_M1_          0x20
#define T1_CT_          0x40
#define T1_GATE_        0x80

#define T0_MASK         0x0F
#define T1_MASK         0xF0

#define T0_MASK_        0x0F
#define T1_MASK_        0xF0


/* Interrupt numbers: address = (number * 8) + 3 */
#define IE0_VECTOR      0       /* 0x03 external interrupt 0 */
#define TF0_VECTOR      1       /* 0x0b timer 0 */
#define IE1_VECTOR      2       /* 0x13 external interrupt 1 */
#define TF1_VECTOR      3       /* 0x1b timer 1 */
#define SI0_VECTOR      4       /* 0x23 serial port 0 */
 
#endif
