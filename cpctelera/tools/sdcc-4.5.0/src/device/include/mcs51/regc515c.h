/*---------------------------------------------------------------------------
   regc515c.h - This file contains definitions for the builtin CAN-Bus
   Controller of the Siemens c515c controller

   Copyright (C) 2005, Maarten Brock, sourceforge.brock@dse.nl

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

#ifndef _REGC515C_H
#define _REGC515C_H

#include <compiler.h>

/* define CPU_CLK_10MHZ or CPU_CLK_8MHZ to select the right values for */
/* the bit timing registers */

#define CPU_CLK_10MHZ

/* address of can controller in xmem */
#define CAN_CTRL	0xf700

/* size of message buffer including 1 dummy byte at end */
#define CAN_MSG_SZ	0x10

/* register offset definitions */
#define CR	0
#define SR	1
#define IR	2

#define BT_0	4
#define BT_1	5
#define GMS_0	6
#define GMS_1	7
#define GME_0	8
#define GME_1	9
#define GME_2	0xa
#define GME_3	0xb
#define MSG15MSK_0	0xc
#define MSG15MSK_1	0xd
#define MSG15MSK_2	0xe
#define MSG15MSK_3	0xf

/* register offsets  in message buffer */
#define MCR_0	0
#define MCR_1	1
#define ARB_0	2
#define ARB_1	3
#define ARB_2	4
#define ARB_3	5
#define MCFG	6
/* beginning of message data */
#define DATA	7

/* bits in cntr_x registers */
#define MSGVAL	0x80
#define TXIE	0x20
#define RXIE	0x8
#define INTPND	0x2
#define RMTPND	0x80
#define TXRQST	0x20
#define MSGLST	0x8
#define CPUUPD	0x8
#define NEWDAT	0x2

/* macros for setting and resetting above bits, see Siemens documentation */
#define MCR_BIT_SET(p,x) ((p) = (0xff & ~((x) >> 1)))
#define MCR_BIT_RES(p,x) ((p) = (0xff & ~(x)))

/* direction = transmit in mcfg */
#define DIR_TRANSMIT	0x8

/* constants for bit timing registers */
/* 8 MHZ */
#ifdef CPU_CLK_8MHZ
#define BT_0_125K	0x3
#define BT_1_125K	0x1c
#define BT_0_250K	0x1
#define BT_1_250K	0x1c
#define BT_0_500K	0x0
#define BT_1_500K	0x1c
#define BT_0_1M		0x0
#define BT_1_1M		0x14
#endif
/* dito, 10 MHZ */
#ifdef CPU_CLK_10MHZ
#define BT_0_125K	0x3
#define BT_1_125K	0x1c
#define BT_0_250K	0x1
#define BT_1_250K	0x1c
#define BT_0_500K	0x0
#define BT_1_500K	0x2f
#define BT_0_1M		0x0
#define BT_1_1M		0x25
#endif

/* Control register bits */

#define	CINIT	0x1
#define IE	0x2
#define SIE	0x4
#define EIE	0x8

#define CCE	0x40

/* status register bits */
#define	LEC0	0x1
#define	LEC1	0x2
#define	LEC2	0x4
#define	TXOK	0x8
#define	RXOK	0x10
#define	WAKE	0x20
#define	WARN	0x40
#define	BOFF	0x80


typedef struct can_msg
{
    unsigned char mcr_0;
    unsigned char mcr_1;
    unsigned char arb_0;
    unsigned char arb_1;
    unsigned char arb_2;
    unsigned char arb_3;
    unsigned char mcfg;
    unsigned char data_bytes[8];
    unsigned char dummy;
} *can_msgp;

__xdata __at CAN_CTRL struct
{
    unsigned	char	cr;
    unsigned	char	sr;
    unsigned	char	ir;
    unsigned	char	res0;
    unsigned	char	bt_0;
    unsigned	char	bt_1;
    unsigned	char	gms_0;
    unsigned	char	gms_1;
    unsigned	char	gme_0;
    unsigned	char	gme_1;
    unsigned	char	gme_2;
    unsigned	char	gme_3;
    unsigned	char	msg15msk_0;
    unsigned	char	msg15msk_1;
    unsigned	char	msg15msk_2;
    unsigned	char	msg15msk_3;
    struct	can_msg msgbufs[15];
} can_ctrl;

/* Byte registers in numerical order */

SFR(P0,		0x80);
SFR(SP,		0x81);
SFR(DPL,	0x82);
SFR(DPH,	0x83);
SFR(WDTREL,	0x86);
SFR(PCON,	0x87);
SFR(TCON,	0x88);
SFR(PCON1,	0x88);
SFR(TMOD,	0x89);
SFR(TL0,	0x8A);
SFR(TL1,	0x8B);
SFR(TH0,	0x8C);
SFR(TH1,	0x8D);
SFR(P1,		0x90);
SFR(XPAGE,	0x91);
SFR(DPSEL,	0x92);
SFR(SSCCON,	0x93);
SFR(STB,	0x94);
SFR(SRB,	0x95);
SFR(SSCMOD,	0x96);
SFR(SCON,	0x98);
SFR(SBUF,	0x99);
SFR(IEN2,	0x9A);
SFR(P2,		0xA0);
SFR(IEN0,	0xA8);
SFR(IP0,	0xA9);
SFR(SRELL,	0xAA);
SFR(SCF,	0xAB);
SFR(SCIEN,	0xAC);
SFR(P3,		0xB0);
SFR(SYSCON,	0xB1);
SFR(IEN1,	0xB8);
SFR(IP1,	0xB9);
SFR(SRELH,	0xBA);
SFR(IRCON,	0xC0);
SFR(CCEN,	0xC1);
SFR(CCL1,	0xC2);
SFR(CCH1,	0xC3);
SFR(CCL2,	0xC4);
SFR(CCH2,	0xC5);
SFR(CCL3,	0xC6);
SFR(CCH3,	0xC7);
SFR(T2CON,	0xC8);
SFR(CRCL,	0xCA);
SFR(CRCH,	0xCB);
SFR(TL2,	0xCC);
SFR(TH2,	0xCD);
SFR(PSW,	0xD0);
SFR(ADCON0,	0xD8);
SFR(ADDATH,	0xD9);
SFR(ADDATL,	0xDA);
SFR(P6,		0xDB);
SFR(ADCON1,	0xDC);
SFR(ACC,	0xE0);
SFR(P4,		0xE8);
SFR(B,		0xF0);
SFR(P5,		0xF8);
SFR(DIR5,	0xF8);
SFR(P7,		0xFA);


/* defining bits in SFR P0 */
SBIT(P0_0,		0x80,	0);
SBIT(P0_1,		0x80,	1);
SBIT(P0_2,		0x80,	2);
SBIT(P0_3,		0x80,	3);
SBIT(P0_4,		0x80,	4);
SBIT(P0_5,		0x80,	5);
SBIT(P0_6,		0x80,	6);
SBIT(P0_7,		0x80,	7);



/* defining bits in SFR PCON1 */
SBIT(IT0,		0x88,	0);
SBIT(IE0,		0x88,	1);
SBIT(IT1,		0x88,	2);
SBIT(IE1,		0x88,	3);
SBIT(TR0,		0x88,	4);
SBIT(TF0,		0x88,	5);
SBIT(TR1,		0x88,	6);
SBIT(TF1,		0x88,	7);
SBIT(EWPD,		0x88,	7);



/* defining bits in SFR P1 */
SBIT(P1_0,		0x90,	0);
SBIT(INT3,		0x90,	0);
SBIT(P1_1,		0x90,	1);
SBIT(INT4,		0x90,	1);
SBIT(P1_2,		0x90,	2);
SBIT(INT5,		0x90,	2);
SBIT(P1_3,		0x90,	3);
SBIT(INT6,		0x90,	3);
SBIT(P1_4,		0x90,	4);
SBIT(INT2,		0x90,	4);
SBIT(P1_5,		0x90,	5);
SBIT(T2EX,		0x90,	5);
SBIT(P1_6,		0x90,	6);
SBIT(CLKOUT,	0x90,	6);
SBIT(P1_7,		0x90,	7);
SBIT(T2,		0x90,	7);



/* defining bits in SFR SCON */
SBIT(RI,		0x98,	0);
SBIT(TI,		0x98,	1);
SBIT(RB8,		0x98,	2);
SBIT(TB8,		0x98,	3);
SBIT(REN,		0x98,	4);
SBIT(SM2,		0x98,	5);
SBIT(SM1,		0x98,	6);
SBIT(SM0,		0x98,	7);



/* defining bits in SFR P2 */
SBIT(P2_0,		0xa0,	0);
SBIT(P2_1,		0xa0,	1);
SBIT(P2_2,		0xa0,	2);
SBIT(P2_3,		0xa0,	3);
SBIT(P2_4,		0xa0,	4);
SBIT(P2_5,		0xa0,	5);
SBIT(P2_6,		0xa0,	6);
SBIT(P2_7,		0xa0,	7);



/* defining bits in SFR IEN0 */
SBIT(EX0,		0xa8,	0);
SBIT(ET0,		0xa8,	1);
SBIT(EX1,		0xa8,	2);
SBIT(ET1,		0xa8,	3);
SBIT(ES,		0xa8,	4);
SBIT(ET2,		0xa8,	5);
SBIT(WDT,		0xa8,	6);
SBIT(EA,		0xa8,	7);



/* defining bits in SFR P3 */
SBIT(P3_0,		0xb0,	0);
SBIT(RXD,		0xb0,	0);
SBIT(P3_1,		0xb0,	1);
SBIT(TXD,		0xb0,	1);
SBIT(P3_2,		0xb0,	2);
SBIT(INT0,		0xb0,	2);
SBIT(P3_3,		0xb0,	3);
SBIT(INT1,		0xb0,	3);
SBIT(P3_4,		0xb0,	4);
SBIT(T0,		0xb0,	4);
SBIT(P3_5,		0xb0,	5);
SBIT(T1,		0xb0,	5);
SBIT(P3_6,		0xb0,	6);
SBIT(WR,		0xb0,	6);
SBIT(P3_7,		0xb0,	7);
SBIT(RD,		0xb0,	7);



/* defining bits in SFR IEN1 */
SBIT(EADC,		0xb8,	0);
SBIT(EX2,		0xb8,	1);
SBIT(EX3,		0xb8,	2);
SBIT(EX4,		0xb8,	3);
SBIT(EX5,		0xb8,	4);
SBIT(EX6,		0xb8,	5);
SBIT(SWDT,		0xb8,	6);
SBIT(EXEN2,		0xb8,	7);



/* defining bits in SFR IRCON */
SBIT(IADC,		0xc0,	0);
SBIT(IEX2,		0xc0,	1);
SBIT(IEX3,		0xc0,	2);
SBIT(IEX4,		0xc0,	3);
SBIT(IEX5,		0xc0,	4);
SBIT(IEX6,		0xc0,	5);
SBIT(TF2,		0xc0,	6);
SBIT(EXF2,		0xc0,	7);



/* defining bits in SFR T2CON */
SBIT(T2I0,		0xc8,	0);
SBIT(T2I1,		0xc8,	1);
SBIT(T2CM,		0xc8,	2);
SBIT(T2R0,		0xc8,	3);
SBIT(T2R1,		0xc8,	4);
SBIT(I2FR,		0xc8,	5);
SBIT(I3FR,		0xc8,	6);
SBIT(T2PS,		0xc8,	7);



/* defining bits in SFR PSW */
SBIT(P,			0xd0,	0);
SBIT(F1,		0xd0,	1);
SBIT(OV,		0xd0,	2);
SBIT(RS0,		0xd0,	3);
SBIT(RS1,		0xd0,	4);
SBIT(F0,		0xd0,	5);
SBIT(AC,		0xd0,	6);
SBIT(CY,		0xd0,	7);



/* defining bits in SFR ADCON0 */
SBIT(MX0,		0xd8,	0);
SBIT(MX1,		0xd8,	1);
SBIT(MX2,		0xd8,	2);
SBIT(ADM,		0xd8,	3);
SBIT(BSY,		0xd8,	4);
SBIT(ADEX,		0xd8,	5);
SBIT(CLK,		0xd8,	6);
SBIT(BD,		0xd8,	7);



/* defining bits in SFR ACC */
SBIT(ACC_0,		0xe0,	0);
SBIT(ACC_1,		0xe0,	1);
SBIT(ACC_2,		0xe0,	2);
SBIT(ACC_3,		0xe0,	3);
SBIT(ACC_4,		0xe0,	4);
SBIT(ACC_5,		0xe0,	5);
SBIT(ACC_6,		0xe0,	6);
SBIT(ACC_7,		0xe0,	7);



/* defining bits in SFR P4 */
SBIT(P4_0,		0xe8,	0);
SBIT(ADST,		0xe8,	0);
SBIT(P4_1,		0xe8,	1);
SBIT(SCLK,		0xe8,	1);
SBIT(P4_2,		0xe8,	2);
SBIT(SRI,		0xe8,	2);
SBIT(P4_3,		0xe8,	3);
SBIT(STO,		0xe8,	3);
SBIT(P4_4,		0xe8,	4);
SBIT(SLS,		0xe8,	4);
SBIT(P4_5,		0xe8,	5);
SBIT(INT8,		0xe8,	5);
SBIT(P4_6,		0xe8,	6);
SBIT(TXDC,		0xe8,	6);
SBIT(P4_7,		0xe8,	7);
SBIT(RXDC,		0xe8,	7);



/* defining bits in SFR B */
SBIT(B_0,		0xf0,	0);
SBIT(B_1,		0xf0,	1);
SBIT(B_2,		0xf0,	2);
SBIT(B_3,		0xf0,	3);
SBIT(B_4,		0xf0,	4);
SBIT(B_5,		0xf0,	5);
SBIT(B_6,		0xf0,	6);
SBIT(B_7,		0xf0,	7);



/* defining bits in SFR DIR5 */
SBIT(P5_0,		0xf8,	0);
SBIT(P5_1,		0xf8,	9);
SBIT(P5_2,		0xf8,	2);
SBIT(P5_3,		0xf8,	3);
SBIT(P5_4,		0xf8,	4);
SBIT(P5_5,		0xf8,	5);
SBIT(P5_6,		0xf8,	6);
SBIT(P5_7,		0xf8,	7);
SBIT(DIR5_0,	0xf8,	0);
SBIT(DIR5_1,	0xf8,	1);
SBIT(DIR5_2,	0xf8,	2);
SBIT(DIR5_3,	0xf8,	3);
SBIT(DIR5_4,	0xf8,	4);
SBIT(DIR5_5,	0xf8,	5);
SBIT(DIR5_6,	0xf8,	6);
SBIT(DIR5_7,	0xf8,	7);

#endif /* _REGC515C_H */
