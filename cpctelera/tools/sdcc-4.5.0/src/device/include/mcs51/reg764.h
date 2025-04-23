/*-------------------------------------------------------------------------
   reg764 - register Declarations for 87C764

   Copyright (C) 2005, Robert Lacoste <robert_lacoste AT yahoo.fr>
     based upon reg51.h written by Sandeep Dutta
     Registers are taken from the Phillips Semiconductor

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

#ifndef REGC764_H
#define REGC764_H

#include <compiler.h>

/*  Special Function Registers  */

SFR(P0,		0x80);   // Port 0
SFR(SP,		0x81);   // Stack Pointer
SFR(DPL,	0x82);   // Data Pointer Low
SFR(DPH,	0x83);   // Data Pointer High
SFR(P0M1,	0x84);   // Port 0 output mode 1
SFR(P0M2,	0x85);   // Port 0 output mode 2
SFR(KBI,	0x86);   // Keyboard interrupt
SFR(PCON,	0x87);   // Power Control
SFR(TCON,	0x88);   // Timer Control
SFR(TMOD,	0x89);   // Timer Mode
SFR(TL0,	0x8A);   // Timer Low 0
SFR(TL1,	0x8B);   // Timer Low 1
SFR(TH0,	0x8C);   // Timer High 0
SFR(TH1,	0x8D);   // Timer High 1

SFR(P1,		0x90);   // Port 1
SFR(P1M1,	0x91);   // Port 1 output mode 1
SFR(P1M2,	0x92);   // Port 1 output mode 2
SFR(DIVM,	0x95);   // CPU clock divide by N control
SFR(SCON,	0x98);   // Serial Control
SFR(SBUF,	0x99);   // Serial Data Buffer

SFR(P2,		0xA0);   // Port 2
SFR(AUXR1,	0xA2);   // Auxilliary 1 (not available on 80C51FA/87C51Fx)
SFR(P2M1,	0xA4);   // Port 2 output mode 1
SFR(P2M2,	0xA5);   // Port 2 output mode 2
SFR(WDRST,	0xA6);   // Watchdog reset register
SFR(WDCON,	0xA7);   // Watchdog control register
SFR(IEN0,	0xA8);   // Interrupt Enable 0
SFR(SADDR,	0xA9);   // Serial slave Address
SFR(CMP1,	0xAC);   // Comparator 1 control register
SFR(CMP2,	0xAD);   // Comparator 2 control register

SFR(IP0H,	0xB7);   // Interrupt Priority 0 High
SFR(IP0,	0xB8);   // Interrupt Priority 0
SFR(SADEN,	0xB9);   // Serial slave Address Mask

SFR(I2CFG,	0xC8);   // I2C configuration register

SFR(PSW,	0xD0);   // Program Status Word
SFR(I2CON,	0xD8);   // I2C control register
SFR(I2DAT,	0xD9);   // I2C data register

SFR(ACC,	0xE0);   // Accumulator
SFR(IEN1,	0xE8);   // Interrupt enable 1

SFR(B,		0xF0);   // B Register
SFR(PT0AD,	0xF6);   // Port 0 digital input disable
SFR(IP1H,	0xF7);   // Interrupt Priority 1 High
SFR(IP1,	0xF8);   // Interrupt Priority 1


/*  Bit Addressable Registers  */

/*  P0    */
SBIT(P0_0,		0x80,	0); // Also CMP2
SBIT(P0_1,		0x80,	1); // Also CIN2B
SBIT(P0_2,		0x80,	2); // Also CIN2A
SBIT(P0_3,		0x80,	3); // Also CIN1B
SBIT(P0_4,		0x80,	4); // Also CIN1A
SBIT(P0_5,		0x80,	5); // Also CMPREF
SBIT(P0_6,		0x80,	6); // Also CMP1
SBIT(P0_7,		0x80,	7); // Also T1

/*  TCON  */
SBIT(IT0,		0x88,	0); // External Interrupt 0 Type
SBIT(IE0,		0x88,	1); // External Interrupt 0 Edge Flag
SBIT(IT1,		0x88,	2); // External Interrupt 1 Type
SBIT(IE1,		0x88,	3); // External Interrupt 1 Edge Flag
SBIT(TR0,		0x88,	4); // Timer 0 Run Control
SBIT(TF0,		0x88,	5); // Timer 0 Overflow Flag
SBIT(TR1,		0x88,	6); // Timer 1 Run Control
SBIT(TF1,		0x88,	7); // Timer 1 Overflow Flag

/*  P1 */
SBIT(P1_0,		0x90,	0); // Also TxD
SBIT(P1_1,		0x90,	1); // Also RxD
SBIT(P1_2,		0x90,	2); // Also T0
SBIT(P1_3,		0x90,	3); // Also INT0
SBIT(P1_4,		0x90,	4); // Also INT1
SBIT(P1_5,		0x90,	5); // Also RST
SBIT(P1_6,		0x90,	6);
SBIT(P1_7,		0x90,	7);

/*  SCON  */
SBIT(RI,		0x98,	0); // Receive Interrupt Flag
SBIT(TI,		0x98,	1); // Transmit Interrupt Flag
SBIT(RB8,		0x98,	2); // Receive Bit 8
SBIT(TB8,		0x98,	3); // Transmit Bit 8
SBIT(REN,		0x98,	4); // Receiver Enable
SBIT(SM2,		0x98,	5); // Serial Mode Control Bit 2
SBIT(SM1,		0x98,	6); // Serial Mode Control Bit 1
SBIT(SM0,		0x98,	7); // Serial Mode Control Bit 0

/*  P2    */
SBIT(P2_0,		0xA0,	0); // Also X2
SBIT(P2_1,		0xA0,	1); // Also X1

/*  IEN0 */
SBIT(EX0,		0xA8,	0); // External Interrupt 0 Enable
SBIT(ET0,		0xA8,	1); // Timer 0 Interrupt Enable
SBIT(EX1,		0xA8,	2); // External Interrupt 1 Enable
SBIT(ET1,		0xA8,	3); // Timer 1 Interrupt Enable
SBIT(ES,		0xA8,	4); // Serial Port Interrupt Enable
SBIT(EBO,		0xA8,	5); // Brownout Interrupt Enable
SBIT(EWD,		0xA8,	6); // Watchdog Interrupt Enable
SBIT(EA,		0xA8,	7); // Global Interrupt Enable

/*  IP0   */
SBIT(PX0,		0xB8,	0); // External Interrupt 0 Priority
SBIT(PT0,		0xB8,	1); // Timer 0 Interrupt Priority
SBIT(PX1,		0xB8,	2); // External Interrupt 1 Priority
SBIT(PT1,		0xB8,	3); // Timer 1 Interrupt Priority
SBIT(PS,		0xB8,	4); // Serial Port Interrupt Priority
SBIT(PBO,		0xB8,	5); // Brownout Interrupt Priority
SBIT(PWD,		0xB8,	6); // Watchdog Interrupt Priority

/*  I2CFG */
SBIT(CT0,		0xC8,	0); // Clock Time Select 0
SBIT(CT1,		0xC8,	1); // Clock Time Select 1
SBIT(TIRUN,		0xC8,	4); // Timer I Run Enable
SBIT(CLRTI,		0xC8,	5); // Clear Timer I
SBIT(MASTRQ,	0xC8,	6); // Master Request
SBIT(SLAVEN,	0xC8,	7); // Slave Enable

/*  PSW   */
SBIT(P,			0xD0,	0); // Accumulator Parity Flag
SBIT(F1,		0xD0,	1); // Flag 1
SBIT(OV,		0xD0,	2); // Overflow Flag
SBIT(RS0,		0xD0,	3); // Register Bank Select 0
SBIT(RS1,		0xD0,	4); // Register Bank Select 1
SBIT(F0,		0xD0,	5); // Flag 0
SBIT(AC,		0xD0,	6); // Auxiliary Carry Flag
SBIT(CY,		0xD0,	7); // Carry Flag

/*  I2CON */
SBIT(XSTP,		0xD8,	0);
SBIT(MASTER,	0xD8,	1);// Master Status
SBIT(STP,		0xD8,	2); // Stop Detect Flag
SBIT(STR,		0xD8,	3); // Start Detect Flag
SBIT(ARL,		0xD8,	4); // Arbitration Loss Flag
SBIT(DRDY,		0xD8,	5); // Data Ready Flag
SBIT(ATN,		0xD8,	6); // Attention: I2C Interrupt Flag
SBIT(RDAT,		0xD8,	7); // I2C Read Data

/*  ACC   */
SBIT(ACC_0,		0xE0,	0);
SBIT(ACC_1,		0xE0,	1);
SBIT(ACC_2,		0xE0,	2);
SBIT(ACC_3,		0xE0,	3);
SBIT(ACC_4,		0xE0,	4);
SBIT(ACC_5,		0xE0,	5);
SBIT(ACC_6,		0xE0,	6);
SBIT(ACC_7,		0xE0,	7);

/*  IEN1  */
SBIT(EI2,		0xE8,	0); // I2C Interrupt Enable
SBIT(EKB,		0xE8,	1); // Keyboard Interrupt Enable
SBIT(EC2,		0xE8,	2); // Comparator 2 Interrupt Enable
SBIT(EC1,		0xE8,	5); // Comparator 1 Interrupt Enable
SBIT(ETI,		0xE8,	7); // Timer I Interrupt Enable

/*  B     */
SBIT(B_0,		0xF0,	0);
SBIT(B_1,		0xF0,	1);
SBIT(B_2,		0xF0,	2);
SBIT(B_3,		0xF0,	3);
SBIT(B_4,		0xF0,	4);
SBIT(B_5,		0xF0,	5);
SBIT(B_6,		0xF0,	6);
SBIT(B_7,		0xF0,	7);

/*  IP1  */
SBIT(PI2,		0xF8,	0); // I2C Interrupt Priority
SBIT(PKB,		0xF8,	1); // Keyboard Interrupt Priority
SBIT(PC2,		0xF8,	2); // Comparator 2 Interrupt Priority
SBIT(PC1,		0xF8,	5); // Comparator 1 Interrupt Priority
SBIT(PTI,		0xF8,	7); // Timer I Interrupt Priority

/* Bitmasks for SFRs */

/* AUXR1 bits  */
#define DPS     0x01
#define SRST    0x08
#define LPEP    0x10
#define BOI     0x20
#define BOD     0x40
#define KBF     0x80

/* CMP1 bits   */
#define CMF1    0x01
#define CO1     0x02
#define OE1     0x04
#define CN1     0x08
#define CP1     0x10
#define CE1     0x20

/* CMP2 bits   */
#define CMF2    0x01
#define CO2     0x02
#define OE2     0x04
#define CN2     0x08
#define CP2     0x10
#define CE2     0x20

/* I2DAT bits  */
#define RDAT    0x80
#define XDAT    0x80

/* IP1H bits   */
#define PI2H    0x01
#define PKBH    0x02
#define PC2H    0x04
#define PC1H    0x20
#define PTIH    0x80

/* PCON bits   */
#define IDL     0x01
#define PD      0x02
#define GF0     0x04
#define GF1     0x08
#define POF     0x10
#define BOF     0x20
#define SMOD0   0x40
#define SMOD1   0x80

/* P2M1 bits   */
#define ENT0    0x04
#define ENT1    0x08
#define ENTCLK  0x10
#define P0S     0x20
#define P1S     0x40
#define P2S     0x80

/* TMOD bits */
#define M0_0    0x01
#define M1_0    0x02
#define C_T0    0x04
#define GATE0   0x08
#define M0_1    0x10
#define M1_1    0x20
#define C_T1    0x40
#define GATE1   0x80

/* WDCON bits */
#define WDS0    0x01
#define WDS1    0x02
#define WDS2    0x04
#define WDCLK   0x08
#define WDRUN   0x10
#define WDOVF   0x20


/*  Masks for I2CFG bits */
#define BTIR    0x10       // Mask for TIRUN bit.
#define BMRQ    0x40       // Mask for MASTRQ bit.
#define BSLV    0x80       // Mask for SLAVEN bit.


/* Masks for I2CON bits */
#define BCXA    0x80       // Mask for CXA bit.
#define BIDLE   0x40       // Mask for IDLE bit.
#define BCDR    0x20       // Mask for CDR bit.
#define BCARL   0x10       // Mask for CARL bit.
#define BCSTR   0x08       // Mask for CSTR bit.
#define BCSTP   0x04       // Mask for CSTP bit.
#define BXSTR   0x02       // Mask for XSTR bit.
#define BXSTP   0x01       // Mask for XSTP bit.


#endif
