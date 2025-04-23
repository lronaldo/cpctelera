/*-------------------------------------------------------------------------
   sab80515.h - Register Declarations for SIEMENS/INFINEON SAB 80515 Processor
   based on reg51.h by Sandeep Dutta sandeep.dutta@usa.net
   KEIL C compatible definitions are included

   Copyright (C) 2005, Bela Torok / bela.torok@kssg.ch

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

#ifndef SAB80515_H
#define SAB80515_H

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
SFR(IEN0,	0xA8); /* as called by Siemens */
SFR(IP0,	0xA9); /* interrupt priority register - SAB80515 specific */
SFR(P3,		0xB0);
SFR(IEN1,	0xB8); /* interrupt enable register - SAB80515 specific */
SFR(IP1,	0xB9); /* interrupt priority register as called by Siemens */
SFR(IRCON,	0xC0); /* interrupt control register - SAB80515 specific */
SFR(CCEN,	0xC1); /* compare/capture enable register */
SFR(CCL1,	0xC2); /* compare/capture register 1, low byte */
SFR(CCH1,	0xC3); /* compare/capture register 1, high byte */
SFR(CCL2,	0xC4); /* compare/capture register 2, low byte */
SFR(CCH2,	0xC5); /* compare/capture register 2, high byte */
SFR(CCL3,	0xC6); /* compare/capture register 3, low byte */
SFR(CCH3,	0xC7); /* compare/capture register 3, high byte */
SFR(T2CON,	0xC8);
SFR(CRCL,	0xCA); /* compare/reload/capture register, low byte */
SFR(CRCH,	0xCB); /* compare/reload/capture register, high byte */
SFR(TL2,	0xCC);
SFR(TH2,	0xCD);
SFR(PSW,	0xD0);
SFR(ADCON,	0xD8); /* A/D-converter control register */
SFR(ADDAT,	0xD9); /* A/D-converter data register */
SFR(DAPR,	0xDA); /* D/A-converter program register */
SFR(P6,		0xDB); /* Port 6 - SAB80515 specific */
SFR(ACC,	0xE0);
SFR(A,		0xE0);
SFR(P4,		0xE8); /* Port 4 - SAB80515 specific */
SFR(B,		0xF0);
SFR(P5,		0xF8); /* Port 5 - SAB80515 specific */


/* BIT addressable registers */
/* P0 */
SBIT(P0_0,		0x80,	0);
SBIT(P0_1,		0x80,	1);
SBIT(P0_2,		0x80,	2);
SBIT(P0_3,		0x80,	3);
SBIT(P0_4,		0x80,	4);
SBIT(P0_5,		0x80,	5);
SBIT(P0_6,		0x80,	6);
SBIT(P0_7,		0x80,	7);

/* TCON */
SBIT(IT0,		0x88,	0);
SBIT(IE0,		0x88,	1);
SBIT(IT1,		0x88,	2);
SBIT(IE1,		0x88,	3);
SBIT(TR0,		0x88,	4);
SBIT(TF0,		0x88,	5);
SBIT(TR1,		0x88,	6);
SBIT(TF1,		0x88,	7);

/* P1 */
SBIT(P1_0,		0x90,	0);
SBIT(P1_1,		0x90,	1);
SBIT(P1_2,		0x90,	2);
SBIT(P1_3,		0x90,	3);
SBIT(P1_4,		0x90,	4);
SBIT(P1_5,		0x90,	5);
SBIT(P1_6,		0x90,	6);
SBIT(P1_7,		0x90,	7);

SBIT(INT3_CC0,	0x90,	0); /* P1 alternate functions - SAB80515 specific */
SBIT(INT4_CC1,	0x90,	1);
SBIT(INT5_CC2,	0x90,	2);
SBIT(INT6_CC3,	0x90,	3);
SBIT(INT2,		0x90,	4);
SBIT(T2EX,		0x90,	5);
SBIT(CLKOUT,	0x90,	6);
SBIT(T2,		0x90,	7);

/* SCON */
SBIT(RI,		0x98,	0);
SBIT(TI,		0x98,	1);
SBIT(RB8,		0x98,	2);
SBIT(TB8,		0x98,	3);
SBIT(REN,		0x98,	4);
SBIT(SM2,		0x98,	5);
SBIT(SM1,		0x98,	6);
SBIT(SM0,		0x98,	7);

/* P2 */
SBIT(P2_0,		0xA0,	0);
SBIT(P2_1,		0xA0,	1);
SBIT(P2_2,		0xA0,	2);
SBIT(P2_3,		0xA0,	3);
SBIT(P2_4,		0xA0,	4);
SBIT(P2_5,		0xA0,	5);
SBIT(P2_6,		0xA0,	6);
SBIT(P2_7,		0xA0,	7);

/* IEN0 */
SBIT(EX0,		0xA8,	0);
SBIT(ET0,		0xA8,	1);
SBIT(EX1,		0xA8,	2);
SBIT(ET1,		0xA8,	3);
SBIT(ES,		0xA8,	4);
SBIT(ET2,		0xA8,	5);
SBIT(WDT,		0xA8,	6); /* watchdog timer reset - SAB80515 specific */
SBIT(EA,		0xA8,	7);

SBIT(EAL,		0xA8,	7); /* EA as called by Siemens */

/* P3 */
SBIT(P3_0,		0xB0,	0);
SBIT(P3_1,		0xB0,	1);
SBIT(P3_2,		0xB0,	2);
SBIT(P3_3,		0xB0,	3);
SBIT(P3_4,		0xB0,	4);
SBIT(P3_5,		0xB0,	5);
SBIT(P3_6,		0xB0,	6);
SBIT(P3_7,		0xB0,	7);

SBIT(RXD,		0xB0,	0);
SBIT(TXD,		0xB0,	1);
SBIT(INT0,		0xB0,	2);
SBIT(INT1,		0xB0,	3);
SBIT(T0,		0xB0,	4);
SBIT(T1,		0xB0,	5);
SBIT(WR,		0xB0,	6);
SBIT(RD,		0xB0,	7);

/* IEN1 */
SBIT(EADC,		0xB8,	0); /* A/D converter interrupt enable */
SBIT(EX2,		0xB8,	1);
SBIT(EX3,		0xB8,	2);
SBIT(EX4,		0xB8,	3);
SBIT(EX5,		0xB8,	4);
SBIT(EX6,		0xB8,	5);
SBIT(SWDT,		0xB8,	6); /* watchdog timer start/reset */
SBIT(EXEN2,		0xB8,	7); /* timer2 external reload interrupt enable */

/* IRCON */
SBIT(IADC,		0xC0,	0); /* A/D converter irq flag */
SBIT(IEX2,		0xC0,	1); /* external interrupt edge detect flag */
SBIT(IEX3,		0xC0,	2);
SBIT(IEX4,		0xC0,	3);
SBIT(IEX5,		0xC0,	4);
SBIT(IEX6,		0xC0,	5);
SBIT(TF2,		0xC0,	6); /* timer 2 owerflow flag  */
SBIT(EXF2,		0xC0,	7); /* timer2 reload flag */

/* T2CON */
SBIT(T2CON_0,	0xC8,	0);
SBIT(T2CON_1,	0xC8,	1);
SBIT(T2CON_2,	0xC8,	2);
SBIT(T2CON_3,	0xC8,	3);
SBIT(T2CON_4,	0xC8,	4);
SBIT(T2CON_5,	0xC8,	5);
SBIT(T2CON_6,	0xC8,	6);
SBIT(T2CON_7,	0xC8,	7);

SBIT(T2I0,		0xC8,	0);
SBIT(T2I1,		0xC8,	1);
SBIT(T2CM,		0xC8,	2);
SBIT(T2R0,		0xC8,	3);
SBIT(T2R1,		0xC8,	4);
SBIT(I2FR,		0xC8,	5);
SBIT(I3FR,		0xC8,	6);
SBIT(T2PS,		0xC8,	7);


/* PSW */
SBIT(P,			0xD0,	0);
SBIT(FL,		0xD0,	1);
SBIT(OV,		0xD0,	2);
SBIT(RS0,		0xD0,	3);
SBIT(RS1,		0xD0,	4);
SBIT(F0,		0xD0,	5);
SBIT(AC,		0xD0,	6);
SBIT(CY,		0xD0,	7);

SBIT(F1,		0xD0,	1);

/* ADCON */
SBIT(MX0,		0xD8,	0);
SBIT(MX1,		0xD8,	1);
SBIT(MX2,		0xD8,	2);
SBIT(ADM,		0xD8,	3);
SBIT(BSY,		0xD8,	4);

SBIT(CLK,		0xD8,	6);
SBIT(BD,		0xD8,	7);

/* A */
SBIT(AREG_F0,	0xA0,	0);
SBIT(AREG_F1,	0xA0,	1);
SBIT(AREG_F2,	0xA0,	2);
SBIT(AREG_F3,	0xA0,	3);
SBIT(AREG_F4,	0xA0,	4);
SBIT(AREG_F5,	0xA0,	5);
SBIT(AREG_F6,	0xA0,	6);
SBIT(AREG_F7,	0xA0,	7);

/* P4 */
SBIT(P4_0,		0xE8,	0);
SBIT(P4_1,		0xE8,	1);
SBIT(P4_2,		0xE8,	2);
SBIT(P4_3,		0xE8,	3);
SBIT(P4_4,		0xE8,	4);
SBIT(P4_5,		0xE8,	5);
SBIT(P4_6,		0xE8,	6);
SBIT(P4_7,		0xE8,	7);

/* B */
SBIT(BREG_F0,	0xF0,	0);
SBIT(BREG_F1,	0xF0,	1);
SBIT(BREG_F2,	0xF0,	2);
SBIT(BREG_F3,	0xF0,	3);
SBIT(BREG_F4,	0xF0,	4);
SBIT(BREG_F5,	0xF0,	5);
SBIT(BREG_F6,	0xF0,	6);
SBIT(BREG_F7,	0xF0,	7);

/* P5 */
SBIT(P5_0,		0xF8,	0);
SBIT(P5_1,		0xF8,	1);
SBIT(P5_2,		0xF8,	2);
SBIT(P5_3,		0xF8,	3);
SBIT(P5_4,		0xF8,	4);
SBIT(P5_5,		0xF8,	5);
SBIT(P5_6,		0xF8,	6);
SBIT(P5_7,		0xF8,	7);

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

/* T2MOD bits */
#define DCEN            0x01
#define T2OE            0x02

#define DCEN_           0x01
#define T2OE_           0x02

/* WMCON bits */
#define WMCON_WDTEN     0x01
#define WMCON_WDTRST    0x02
#define WMCON_DPS       0x04
#define WMCON_EEMEN     0x08
#define WMCON_EEMWE     0x10
#define WMCON_PS0       0x20
#define WMCON_PS1       0x40
#define WMCON_PS2       0x80

/* SPCR-SPI bits */
#define SPCR_SPR0       0x01
#define SPCR_SPR1       0x02
#define SPCR_CPHA       0x04
#define SPCR_CPOL       0x08
#define SPCR_MSTR       0x10
#define SPCR_DORD       0x20
#define SPCR_SPE        0x40
#define SPCR_SPIE       0x80

/* SPSR-SPI bits */
#define SPSR_WCOL       0x40
#define SPSR_SPIF       0x80

/* SPDR-SPI bits */
#define SPDR_SPD0       0x10
#define SPDR_SPD1       0x20
#define SPDR_SPD2       0x40
#define SPDR_SPD3       0x80
#define SPDR_SPD4       0x10
#define SPDR_SPD5       0x20
#define SPDR_SPD6       0x40
#define SPDR_SPD7       0x80

/* Interrupt numbers: address = (number * 8) + 3 */
#define IE0_VECTOR      0       /* 0x03 external interrupt 0 */
#define TF0_VECTOR      1       /* 0x0b timer 0 */
#define IE1_VECTOR      2       /* 0x13 external interrupt 1 */
#define TF1_VECTOR      3       /* 0x1b timer 1 */
#define SI0_VECTOR      4       /* 0x23 serial port 0 */
#define TF2_VECTOR      5       /* 0x2B timer 2 */
#define EX2_VECTOR      5       /* 0x2B external interrupt 2 */

#define IADC_VECTOR     8       /* 0x43 A/D converter interrupt */
#define IEX2_VECTOR     9       /* 0x4B external interrupt 2 */
#define IEX3_VECTOR    10       /* 0x53 external interrupt 3 */
#define IEX4_VECTOR    11       /* 0x5B external interrupt 4 */
#define IEX5_VECTOR    12       /* 0x63 external interrupt 5 */
#define IEX6_VECTOR    13       /* 0x6B external interrupt 6 */

#endif

