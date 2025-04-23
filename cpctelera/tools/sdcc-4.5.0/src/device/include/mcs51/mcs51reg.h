/*-------------------------------------------------------------------------
   mcs51reg.h - Register Declarations for the mcs51 compatible
   microcontrollers

   Copyright (C) 2000, Bela Torok / bela.torok@kssg.ch

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

/*-------------------------------------------------------------------------
   History:
   --------
   Version 1.0 Nov 2, 2000 - B. Torok  / bela.torok@kssg.ch
   Initial release, supported microcontrollers:
   8051, 8052, Atmel AT89C1051, AT89C2051, AT89C4051,
   Infineon / Siemens SAB80515, SAB80535, SAB80515A

   Version 1.0.1 (Nov 3, 2000)
   SAB80515A definitions revised by Michael Schmitt / michael.schmitt@t-online.de

   Version 1.0.2 (Nov 6, 2000)
   T2CON bug corrected 8052 and SABX microcontrollers have different T2CONs
   Support for the Atmel AT89C52, AT80LV52, AT89C55, AT80LV55
   Support for the Dallas DS80C320 and DS80C323
   B. Torok / bela.torok@kssg.ch

   Version 1.0.3 (Nov 7, 2000)
   SAB80517 definitions added by Michael Schmitt / michael.schmitt@t-online.de
   Dallas AT89S53 definitions added by B. Torok / bela.torok@kssg.ch
   Dallas DS87C520 and DS83C520 definitions added by B. Torok / bela.torok@kssg.ch

   Version 1.0.4 (Nov 9, 2000)
   To simplify the identication of registers, a large number of definitios
   were renamed. Long register names now (hopefully) clearly define the
   function of the registers.
   Dallas DS89C420 definitions added by B. Torok / bela.torok@kssg.ch

   Version 1.0.5 (Dec 15, 2000)
   Definitions added: #ifdef MCS51REG_EXTERNAL_ROM
                      #ifdef MCS51REG_EXTERNAL_RAM
                      #ifndef MCS51REG_DISABLE_WARNINGS


   Version 1.0.6 (March 10, 2001)
   Support for the Dallas DS5000 & DS2250
   Support for the Dallas DS5001 & DS2251
   Support for the Dallas DS80C390
   microcontrollers - B. Torok / bela.torok@kssg.ch

   Version 1.0.7 (June 7, 2001)
   #ifndef MCS51REG_DISABLE_WARNINGS removed
   #ifdef MCS51REG_DISABLE_WARNINGS added - B. Torok / bela.torok@kssg.ch
   Support for the Philips P80C552 added - Bernhard Held / Bernhard.Held@otelo-online.de

   Version 1.0.8 (Feb 28, 2002)
   Dallas DS89C420 definitions corrected by B. Torok / bela.torok@kssg.ch
   Revised by lanius@ewetel.net

   Version 1.0.9 (Sept 9, 2002)
   Register declarations for the Atmel T89C51RD2 added by Johannes Hoelzl / johannes.hoelzl@gmx.de

   Version 1.0.10 (Sept 19, 2002)
   Register declarations for the Philips P89C668 added by Eric Limpens / Eric@limpens.net

   Version 1.0.11 (Sept 19, 2004)
   Dallas DS5000 MCON Register declarations corrected by Radek Zadera / a2i@swipnet.se

   Version 1.0.12 (March 2, 2005)
   Infineon SAB80C509 Register declarations added Thomas Boje / thomas@boje.name

   Adding support for additional microcontrollers:
   -----------------------------------------------

   1. Don't modify this file!!!

   2. Insert your code in a separate file e.g.: mcs51reg_update.h and include
      this after the #define HEADER_MCS51REG statement in this file

   3. The mcs51reg_update.h file should contain following definitions:

          a. An entry with the inventory of the register set of the
             microcontroller in the  "Describe microcontrollers" section.

          b. If necessary add entry(s) for registers not defined in this file

          c. Define interrupt vectors

   4. Compile a program for the microcontroller using the Preprocessor only, e.g.:,
      sdcc -E test.c > t.txt
      and check definitions for validity in the t.txt file.

   5. If everithing seems to be OK send me the mcs51reg_update.h file. --> bela.torok@kssg.ch
      I'm going to resolve conflicts & verify/merge new definitions to this file.


   Microcontroller support:

   Use one of the following options:

   1. use #include <mcs51reg.h> in your program & define MICROCONTROLLER_XXXX in your makefile.

   2. use following definitions prior the
      #include <mcs51reg.h> line in your program:
      e.g.:
      #define MICROCONTROLLER_8052       -> 8052 type microcontroller
      or
      #define MICROCONTROLLER_AT89CX051  -> Atmel AT89C1051, AT89C2051 and AT89C4051 microcontrollers


   Use only one of the following definitions!!!

   Supported Microcontrollers:

   No definition                8051
   MICROCONTROLLER_8051         8051
   MICROCONTROLLER_8052         8052
   MICROCONTROLLER_AT89CX051    Atmel AT89C1051, AT89C2051 and AT89C4051
   MICROCONTROLLER_AT89S53      Atmel AT89S53 microcontroller
   MICROCONTROLLER_AT89X52      Atmel AT89C52 and AT80LV52 microcontrollers
   MICROCONTROLLER_AT89X55      Atmel AT89C55 and AT80LV55 microcontrollers
   MICROCONTROLLER_DS5000       Dallas DS5000 & DS2250 microcontroller
   MICROCONTROLLER_DS5001       Dallas DS5001 & DS2251 microcontroller
   MICROCONTROLLER_DS80C32X     Dallas DS80C320 and DS80C323 microcontrollers
   MICROCONTROLLER_DS80C390     Dallas DS80C390 microcontroller
   MICROCONTROLLER_DS89C420     Dallas DS89C420 microcontroller
   MICROCONTROLLER_DS8XC520     Dallas DS87C520 and DS83C520 microcontrollers
   MICROCONTROLLER_P80C552      Philips P80C552
   MICROCONTROLLER_P89C668      Philips P89C668
   MICROCONTROLLER_SAB80C509    Infineon / Siemens SAB80C509
   MICROCONTROLLER_SAB80515     Infineon / Siemens SAB80515 & SAB80535
   MICROCONTROLLER_SAB80515A    Infineon / Siemens SAB80515A
   MICROCONTROLLER_SAB80517     Infineon / Siemens SAB80517
   MICROCONTROLLER_T89C51RD2    Atmel T89C51RD2

   Additional definitions (use them prior the #include mcs51reg.h statement):

   Ports P0 & P2 are not available if external ROM used.
   Use statement "#define MCS51REG_EXTERNAL_ROM" to undefine P0 & P2.

   Ports P0, P2, P3_6, WR, P3_7 & RD are not available if external RAM is used.
   Use statement "#define MCS51REG_EXTERNAL_RAM" to undefine P0, P2,
   P3_6, WR, P3_7 & RD.

   #define MCS51REG_ENABLE_WARNINGS -> enable warnings

-----------------------------------------------------------------------*/


#ifndef HEADER_MCS51REG
#define HEADER_MCS51REG

#include <compiler.h>

///////////////////////////////////////////////////////
///  Insert header here (for developers only)       ///
///  remove "//" from the begining of the next line ///
//#include "mcs51reg_update.h"                      ///
///////////////////////////////////////////////////////

//////////////////////////////////
///  Describe microcontrollers ///
///  (inventory of registers)  ///
//////////////////////////////////

// definitions for the 8051
#ifdef MICROCONTROLLER_8051
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: 8051
#endif
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__x__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__x__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
#endif
// end of definitions for the 8051


// definitions for the 8052 microcontroller
#ifdef MICROCONTROLLER_8052
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: 8052
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__PT2__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
#endif
// end of definitions for the 8052 microcontroller


// definitionsons for the Atmel
// AT89C1051, AT89C2051 and AT89C4051 microcontrollers
#ifdef MICROCONTROLLER_AT89CX051
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Atmel AT89Cx051
#endif
// 8051 register set without P0 & P2
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define IE__EA__x__x__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__x__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
#endif
// end of definitionsons for the Atmel
// AT89C1051, AT89C2051 and AT89C4051 microcontrollers


// definitions for the Atmel AT89S53
#ifdef MICROCONTROLLER_AT89S53
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: AT89S53
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__PT2__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// AT89S53 specific register
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define P1_EXT__x__x__x__x__x__x__T2EX__T2
#define SPCR
#define SPDR
#define SPSR
#define WCOM
#define DPL1
#define DPH1
#endif
// end of definitions for the Atmel AT89S53 microcontroller


// definitions for the Atmel AT89C52 and AT89LV52 microcontrollers
#ifdef MICROCONTROLLER_AT89X52
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: AT89C52 or AT89LV52
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__PT2__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// AT89X55 specific register
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define P1_EXT__x__x__x__x__x__x__T2EX__T2
#endif
// end of definitions for the Atmel AT89C52 and AT89LV52 microcontrollers


// definitions for the Atmel AT89C55 and AT89LV55 microcontrollers
#ifdef MICROCONTROLLER_AT89X55
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: AT89C55 or AT89LV55
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__PT2__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// AT89X55 specific register
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define P1_EXT__x__x__x__x__x__x__T2EX__T2
#endif
// end of definitions for the Atmel AT89C55 and AT89LV55 microcontrollers


// definitions for the Dallas DS5000
#ifdef MICROCONTROLLER_DS5000
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: DS5000
#endif
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__POR__PFW__WTR__EPFW__EWT__STOP__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__x__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__RWT__x__x__PS__PT1__PX1__PT0__PX0
#define MCON__PA3__PA2__PA1__PA0__RA32_8__ECE2__PAA__SL
#define TA
#define PSW
#define ACC
#define B
#endif
// end of definitions for the Dallas DS5000


// definitions for the Dallas DS5001
#ifdef MICROCONTROLLER_DS5001
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: DS5001
#endif
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__POR__PFW__WTR__EPFW__EWT__STOP__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__x__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__RWT__x__x__PS__PT1__PX1__PT0__PX0
#define CRC
#define CRCLOW
#define CRCHIGH
#define MCON__PA3__PA2__PA1__PA0__RG1__PES__PM__SL
#define TA
#define RNR
#define PSW
#define RPCTL
#define STATUS__ST7__ST6__ST5__ST4__IA0__F0__IBF__OBF
#define ACC
#define B
#endif
// end of definitions for the Dallas DS5001


// definitions for the Dallas DS80C320 and DS80C323 microcontrollers
#ifdef MICROCONTROLLER_DS80C32X
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Dallas DS80C320 or DS80C323
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__SMOD0__x__x__GF1__GF0__STOP__IDLE
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SCON0
#define SBUF
#define P2
#define IE__EA__ES1__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__PS1__PT2__PS__PT1_PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// DS80C320 specific register
#define DPL1
#define DPH1
#define DPS__x__x__x__x__x__x__x__SEL
#define CKCON__WD1__WD0__T2M__T1M__TOM__MD2__MD1__MD0
#define EXIF__IE5__IE4__IE3__IE2__x__RGMD__RGSL__BGS
#define SADDR0
#define SADDR1
#define SADEN0
#define SADEN1
#define SCON1
#define SBUF1
#define STATUS__PIP__HIP__LIP__x__x__x__x__x
#define TA
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define P1_EXT__INT5__INT4__INT3__INT2__TXD1__RXD1__T2EX__T2
#define WDCON
#define EIE__x__x__x__EWDI__EX5__EX4__EX3__EX2
#define EIP__x__x__x__PWDI__PX5__PX4__PX3__PX2
#endif
// end of definitions for the Dallas DS80C320 and DS80C323 microcontrollers


// definitions for the Dallas DS80C390
#ifdef MICROCONTROLLER_DS80C390
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Dallas DS80C390
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__SMOD0__OFDF__OFDE__GF1__GF0__STOP__IDLE
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SCON0
#define SBUF
#define P2
#define IE__EA__ES1__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__PS1__PT2__PS__PT1_PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// DS80C390 specific register
#define P4_AT_0X80
#define DPL1
#define DPH1
#define DPS__ID1__ID0__TSL__x__x__x__x__SEL
#define CKCON__WD1__WD0__T2M__T1M__TOM__MD2__MD1__MD0
#define EXIF__IE5__IE4__IE3__IE2__CKRY__RGMD__RGSL__BGS
#define P4CNT
#define DPX
#define DPX1
#define C0RMS0
#define C0RMS1
#define ESP
#define AP
#define ACON__x__x__x__x__x__SA__AM1__AM0
#define C0TMA0
#define C0TMA1
#define P5_AT_0XA1
#define P5CNT
#define C0C
#define C0S
#define C0IR
#define C0TE
#define C0RE
#define SADDR0
#define SADDR1
#define C0M1C
#define C0M2C
#define C0M3C
#define C0M4C
#define C0M5C
#define C0M6C
#define C0M7C
#define C0M8C
#define C0M9C
#define C0M10C
#define SADEN0
#define SADEN1
#define C0M11C
#define C0M12C
#define C0M13C
#define C0M14C
#define C0M15C
#define SCON1
#define SBUF1
#define PMR__CD1__CD0__SWB__CTM__4X_2X__ALEOFF__x__x
#define STATUS__PIP__HIP__LIP__x__SPTA1__SPRA1__SPTA0__SPRA0
#define MCON__IDM1__IDM0__CMA__x__PDCE3__PDCE2__PDCE1__PDCE0
#define TA
#define T2MOD__x__x__x__D13T1__D13T2__x__T2OE__DCEN
#define COR
#define MCNT0
#define MCNT1
#define MA
#define MB
#define MC
#define C1RSM0
#define C1RSM1
#define WDCON
#define C1TMA0
#define C1TMA1
#define C1C
#define C1S
#define C1IR
#define C1TE
#define C1RE
#define EIE__CANBIE__C0IE__C1IE__EWDI__EX5__EX4__EX3__EX2
#define MXMAX
#define C1M1C
#define C1M2C
#define C1M3C
#define C1M4C
#define C1M5C
#define C1M6C
#define C1M7C
#define C1M8C
#define C1M9C
#define EIP__CANBIP__C0IP__C1IP__PWDI__PX5__PX4__PX3__PX2__PX1__PX0
#define C1M10C
#define C1M11C
#define C1M12C
#define C1M13C
#define C1M14C
#define C1M15C
#define P1_EXT__INT5__INT4__INT3__INT2__TXD1__RXD1__T2EX__T2
#endif
// end of definitions for the Dallas DS80C390

// definitions for the Dallas DS89C420 microcontroller
#ifdef MICROCONTROLLER_DS89C420
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Dallas DS89C420
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__SMOD0__OFDF__OFDE__GF1__GF0__STOP__IDLE
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SCON0
#define SBUF
#define P2
#define IE__EA__ES1__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__PS1__PT2__PS__PT1_PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// DS8XC420 specific registers
#define ACON__PAGEE__PAGES1__PAGES0__x__x__x__x__x
#define DPL1
#define DPH1
#define DPS__ID1__ID0__TSL__AID__x__x__x__SEL
#define CKCON__WD1__WD0__T2M__T1M__TOM__MD2__MD1__MD0
#define CKMOD
#define IP0__x__LPS1__LPT2__LPS0__LPT1__LPX1__LPT0__LPX0
#define IP1__x__MPS1__MPT2__MPS0__MPT1__MPX1__MPT0__MPX0
#define EXIF__IE5__IE4__IE3__IE2__CKRY__RGMD__RGSL__BGS
#define PMR__CD1__CD0__SWB__CTM__4X_2X__ALEON__DME1__DME0
#define SADDR0
#define SADDR1
#define SADEN0
#define SADEN1
#define SCON1
#define SBUF1
#define STATUS__PIS2__PIS1__PIS0__x__SPTA1__SPRA1__SPTA0__SPRA0
#define TA
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define P1_EXT__INT5__INT4__INT3__INT2__TXD1__RXD1__T2EX__T2
#define ROMSIZE__x__x__x__x__PRAME__RMS2__RMS1__RMS0
#define WDCON
#define EIE__x__x__x__EWDI__EX5__EX4__EX3__EX2
#define EIP0__x__x__x__LPWDI__LPX5__LPX4__LPX3__LPX2
#define EIP1__x__x__x__MPWDI__MPX5__MPX4__MPX3__MPX2
#define FCNTL__FBUSY__FERR__x__x__FC3__FC2__FC1__FC0
#endif
// end of definitions for the Dallas DS89C420 microcontroller

// definitions for the Dallas DS87C520 and DS83C520 microcontrollers
#ifdef MICROCONTROLLER_DS8XC520
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Dallas DS87C520 or DS85C520
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__SMOD0__x__x__GF1__GF0__STOP__IDLE
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SCON0
#define SBUF
#define P2
#define IE__EA__ES1__ET2__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__PS1__PT2__PS__PT1_PX1__PT0__PX0
#define PSW
#define ACC
#define B
// 8052 specific registers
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
// DS8XC520 specific registers
#define DPL1
#define DPH1
#define DPS__x__x__x__x__x__x__x__SEL
#define CKCON__WD1__WD0__T2M__T1M__TOM__MD2__MD1__MD0
#define EXIF__IE5__IE4__IE3__IE2__XT_RG__RGMD__RGSL__BGS
#define PMR__CD1__CD0__SWB__x__XTOFF__ALEOFF__DME1__DME0
#define SADDR0
#define SADDR1
#define SADEN0
#define SADEN1
#define SCON1
#define SBUF1
#define STATUS__PIP__HIP__LIP__XTUP__SPTA2__SPTA1__SPTA0__SPRA0
#define TA
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define P1_EXT__INT5__INT4__INT3__INT2__TXD1__RXD1__T2EX__T2
#define WDCON
#define ROMSIZE__x__x__x__x__x__RMS2__RMS1__RMS0
#define BP2
#define WDCON
#define EIE__x__x__x__EWDI__EX5__EX4__EX3__EX2
#define EIP__x__x__x__PWDI__PX5__PX4__PX3__PX2
#endif
// end of definitions for the Dallas DS87C520 and DS83C520 microcontrollers


// definitions for the Philips P80C552 microcontroller
#ifdef MICROCONTROLLER_P80C552
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Philips P80C552
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__WLE__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__EAD__ES1__ES0__ET1__EX1__ET0__EX0
#define P3
#define IP__x__PAD__PS1__PS0__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
// P80C552 specific register-names
#define S0BUF           // same as SBUF, set in mcs51reg.h
#define S0CON__SM0__SM1__SM2__REN__TB8__RB8__TI__RI
// P80C552 specific registers
#define ADCH_AT_0XC6
#define ADCON__ADC_1__ADC_0__ADEX__ADCI__ADCS__AADR2__AADR1__AADR0
#define CTCON__CTN3__CTP3__CTN2__CTP2__CTN1__CTP1__CTN0__CTP0
#define CTH0_AT_0XCC
#define CTH1_AT_0XCD
#define CTH2_AT_0XCE
#define CTH3_AT_0XCF
#define CMH0_AT_0XC9
#define CMH1_AT_0XCA
#define CMH2_AT_0XCB
#define CTL0_AT_0XAC
#define CTL1_AT_0XAD
#define CTL2_AT_0XAE
#define CTL3_AT_0XAF
#define CML0_AT_0XA9
#define CML1_AT_0XAA
#define CML2_AT_0XAB
#define IEN1__ET2__ECM2__ECM1__ECM0__ECT3__ECT2__ECT1__ECT0
#define IP1__PT2__PCM2__PCM1__PCM0__PCT3__PCT2__PCT1__PCT0
#define PWM0_AT_0XFC
#define PWM1_AT_0XFD
#define PWMP_AT_0XFE
#define P1_EXT__SDA__SCL__RT2__T2__CT3I__CT2I__CT1I__CT0I
#define P4_AT_0XC0__CMT0__CMT1__CMSR5__CMSR4__CMSR3__CMSR2__CMSR1__CMSR0
#define P5_AT_0XC4
#define RTE__TP47__TP46__RP45__RP44__RP43__RP42__RP41__RP40
#define S1ADR__x__x__x__x__x__x__x__GC
#define S1DAT_AT_0XDA
#define S1STA__SC4__SC3__SC2__SC1__SC0__x__x__x
#define S1CON__CR2__ENS1__STA__ST0__SI__AA__CR1__CR0
#define STE__TG47__TG46__SP45__SP44__SP43__SP42__SP41__SP40
#define TMH2_AT_0XED
#define TML2_AT_0XEC
#define TM2CON__T2IS1__T2IS0__T2ER__T2B0__T2P1__T2P0__T2MS1__T2MS0
#define TM2IR__T20V__CMI2__CMI1__CMI0__CTI3__CTI2__CTI1__CTI0
#define T3_AT_0XFF
#endif
// end of definitions for the Philips P80C552 microcontroller


// definitions for the Philips P89C668
#ifdef MICROCONTROLLER_P89C668
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: P89C668
#endif
#define P0
#define P0_EXT__AD7__AD6__AD5__AD4__AD3__AD2__AD1__AD0
#define P1
#define P1_EXT__SDA__SCL__CEX2__CEX1__CEX0__ECI__T2EX__T2
#define P2
#define P2_EXT__AD15__AD14__AD13__AD12__AD11__AD10__AD9__AD8
#define P3
#define P3_EXT__x__x__CEX4__CEX3__x__x__x__x
#define SP
#define DPL
#define DPH
#define TCON
#define TMOD
#define PCON__SMOD1__SMOD0__x__POF__GF1__GF0__PD__IDL
#define TL0
#define TL1
#define TH0
#define TH1
#define SCON
#define S0CON__SM0__SM1__SM2__REN__TB8__RB8__TI__RI
#define S1CON__CR2__ENS1__STA__ST0__SI__AA__CR1__CR0
#define SBUF
#define S0BUF SBUF
#define PSW
#define ACC
#define B
#define SADR_AT_0XA9
#define SADEN_AT_0XB9
#define S1IST_AT_0XDC
#define S1STA__SC4__SC3__SC2__SC1__SC0__x__x__x
#define S1DAT_AT_0XDA
#define S1ADR__x__x__x__x__x__x__x__GC
#define SBUF
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define T2MOD__x__x__x__x__x__x__T2OE__DCEN
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2
#define IEN0__EA__EC__ES1__ES0__ET1__EX1__ET0__EX0
#define IEN1__x__x__x__x__x__x__x__ET2
#define IP__PT2__PPC__PS1__PS0__PT1__PX1__PT0__PX0
#define IPH__PT2H__PPCH__PS1H__PS0H__PT1H__PX1H__PT0H__PX0H
#define CCON__CF__CR__x__CCF4__CCF3__CCF2__CCF1__CCF0
#define CMOD__CIDL__WDTE__x__x__x__CPS1__CPS0__ECF
#define AUXR__x__x__x__x__x__x__EXTRAM__A0
#define AUXR1__x__x__ENBOOT__x__GF2__0__x__DPS
#define WDTRST_AT_0XA6
#define CCAPM0_AT_0XC2
#define CCAPM1_AT_0XC3
#define CCAPM2_AT_0XC4
#define CCAPM3_AT_0XC5
#define CCAPM4_AT_0XC6
#define CCAP0L_AT_0XEA
#define CCAP1L_AT_0XEB
#define CCAP2L_AT_0XEC
#define CCAP3L_AT_0XED
#define CCAP4L_AT_0XEE
#define CH_AT_0XF9
#define CL_AT_0XE9
#define CCAP0H_AT_0XFA
#define CCAP1H_AT_0XFB
#define CCAP2H_AT_0XFC
#define CCAP3H_AT_0XFD
#define CCAP4H_AT_0XFE
#endif
// end of definitions for the Philips P89C668


// definitions for the Infineon / Siemens SAB80509
#ifdef MICROCONTROLLER_SAB80509
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Infineon / Siemens SAB80509
#endif
// 8051 register set without IP
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__PDS__IDLS__x__x__x__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define WDTREL
#define P1
#define XPAGE
#define S0CON__SM0__SM1__SM20__REN0__TB80__RB80__TI0__RI0
#define IEN2__SAB80517

#define P2
#define IE__EA_WDT_ET2_ES_ET1_EX1_ET0_EX0
#define IP0__x__WDTS__IP0_5__IP0_4__IP0_3__IP0_2__IP0_1__IP0_0

#define P3
#define SYSCON
#define IEN1__EXEN2__SWDT__EX6__EX5__EX4__EX3__EX2__EADC
#define IP1__x__x__IP1_5__IP1_4__IP1_3__IP1_2__IP1_1__IP1_0

#define IRCON
#define CCEN
#define CCL1
#define CCH1
#define CCL2
#define CCH2
#define CCL3
#define CCH3
#define CCL4
#define CCH4
#define CC4EN
#define S0RELH
#define S0RELL
#define S1BUF
#define S1CON_AT_0X9B
#define S1RELH
#define S1RELL
#define T2CON__T2PS__I3FR__I2FR__T2R1__T2R0__T2CM__T2I1__T2I0

#define PSW
#define CMEN
#define CMH0
#define CML0
#define CMH1
#define CML1
#define CMH2
#define CML2
#define CMH3
#define CML3
#define CMH4
#define CML4
#define CMH5
#define CML5
#define CMH6
#define CML6
#define CMH7
#define CML7
#define CMSEL
#define CRCL
#define CRCH
#define CTCOM_AT_0XE1
#define CTRELH
#define CTRELL
#define TL2
#define TH2
#define ADCON0
#define ADCON1
#define ADDATH
#define ADDATL

#define P4_AT_0XE8
#define DPSEL
#define ARCON
#define MD0
#define MD1
#define MD2
#define MD3
#define MD4
#define MD5
#define S0BUF

#define ACC

#define B

#define P5_AT_0XF8
#define P6_AT_0XFA
#define P7
#define P8

#define COMSETL
#define COMSETH
#define COMCLRL
#define COMCLRH
#define SETMSK
#define CLRMSK
#define SYSCON1
#define FMODE
#define PRSC
#define CT1COM
#define IEN3
#define IRCON2
#define EICC1
#define CC1
#define CC2
#define CC3
#define CC4
#define CCR
#define T2
#define P9_AT_0XF9
#endif
// end of definitions for the Infineon / Siemens SAB80509


// definitions for the Infineon / Siemens SAB80515 & SAB80535
#ifdef MICROCONTROLLER_SAB80515
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Infineon / Siemens SAB80515 & SAB80535
#endif
// 8051 register set without IP
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__x__x__x__x
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA_WDT_ET2_ES_ET1_EX1_ET0_EX0
#define P3
#define PSW
#define ACC
#define B
// SAB80515 specific registers
#define P1_EXT__T2__CLKOUT__T2EX__INT2__INT6_CC3__INT5_CC2__INT4_CC1__INT3_CC0
#define IP0__x__WDTS__IP0_5__IP0_4__IP0_3__IP0_2__IP0_1__IP0_0
#define IEN1__EXEN2__SWDT__EX6__EX5__EX4__EX3__EX2__EADC
#define IRCON
#define CCEN
#define CCL1
#define CCH1
#define CCL2
#define CCH2
#define CCL3
#define CCH3
#define T2CON__T2PS__I3FR__I2FR__T2R1__T2R0__T2CM__T2I1__T2I0
#define CRCL
#define CRCH
#define TL2
#define TH2
#define ADCON
#define ADDAT
#define DAPR__SAB80515
#define P4_AT_0XE8
#define P5_AT_0XF8
#define P6_AT_0XDB
#endif
// end of definitions for the Infineon / Siemens SAB80515


// definitions for the Infineon / Siemens SAB80515A
#ifdef MICROCONTROLLER_SAB80515A
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Infineon / Siemens SAB80515A
#endif
// 8051 register set without IP
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__PDS__IDLS__x__x__x__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA_WDT_ET2_ES_ET1_EX1_ET0_EX0
#define P3
#define PSW
#define ACC
#define B
// SAB80515A specific registers
#define P1_EXT__T2__CLKOUT__T2EX__INT2__INT6_CC3__INT5_CC2__INT4_CC1__INT3_CC0
#define IP0__x__WDTS__IP0_5__IP0_4__IP0_3__IP0_2__IP0_1__IP0_0
#define IP1__x__x__IP1_5__IP1_4__IP1_3__IP1_2__IP1_1__IP1_0
#define IEN1__EXEN2__SWDT__EX6__EX5__EX4__EX3__EX2__EADC
#define IRCON
#define CCEN
#define CCL1
#define CCH1
#define CCL2
#define CCH2
#define CCL3
#define CCH3
#define T2CON__T2PS__I3FR__I2FR__T2R1__T2R0__T2CM__T2I1__T2I0
#define CRCL
#define CRCH
#define TL2
#define TH2
#define ADCON0
#define ADDATH
#define ADDATL
#define ADCON1
#define SRELL
#define SYSCON
#define SRELH
#define P4_AT_0XE8
#define P5_AT_0XF8
#define P6_AT_0XDB
#define XPAGE
#endif
// end of definitions for the Infineon / Siemens SAB80515A


// definitions for the Infineon / Siemens SAB80517
#ifdef MICROCONTROLLER_SAB80517
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: Infineon / Siemens SAB80517
#endif
// 8051 register set without IP, SCON & SBUF
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__PDS__IDLS__x__x__x__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
// #define SCON
// #define SBUF
#define P2
#define IE__EA_WDT_ET2_ES_ET1_EX1_ET0_EX0
#define P3
#define PSW
#define ACC
#define B
// SAB80517 specific registers
#define P1_EXT__T2__CLKOUT__T2EX__INT2__INT6_CC3__INT5_CC2__INT4_CC1__INT3_CC0
#define IP0__x__WDTS__IP0_5__IP0_4__IP0_3__IP0_2__IP0_1__IP0_0
#define IP1__x__x__IP1_5__IP1_4__IP1_3__IP1_2__IP1_1__IP1_0
#define IEN1__EXEN2__SWDT__EX6__EX5__EX4__EX3__EX2__EADC
#define IEN2__SAB80517
#define IRCON
#define CCEN
#define CCL1
#define CCH1
#define CCL2
#define CCH2
#define CCL3
#define CCH3
#define CCL4
#define CCH4
#define CC4EN
#define CMEN
#define CMH0
#define CML0
#define CMH1
#define CML1
#define CMH2
#define CML2
#define CMH3
#define CML3
#define CMH4
#define CML4
#define CMH5
#define CML5
#define CMH6
#define CML6
#define CMH7
#define CML7
#define CMSEL
#define T2CON__T2PS__I3FR__I2FR__T2R1__T2R0__T2CM__T2I1__T2I0
#define CRCL
#define CRCH
#define CTCOM_AT_0XE1
#define CTRELH
#define CTRELL
#define TL2
#define TH2
#define ADCON0
#define ADCON1
#define ADDAT
#define DAPR__SAB80517
#define P4_AT_0XE8
#define P5_AT_0XF8
#define P6_AT_0XFA
#define P7_AT_0XDB
#define P8_AT_0XDD
#define DPSEL
#define ARCON
#define MD0
#define MD1
#define MD2
#define MD3
#define MD4
#define MD5
#define S0BUF
#define S0CON__SM0__SM1__SM20__REN0__TB80__RB80__TI0__RI0
#define S0RELH
#define S0RELL
#define S1BUF
#define S1CON_AT_0X9B
#define S1RELH
#define S1RELL
#define WDTH
#define WDTL
#define WDTREL
#endif
// end of definitions for the Infineon / Siemens SAB80517


// definitions for the Atmel T89C51RD2
#ifdef MICROCONTROLLER_T89C51RD2
#ifdef MICROCONTROLLER_DEFINED
#define MCS51REG_ERROR
#endif
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#endif
#ifdef MCS51REG_ENABLE_WARNINGS
#warning Selected HW: T89C51RD2
#endif

// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD1__SMOD0__x__POF__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__EC__ET2__ES__ET1__EX1__ET0__EX0
#define SADDR
#define P3
#define IP__x__PPC__PT2__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B

// 8052 register set
#define T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#define RCAP2L
#define RCAP2H
#define TL2
#define TH2

// T89C51RD2 register set
#define P4_AT_0XC0__P4_7__P4_6__P4_5__P4_3__P4_2__P4_1__P4_0
#define P5_AT_0XE8
#define SADEN0

#define AUXR1__x__x__x__x__GF3__x__x__DPS
#define WDTRST_AT_0XA6
#define WDTPRG_AT_0XA7
#define AUXR__x__x__M0__x__XRS1__XRS0__EXTRAM__A0
#define IPH__x__PPCH__PT2H__PSH__PT1H__PX1H__PT0H__PX0H
#define FCON
#define EECON
#define EETIM
#define CKCON__X2__T0X2__T1X2__T2X2__SiX2__PcaX2__WdX2__x
#define CCON__0xD8__CF__CR__x__CCF4__CCF3__CCF2__CCF1__CCF0
#define CMOD__0xD9__CIDL__WDTE__x__x__x__CPS1__CPS0__ECF
#define CCAPM0_AT_0XDA
#define CCAPM1_AT_0XDB
#define CCAPM2_AT_0XDC
#define CCAPM3_AT_0XDD
#define CCAPM4_AT_0XDE
#define CL_AT_0XE9
#define CCAP0L_AT_0XEA
#define CCAP1L_AT_0XEB
#define CCAP2L_AT_0XEC
#define CCAP3L_AT_0XED
#define CCAP4L_AT_0XEE
#define CH_AT_0XF9
#define CCAP0H_AT_0XFA
#define CCAP1H_AT_0XFB
#define CCAP2H_AT_0XFC
#define CCAP3H_AT_0XFD
#define CCAP4H_AT_0XFE
#endif /* MICROCONTROLLER_T89C51RD2 */
/* end of definition for the Atmel T89C51RD2 */


/////////////////////////////////////////////////////////
///  don't specify microcontrollers below this line!  ///
/////////////////////////////////////////////////////////


// default microcontroller -> 8051
// use default if no microcontroller specified
#ifndef MICROCONTROLLER_DEFINED
#define MICROCONTROLLER_DEFINED
#ifdef MCS51REG_ENABLE_WARNINGS
#warning No microcontroller defined!
#warning Code generated for the 8051
#endif
// 8051 register set
#define P0
#define SP
#define DPL
#define DPH
#define PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#define TCON
#define TMOD
#define TL0
#define TL1
#define TH0
#define TH1
#define P1
#define SCON
#define SBUF
#define P2
#define IE__EA__x__x__ES__ET1__EX1__ET0__EX0
#define P3
#define IP__x__x__x__PS__PT1__PX1__PT0__PX0
#define PSW
#define ACC
#define B
#endif
// end of definitions for the default microcontroller


#ifdef MCS51REG_ERROR
#error Two or more microcontrollers defined!
#endif

#ifdef MCS51REG_EXTERNAL_ROM
#ifndef MCS51REG_UNDEFINE_P0
#define MCS51REG_UNDEFINE_P0
#endif
#ifndef MCS51REG_UNDEFINE_P2
#define MCS51REG_UNDEFINE_P2
#endif
#endif

#ifdef MCS51REG_EXTERNAL_RAM
#ifndef MCS51REG_UNDEFINE_P0
#define MCS51REG_UNDEFINE_P0
#endif
#ifndef MCS51REG_UNDEFINE_P2
#define MCS51REG_UNDEFINE_P2
#endif
#endif

#ifdef MCS51REG_UNDEFINE_P0
#undef P0
#endif

#ifdef MCS51REG_UNDEFINE_P2
#undef P2
#endif

////////////////////////////////
///  Register definitions    ///
///  (In alphabetical order) ///
////////////////////////////////

#ifdef ACC
#undef ACC
SFR(ACC,	0xE0);
#endif

#ifdef ACON__PAGEE__PAGES1__PAGES0__x__x__x__x__x
#undef ACON__PAGEE__PAGES1__PAGES0__x__x__x__x__x
SFR(ACON,	0x9D); // DS89C420 specific
// Not directly accessible bits
#define PAGES0      0x20
#define PAGES1      0x40
#define PAGEE       0x80
#endif

#ifdef ACON__x__x__x__x__x__SA__AM1__AM0
#undef ACON__x__x__x__x__x__SA__AM1__AM0
SFR(ACON,	0x9D); // DS89C390 specific
// Not directly accessible bits
#define AM0         0x01
#define AM1         0x02
#define SA          0x04
#endif

#ifdef ADCH_AT_0XC6
#undef ADCH_AT_0XC6
SFR(ADCH,	0xC6); // A/D converter high
#endif

#ifdef ADCON
#undef ADCON
SFR(ADCON,	0xD8); // A/D-converter control register SAB80515 specific
// Bit registers
SBIT(MX0,	0xD8,	0);
SBIT(MX1,	0xD8,	1);
SBIT(MX2,	0xD8,	2);
SBIT(ADM,	0xD8,	3);
SBIT(BSY,	0xD8,	4);
SBIT(CLK,	0xD8,	6);
SBIT(BD,	0xD8,	7);
#endif

// ADCON0 ... Infineon / Siemens also called this register ADCON in the User Manual
#ifdef ADCON0
#undef ADCON0
SFR(ADCON0,	0xD8); // A/D-converter control register 0 SAB80515A &
// Bit registers          // SAB80517 specific
SBIT(MX0,	0xD8,	0);
SBIT(MX1,	0xD8,	1);
SBIT(MX2,	0xD8,	2);
SBIT(ADM,	0xD8,	3);
SBIT(BSY,	0xD8,	4);
SBIT(ADEX,	0xD8,	5);
SBIT(CLK,	0xD8,	6);
SBIT(BD,	0xD8,	7);
// Not directly accessible ADCON0
#define ADCON0_MX0  0x01
#define ADCON0_MX1  0x02
#define ADCON0_MX2  0x04
#define ADCON0_ADM  0x08
#define ADCON0_BSY  0x10
#define ADCON0_ADEX 0x20
#define ADCON0_CLK  0x40
#define ADCON0_BD   0x80
#endif

#ifdef ADCON1
#undef ADCON1
SFR(ADCON1,	0xDC); // A/D-converter control register 1 SAB80515A & SAB80517 specific
// Not directly accessible ADCON1
#define ADCON1_MX0  0x01
#define ADCON1_MX1  0x02
#define ADCON1_MX2  0x04
#define ADCON1_ADCL 0x80
#endif

#ifdef ADCON__ADC_1__ADC_0__ADEX__ADCI__ADCS__AADR2__AADR1__AADR0
#undef ADCON__ADC_1__ADC_0__ADEX__ADCI__ADCS__AADR2__AADR1__AADR0
SFR(ADCON,	0xC5); // A/D control, P80C552 specific
// Not directly accessible Bits.
#define AADR0       0x01
#define AADR1       0x02
#define AADR2       0x04
#define ADCS        0x08
#define ADCI        0x10
#define ADEX        0x20
#define ADC_0       0x40    // different name as ADC0 in P5
#define ADC_1       0x80    // different name as ADC1 in P5
#endif

#ifdef ADDAT
#undef ADDAT
SFR(ADDAT,	0xD9); // A/D-converter data register SAB80515 specific
#endif

#ifdef ADDATH
#undef ADDATH
SFR(ADDATH,	0xD9); // A/D data high byte SAB80515A specific
#endif

#ifdef ADDATL
#undef ADDATL
SFR(ADDATL,	0xDA); // A/D data low byte SAB80515A specific
#endif

#ifdef ARCON
#undef ARCON
SFR(ARCON,	0xEF); // arithmetic control register SAB80517
#endif

#ifdef AP
#undef AP
SFR(AP,	0x9C); // DS80C390
#endif

#ifdef AUXR__x__x__x__x__x__x__EXTRAM__A0
#undef AUXR__x__x__x__x__x__x__EXTRAM__A0
// P89C668 specific, Auxilary
SFR(AUXR,	0x8E);
// not bit addressable:
#define EXTRAM      0x02
#define A0          0x01
#endif

#ifdef AUXR__x__x__M0__x__XRS1__XRS0__EXTRAM__A0
#undef AUXR__x__x__M0__x__XRS1__XRS0__EXTRAM__A0
SFR(AUXR,	0x8E);
#define AO          0x01
#define EXTRAM      0x02
#define XRS0        0x04
#define XRS1        0x08
#define M0          0x20
#endif
#ifdef B
#undef B
SFR(B,	0xF0);
// Bit registers
SBIT(BREG_F0,	0xF0,	0);
SBIT(BREG_F1,	0xF0,	1);
SBIT(BREG_F2,	0xF0,	2);
SBIT(BREG_F3,	0xF0,	3);
SBIT(BREG_F4,	0xF0,	4);
SBIT(BREG_F5,	0xF0,	5);
SBIT(BREG_F6,	0xF0,	6);
SBIT(BREG_F7,	0xF0,	7);
#endif

#ifdef AUXR1__x__x__x__x__GF3__x__x__DPS
#undef AUXR1__x__x__x__x__GF3__x__x__DPS
SFR(AUXR1,	0xA2);
#define DPS         0x01
#define GF3         0x08
#endif

#ifdef AUXR1__x__x__ENBOOT__x__GF2__0__x__DPS
#undef AUXR1__x__x__ENBOOT__x__GF2__0__x__DPS
// P89C668 specific, Auxilary 1
SFR(AUXR1,	0xA2);
#define ENBOOT      0x20
#define GF2         0x08
#define ALWAYS_ZERO 0x04
#define DPS         0x01
#endif

#ifdef BP2
#undef BP2
SFR(BP2,	0xC3);
// Not directly accessible bits
#define MS0         0x01
#define MS1         0x02
#define MS2         0x04
#define LB1         0x08
#define LB2         0x10
#define LB3         0x20
#endif

#ifdef C0C
#undef C0C
SFR(C0C,	0xA3); // DS80C390 specific
// Not directly accessible bits
#define SWINT       0x01
#define ERCS        0x02
#define AUTOB       0x04
#define CRST        0x08
#define SIESTA      0x10
#define PDE         0x20
#define STIE        0x40
#define ERIE        0x80
#endif

#ifdef C0IR
#undef C0IR
SFR(C0IR,	0xA5); // DS80C390 specific
// Not directly accessible bits
#define INTIN0      0x01
#define INTIN1      0x02
#define INTIN2      0x04
#define INTIN3      0x08
#define INTIN4      0x10
#define INTIN5      0x20
#define INTIN6      0x40
#define INTIN7      0x80
#endif

#ifdef C0M1C
#undef C0M1C
SFR(C0M1C,	0xAB); // DS80C390 specific
// Not directly accessible bits
#define DTUP        0x01
#define ROW_TIH     0x02
#define MTRQ        0x04
#define EXTRQ       0x08
#define INTRQ       0x10
#define ERI         0x20
#define ETI         0x40
#define MSRDY       0x80
#endif

#ifdef C0M2C
#undef C0M2C
SFR(C0M2C,	0xAC); // DS80C390 specific
#endif

#ifdef C0M3C
#undef C0M3C
SFR(C0M3C,	0xAD); // DS80C390 specific
#endif

#ifdef C0M4C
#undef C0M4C
SFR(C0M4C,	0xAE); // DS80C390 specific
#endif

#ifdef C0M5C
#undef C0M5C
SFR(C0M5C,	0xAF); // DS80C390 specific
#endif

#ifdef C0M6C
#undef C0M6C
SFR(C0M6C,	0xB3); // DS80C390 specific
#endif

#ifdef C0M7C
#undef C0M7C
SFR(C0M7C,	0xB4); // DS80C390 specific
#endif

#ifdef C0M8C
#undef C0M8C
SFR(C0M8C,	0xB5); // DS80C390 specific
#endif

#ifdef C0M9C
#undef C0M9C
SFR(C0M9C,	0xB6); // DS80C390 specific
#endif

#ifdef C0M10C
#undef C0M10C
SFR(C0M10C,	0xB7); // DS80C390 specific
#endif

#ifdef C0M11C
#undef C0M11C
SFR(C0M11C,	0xBB); // DS80C390 specific
#endif

#ifdef C0M12C
#undef C0M12C
SFR(C0M12C,	0xBC); // DS80C390 specific
#endif

#ifdef C0M13C
#undef C0M13C
SFR(C0M13C,	0xBD); // DS80C390 specific
#endif

#ifdef C0M14C
#undef C0M14C
SFR(C0M14C,	0xBE); // DS80C390 specific
#endif

#ifdef C0M15C
#undef C0M15C
SFR(C0M15C,	0xBF); // DS80C390 specific
#endif

#ifdef C0RE
#undef C0RE
SFR(C0RE,	0xA7); // DS80C390 specific
#endif

#ifdef C0RMS0
#undef C0RMS0
SFR(C0RMS0,	0x96); // DS80C390 specific
#endif

#ifdef C0RMS1
#undef C0RMS1
SFR(C0RMS1,	0x97); // DS80C390 specific
#endif

#ifdef C0S
#undef C0S
SFR(C0S,	0xA4); // DS80C390 specific
// Not directly accessible bits
#define ER0         0x01
#define ER1         0x02
#define ER2         0x04
#define TXS         0x08
#define RXS         0x10
#define WKS         0x20
#define EC96_128    0x40
#define BSS         0x80
#endif

#ifdef C0TE
#undef C0TE
SFR(C0TE,	0xA6); // DS80C390 specific
#endif

#ifdef C0TMA0
#undef C0TMA0
SFR(C0TMA0,	0x9E); // DS80C390 specific
#endif

#ifdef C0TMA1
#undef C0TMA1
SFR(C0TMA1,	0x9F); // DS80C390 specific
#endif

#ifdef C1C
#undef C1C
SFR(C1C,	0xE3); // DS80C390 specific
// Not directly accessible bits
#define SWINT       0x01
#define ERCS        0x02
#define AUTOB       0x04
#define CRST        0x08
#define SIESTA      0x10
#define PDE         0x20
#define STIE        0x40
#define ERIE        0x80
#endif

#ifdef C1IR
#undef C1IR
SFR(C1IR,	0xE5); // DS80C390 specific
// Not directly accessible bits
#define INTIN0      0x01
#define INTIN1      0x02
#define INTIN2      0x04
#define INTIN3      0x08
#define INTIN4      0x10
#define INTIN5      0x20
#define INTIN6      0x40
#define INTIN7      0x80
#endif

#ifdef C1IRE
#undef C1IRE
SFR(C1RE,	0xE7); // DS80C390 specific
#endif

#ifdef C1M1C
#undef C1M1C
SFR(C1M1C,	0xEB); // DS80C390 specific
#endif

#ifdef C1M2C
#undef C1M2C
SFR(C1M2C,	0xEC); // DS80C390 specific
#endif

#ifdef C1M3C
#undef C1M3C
SFR(C1M3C,	0xED); // DS80C390 specific
#endif

#ifdef C1M4C
#undef C1M4C
SFR(C1M4C,	0xEE); // DS80C390 specific
#endif

#ifdef C1M5C
#undef C1M5C
SFR(C1M5C,	0xEF); // DS80C390 specific
#endif

#ifdef C1M6C
#undef C1M6C
SFR(C1M6C,	0xF3); // DS80C390 specific
#endif

#ifdef C1M7C
#undef C1M7C
SFR(C1M7C,	0xF4); // DS80C390 specific
#endif

#ifdef C1M8C
#undef C1M8C
SFR(C1M8C,	0xF5); // DS80C390 specific
#endif

#ifdef C1M9C
#undef C1M9C
SFR(C1M9C,	0xF6); // DS80C390 specific
#endif

#ifdef C1M10C
#undef C1M10C
SFR(C1M10C,	0xF7); // DS80C390 specific
#endif

#ifdef C1M11C
#undef C1M11C
SFR(C1M11C,	0xFB); // DS80C390 specific
#endif

#ifdef C1M12C
#undef C1M12C
SFR(C1M12C,	0xFC); // DS80C390 specific
#endif

#ifdef C1M13C
#undef C1M13C
SFR(C1M13C,	0xFD); // DS80C390 specific
#endif

#ifdef C1M14C
#undef C1M14C
SFR(C1M14C,	0xFE); // DS80C390 specific
#endif

#ifdef C1M15C
#undef C1M15C
SFR(C1M15C,	0xFF); // DS80C390 specific
#endif

#ifdef C1S
#undef C1S
SFR(C1S,	0xE4); // DS80C390 specific
// Not directly accessible bits
#define ER0         0x01
#define ER1         0x02
#define ER2         0x04
#define TXS         0x08
#define RXS         0x10
#define WKS         0x20
#define CECE        0x40
#define BSS         0x80
#endif

#ifdef C1ITE
#undef C1ITE
SFR(C1TE,	0xE6); // DS80C390 specific
#endif

#ifdef C1RSM0
#undef C1RSM0
SFR(C1RSM0,	0xD6); // DS80C390 specific
#endif

#ifdef C1RSM1
#undef C1RSM1
SFR(C1RSM1,	0xD7); // DS80C390 specific
#endif

#ifdef C1TMA0
#undef C1TMA0
SFR(C1TMA0,	0xDE); // DS80C390 specific
#endif

#ifdef C1TMA1
#undef C1TMA1
SFR(C1TMA1,	0xDF); // DS80C390 specific
#endif

#ifdef CC1
#undef CC1
SFR(CC1,	0xC2);
#endif

#ifdef CC2
#undef CC2
SFR(CC2,	0xC4);
#endif

#ifdef CC3
#undef CC3
SFR(CC3,	0xC6);
#endif

#ifdef CC4
#undef CC4
SFR(CC4,	0xCE);
#endif

#ifdef CC4EN
#undef CC4EN
SFR(CC4EN,	0xC9); // compare/capture 4 enable register SAB80517 specific
#endif

#ifdef CCAP0H_AT_0XFA
#undef CCAP0H_AT_0XFA
SFR(CCAP0H,	0xFA);
#endif

#ifdef CCAP1H_AT_0XFB
#undef CCAP1H_AT_0XFB
SFR(CCAP1H,	0xFB);
#endif

#ifdef CCAP2H_AT_0XFC
#undef CCAP2H_AT_0XFC
SFR(CCAP2H,	0xFC);
#endif

#ifdef CCAP3H_AT_0XFD
#undef CCAP3H_AT_0XFD
SFR(CCAP3H,	0xFD);
#endif

#ifdef CCAP4H_AT_0XFE
#undef CCAP4H_AT_0XFE
SFR(CCAP4H,	0xFE);
#endif

#ifdef CCAP0L_AT_0XEA
#undef CCAP0L_AT_0XEA
SFR(CCAP0L,	0xEA);
#endif

#ifdef CCAP1L_AT_0XEB
#undef CCAP1L_AT_0XEB
SFR(CCAP1L,	0xEB);
#endif

#ifdef CCAP2L_AT_0XEC
#undef CCAP2L_AT_0XEC
SFR(CCAP2L,	0xEC);
#endif

#ifdef CCAP3L_AT_0XED
#undef CCAP3L_AT_0XED
SFR(CCAP3L,	0xED);
#endif

#ifdef CCAP4L_AT_0XEE
#undef CCAP4L_AT_0XEE
SFR(CCAP4L,	0xEE);
#endif

#ifdef CCAPM0_AT_0XC2
#undef CCAPM0_AT_0XC2
// P89C668 specific, Capture module:
SFR(CCAPM0,	0xC2);
#endif

#ifdef CCAPM0_AT_0XDA
#undef CCAPM0_AT_0XDA
SFR(CCAPM0,	0xDA);
#define ECCF        0x01
#define PWM         0x02
#define TOG         0x04
#define MAT         0x08
#define CAPN        0x10
#define CAPP        0x20
#define ECOM        0x40
#endif

#ifdef CCAPM1_AT_0XC3
#undef CCAPM1_AT_0XC3
SFR(CCAPM1,	0xC3);
#endif

#ifdef CCAPM1_AT_0XDB
#undef CCAPM1_AT_0XDB
SFR(CCAPM1,	0xDB);
#endif

#ifdef CCAPM2_AT_0XC4
#undef CCAPM2_AT_0XC4
SFR(CCAPM2,	0xC4);
#endif

#ifdef CCAPM2_AT_0XDC
#undef CCAPM2_AT_0XDC
SFR(CCAPM2,	0x0DC);
#endif

#ifdef CCAPM3_AT_0XC5
#undef CCAPM3_AT_0XC5
SFR(CCAPM3,	0xC5);
#endif

#ifdef CCAPM3_AT_0XDD
#undef CCAPM3_AT_0XDD
SFR(CCAPM3,	0x0DD);
#endif

#ifdef CCAPM4_AT_0XDE
#undef CCAPM4_AT_0XDE
SFR(CCAPM4,	0x0DE);
#endif

#ifdef CCAPM4_AT_0XC6
#undef CCAPM4_AT_0XC6
SFR(CCAPM4,	0xC6);
#endif

#ifdef CCEN
#undef CCEN
SFR(CCEN,	0xC1); // compare/capture enable register SAB80515 specific
#endif

#ifdef CCH1
#undef CCH1
SFR(CCH1,	0xC3); // compare/capture register 1, high byte SAB80515 specific
#endif

#ifdef CCH2
#undef CCH2
SFR(CCH2,	0xC5); // compare/capture register 2, high byte SAB80515 specific
#endif

#ifdef CCH3
#undef CCH3
SFR(CCH3,	0xC7); // compare/capture register 3, high byte SAB80515 specific
#endif

#ifdef CCH4
#undef CCH4
SFR(CCH4,	0xCF); // compare/capture register 4, high byte SAB80515 specific
#endif

#ifdef CCL1
#undef CCL1
SFR(CCL1,	0xC2); // compare/capture register 1, low byte SAB80515 specific
#endif

#ifdef CCL2
#undef CCL2
SFR(CCL2,	0xC4); // compare/capture register 2, low byte SAB80515 specific
#endif

#ifdef CCL3
#undef CCL3
SFR(CCL3,	0xC6); // compare/capture register 3, low byte SAB80515 specific
#endif

#ifdef CCL4
#undef CCL4
SFR(CCL4,	0xCE); // compare/capture register 4, low byte SAB80515 specific
#endif

#ifdef CCON__0xD8__CF__CR__x__CCF4__CCF3__CCF2__CCF1__CCF0
#undef CCON__0xD8__CF__CR__x__CCF4__CCF3__CCF2__CCF1__CCF0
SFR(CCON,	0xD8); // T89C51RD2 specific register
// Bit registers
SBIT(CCF0,	0xD8,	0);
SBIT(CCF1,	0xD8,	1);
SBIT(CCF2,	0xD8,	2);
SBIT(CCF3,	0xD8,	3);
SBIT(CCF4,	0xD8,	4);
SBIT(CR,	0xD8,	6);
SBIT(CF,	0xD8,	7);
#endif

#ifdef CCON__CF__CR__x__CCF4__CCF3__CCF2__CCF1__CCF0
#undef CCON__CF__CR__x__CCF4__CCF3__CCF2__CCF1__CCF0
// P89C668 specific, PCA Counter control:
SFR(CCON,	0xC0);
// Bit registers
SBIT(CCF0,	0xC0,	0);
SBIT(CCF1,	0xC0,	1);
SBIT(CCF2,	0xC0,	2);
SBIT(CCF3,	0xC0,	3);
SBIT(CCF4,	0xC0,	4);
//__sbit __at 0xC5 -
SBIT(CR,	0xC0,	6);
SBIT(CF,	0xC0,	7);
#endif

#ifdef CCR
#undef CCR
SFR(CCR,	0xCA);
#endif

#ifdef CH_AT_0XF9
#undef CH_AT_0XF9
SFR(CH,	0xF9);
#endif

#ifdef CMOD__CIDL__WDTE__x__x__x__CPS1__CPS0__ECF
#undef CMOD__CIDL__WDTE__x__x__x__CPS1__CPS0__ECF
// P89C668 specific, PCA Counter mode:
SFR(CMOD,	0xC1);
// not bit addressable:
#define CIDL        0x80
#define WDTE        0x40
#define CPS1        0x04
#define CPS0        0x02
#define ECF         0x01
#endif

#ifdef CKCON__WD1__WD0__T2M__T1M__TOM__MD2__MD1__MD0
#undef CKCON__WD1__WD0__T2M__T1M__TOM__MD2__MD1__MD0
SFR(CKCON,	0x8E); // DS80C320 & DS80C390 specific
// Not directly accessible Bits.
#define MD0         0x01
#define MD1         0x02
#define MD2         0x04
#define T0M         0x08
#define T1M         0x10
#define T2M         0x20
#define WD0         0x40
#define WD1         0x80
#endif

#ifdef CKCON__X2__T0X2__T1X2__T2X2__SiX2__PcaX2__WdX2__x
#undef CKCON__X2__T0X2__T1X2__T2X2__SiX2__PcaX2__WdX2__x
SFR(CKCON,	0x8F);
#define X2          0x01
#define T0X2        0x02
#define T1X2        0x04
#define T2X2        0x08
#define SiX2        0x10
#define PcaX2       0x20
#define WdX2        0x40
#endif

#ifdef CKMOD
#undef CKMOD
SFR(CKMOD,	0x96); // DS89C420 specific
// Not directly accessible Bits.
#define T0MH        0x08
#define T1MH        0x10
#define T2MH        0x20
#endif

#ifdef CL_AT_0XE9
#undef CL_AT_0XE9
SFR(CL,	0xE9);
#endif

#ifdef CLRMSK
#undef CLRMSK
SFR(CLRMSK,	0xA6);
#endif

#ifdef CMEN
#undef CMEN
SFR(CMEN,	0xF6); // compare enable register SAB80517 specific
#endif

#ifdef CMH0
#undef CMH0
SFR(CMH0,	0xD3); // compare register 0 high byte SAB80517 specific
#endif

#ifdef CMH1
#undef CMH1
SFR(CMH1,	0xD5); // compare register 1 high byte SAB80517 specific
#endif

#ifdef CMH2
#undef CMH2
SFR(CMH2,	0xD7); // compare register 2 high byte SAB80517 specific
#endif

#ifdef CMH3
#undef CMH3
SFR(CMH3,	0xE3); // compare register 3 high byte SAB80517 specific
#endif

#ifdef CMH4
#undef CMH4
SFR(CMH4,	0xE5); // compare register 4 high byte SAB80517 specific
#endif

#ifdef CMH5
#undef CMH5
SFR(CMH5,	0xE7); // compare register 5 high byte SAB80517 specific
#endif

#ifdef CMH6
#undef CMH6
SFR(CMH6,	0xF3); // compare register 6 high byte SAB80517 specific
#endif

#ifdef CMH7
#undef CMH7
SFR(CMH7,	0xF5); // compare register 7 high byte SAB80517 specific
#endif

#ifdef CMH0_AT_0XC9
#undef CMH0_AT_0XC9
SFR(CMH0,	0xC9); // Compare high 0, P80C552 specific
#endif

#ifdef CMH1_AT_0XCA
#undef CMH1_AT_0XCA
SFR(CMH1,	0xCA); // Compare high 1, P80C552 specific
#endif

#ifdef CMH2_AT_0XCB
#undef CMH2_AT_0XCB
SFR(CMH2,	0xCB); // Compare high 2, P80C552 specific
#endif

#ifdef CML0
#undef CML0
SFR(CML0,	0xD2); // compare register 0 low byte SAB80517 specific
#endif

#ifdef CML1
#undef CML1
SFR(CML1,	0xD4); // compare register 1 low byte SAB80517 specific
#endif

#ifdef CML2
#undef CML2
SFR(CML2,	0xD6); // compare register 2 low byte SAB80517 specific
#endif

#ifdef CML3
#undef CML3
SFR(CML3,	0xE2); // compare register 3 low byte SAB80517 specific
#endif

#ifdef CML4
#undef CML4
SFR(CML4,	0xE4); // compare register 4 low byte SAB80517 specific
#endif

#ifdef CML5
#undef CML5
SFR(CML5,	0xE6); // compare register 5 low byte SAB80517 specific
#endif

#ifdef CML6
#undef CML6
SFR(CML6,	0xF2); // compare register 6 low byte SAB80517 specific
#endif

#ifdef CML7
#undef CML7
SFR(CML7,	0xF4); // compare register 7 low byte SAB80517 specific
#endif

#ifdef CML0_AT_0XA9
#undef CML0_AT_0XA9
SFR(CML0,	0xA9); // Compare low 0, P80C552 specific
#endif

#ifdef CML1_AT_0XAA
#undef CML1_AT_0XAA
SFR(CML1,	0xAA); // Compare low 1, P80C552 specific
#endif

#ifdef CML2_AT_0XAB
#undef CML2_AT_0XAB
SFR(CML2,	0xAB); // Compare low 2, P80C552 specific
#endif

#ifdef CMOD__0xD9__CIDL__WDTE__x__x__x__CPS1__CPS0__ECF
#undef CMOD__0xD9__CIDL__WDTE__x__x__x__CPS1__CPS0__ECF
SFR(CMOD,	0xD9);
#define ECF         0x01
#define CPS0        0x02
#define CPS1        0x04
#define WDTE        0x40
#define CIDL        0x80
#endif

#ifdef CMSEL
#undef CMSEL
SFR(CMSEL,	0xF7); // compare input select SAB80517
#endif

#ifdef COMCLRH
#undef COMCLRH
SFR(COMCLRH,	0xA4);
#endif

#ifdef COMCLRL
#undef COMCLRL
SFR(COMCLRL,	0xA3);
#endif

#ifdef COMSETH
#undef COMSETH
SFR(COMSETH,	0xA2);
#endif

#ifdef COMSETL
#undef COMSETL
SFR(COMSETL,	0xA1);
#endif

#ifdef COR
#undef COR
SFR(COR,	0xCE); // Dallas DS80C390 specific
#define CLKOE       0x01
#define COD0        0x02
#define COD1        0x04
#define C0BPR6      0x08
#define C0BPR7      0x10
#define C1BPR6      0x20
#define C1BPR7      0x40
#define IRDACK      0x80
#endif

#ifdef CRC
#undef CRC
SFR(CRC,	0xC1); // Dallas DS5001 specific
#define CRC_        0x01
#define MDM         0x02
#define RNGE0       0x10
#define RNGE1       0x20
#define RNGE2       0x40
#define RNGE3       0x80
#endif

#ifdef CRCH
#undef CRCH
SFR(CRCH,	0xCB); // compare/reload/capture register, high byte SAB80515 specific
#endif

#ifdef CRCHIGH
#undef CRCHIGH
SFR(CRCHIGH,	0xC3); // DS5001 specific
#endif

#ifdef CRCL
#undef CRCL
SFR(CRCL,	0xCA); // compare/reload/capture register, low byte SAB80515 specific
#endif

#ifdef CRCLOW
#undef CRCLOW
SFR(CRCLOW,	0xC2); // DS5001 specific
#endif

#ifdef CT1COM
#undef CT1COM
SFR(CT1COM,	0xBC);
#endif

#ifdef CTCOM_AT_0XE1
#undef CTCOM_AT_0XE1
SFR(CTCON,	0xE1); // com.timer control register SAB80517
#endif

#ifdef CTCON__CTN3__CTP3__CTN2__CTP2__CTN1__CTP1__CTN0__CTP0
#undef CTCON__CTN3__CTP3__CTN2__CTP2__CTN1__CTP1__CTN0__CTP0
SFR(CTCON,	0xEB); // Capture control, P80C552 specific
// Not directly accessible Bits.
#define CTP0        0x01
#define CTN0        0x02
#define CTP1        0x04
#define CTN1        0x08
#define CTP2        0x10
#define CTN2        0x20
#define CTP3        0x40
#define CTN3        0x80
#endif

#ifdef CTH0_AT_0XCC
#undef CTH0_AT_0XCC
SFR(CTH0,	0xCC); // Capture high 0, P80C552 specific
#endif

#ifdef CTH1_AT_0XCD
#undef CTH1_AT_0XCD
SFR(CTH1,	0xCD); // Capture high 1, P80C552 specific
#endif

#ifdef CTH2_AT_0XCE
#undef CTH2_AT_0XCE
SFR(CTH2,	0xCE); // Capture high 2, P80C552 specific
#endif

#ifdef CTH3_AT_0XCF
#undef CTH3_AT_0XCF
SFR(CTH3,	0xCF); // Capture high 3, P80C552 specific
#endif

#ifdef CTL0_AT_0XAC
#undef CTL0_AT_0XAC
SFR(CTL0,	0xAC); // Capture low 0, P80C552 specific
#endif

#ifdef CTL1_AT_0XAD
#undef CTL1_AT_0XAD
SFR(CTL1,	0xAD); // Capture low 1, P80C552 specific
#endif

#ifdef CTL2_AT_0XAE
#undef CTL2_AT_0XAE
SFR(CTL2,	0xAE); // Capture low 2, P80C552 specific
#endif

#ifdef CTL3_AT_0XAF
#undef CTL3_AT_0XAF
SFR(CTL3,	0xAF); // Capture low 3, P80C552 specific
#endif

#ifdef CTRELH
#undef CTRELH
SFR(CTRELH,	0xDF); // com.timer rel register high byte SAB80517
#endif

#ifdef CTRELL
#undef CTRELL
SFR(CTRELL,	0xDE); // com.timer rel register low byte SAB80517
#endif

#ifdef DAPR__SAB80515
#undef DAPR__SAB80515
SFR(DAPR,	0xDA); // D/A-converter program register SAB80515 specific
#endif

#ifdef DAPR__SAB80517
#undef DAPR__SAB80517
SFR(DAPR,	0xDA); // D/A-converter program register SAB80517 specific
#endif

#ifdef DPH
#undef DPH
SFR(DPH,	0x83);
SFR(DP0H,	0x83);  // Alternate name for AT89S53
#endif

#ifdef DPH1
#undef DPH1
SFR(DPH1,	0x85); // DS80C320 specific
SFR(DP1H,	0x85); // Alternate name for AT89S53
#endif

#ifdef DPL
#undef DPL
SFR(DPL,	0x82);  // Alternate name for AT89S53
SFR(DP0L,	0x82);
#endif

#ifdef DPL1
#undef DPL1
SFR(DPL1,	0x84); // DS80C320 specific
SFR(DP1L,	0x84); // Alternate name for AT89S53
#endif

#ifdef DPS__x__x__x__x__x__x__x__SEL
#undef DPS__x__x__x__x__x__x__x__SEL
SFR(DPS,	0x86);
// Not directly accessible DPS Bit. DS80C320 & DPS8XC520 specific
#define SEL         0x01
#endif

#ifdef DPS__ID1__ID0__TSL__x__x__x__x__SEL
#undef DPS__ID1__ID0__TSL__x__x__x__x__SEL
SFR(DPS,	0x86);
// Not directly accessible DPS Bit. DS89C390 specific
#define SEL         0x01
#define TSL         0x20
#define ID0         0x40
#define ID1         0x80
#endif

#ifdef DPS__ID1__ID0__TSL__AID__x__x__x__SEL
#undef DPS__ID1__ID0__TSL__AID__x__x__x__SEL
SFR(DPS,	0x86);
// Not directly accessible DPS Bit. DS89C420 specific
#define SEL         0x01
#define AID         0x10
#define TSL         0x20
#define ID0         0x40
#define ID1         0x80
#endif

#ifdef DPSEL
#undef DPSEL
SFR(DPSEL,	0x92); // data pointer select register SAB80517
#endif

#ifdef DPX
#undef DPX
SFR(DPX1,	0x93); // DS80C390 specific
#endif

#ifdef DPX1
#undef DPX1
SFR(DPX1,	0x95); // DS80C390 specific
#endif

#ifdef EECON
#undef EECON
SFR(EECON,	0xD2);
#define EEBUSY      0x01
#define EEE         0x02
#define EEPL0       0x10
#define EEPL1       0x20
#define EEPL2       0x40
#define EEPL3       0x80
#define EEPL        0xF0
#endif

#ifdef EETIM
#undef EETIM
SFR(EETIM,	0xD3);
#endif

#ifdef EICC1
#undef EICC1
SFR(EICC1,	0xBF);
#endif

#ifdef EIE__x__x__x__EWDI__EX5__EX4__EX3__EX2
#undef EIE__x__x__x__EWDI__EX5__EX4__EX3__EX2
SFR(EIE,	0xE8);
// Bit registers DS80C320 specific
SBIT(EX2,	0xE8,	0);
SBIT(EX3,	0xE8,	1);
SBIT(EX4,	0xE8,	2);
SBIT(EX5,	0xE8,	3);
SBIT(EWDI,	0xE8,	4);
#endif

#ifdef EIE__CANBIE__C0IE__C1IE__EWDI__EX5__EX4__EX3__EX2
#undef EIE__CANBIE__C0IE__C1IE__EWDI__EX5__EX4__EX3__EX2
SFR(EIE,	0xE8);
// Bit registers DS80C390 specific
SBIT(EX2,		0xE8,	0);
SBIT(EX3,		0xE8,	1);
SBIT(EX4,		0xE8,	2);
SBIT(EX5,		0xE8,	3);
SBIT(EWDI,		0xE8,	4);
SBIT(C1IE,		0xE8,	5);
SBIT(C0IE,		0xE8,	6);
SBIT(CANBIE,	0xE8,	7);
#endif

#ifdef EIP__x__x__x__PWDI__PX5__PX4__PX3__PX2
#undef EIP__x__x__x__PWDI__PX5__PX4__PX3__PX2
SFR(EIP,	0xF8);
// Bit registers DS80C320 specific
SBIT(PX2,	0xF8,	0);
SBIT(PX3,	0xF8,	1);
SBIT(PX4,	0xF8,	2);
SBIT(PX5,	0xF8,	3);
SBIT(PWDI,	0xF8,	4);
#endif

#ifdef EIP__CANBIP__C0IP__C1IP__PWDI__PX5__PX4__PX3__PX2__PX1__PX0
#undef EIP__CANBIP__C0IP__C1IP__PWDI__PX5__PX4__PX3__PX2__PX1__PX0
SFR(EIP,	0xF8);
// Bit registers DS80C320 specific
SBIT(PX2,		0xF8,	0);
SBIT(PX3,		0xF8,	1);
SBIT(PX4,		0xF8,	2);
SBIT(PX5,		0xF8,	3);
SBIT(PWDI,		0xF8,	4);
SBIT(C1IP,		0xF8,	5);
SBIT(C0IP,		0xF8,	6);
SBIT(CANBIP,	0xF8,	7);
#endif

#ifdef EIP0__x__x__x__LPWDI__LPX5__LPX4__LPX3__LPX2
#undef EIP0__x__x__x__LPWDI__LPX5__LPX4__LPX3__LPX2
SFR(EIP0,	0xF8);
// Bit registers DS89C420 specific
SBIT(LPX2,	0xF8,	0);
SBIT(LPX3,	0xF8,	1);
SBIT(LPX4,	0xF8,	2);
SBIT(LPX5,	0xF8,	3);
SBIT(LPWDI,	0xF8,	4);
#endif

#ifdef EIP1__x__x__x__MPWDI__MPX5__MPX4__MPX3__MPX2
#undef EIP1__x__x__x__MPWDI__MPX5__MPX4__MPX3__MPX2
SFR(EIP1,	0xF1);
// Not directly accessible Bits DS89C420 specific
#define MPX2        0x01
#define MPX3        0x02
#define MPX4        0x04
#define MPX5        0x08
#define MPWDI       0x10
#endif

#ifdef ESP
#undef ESP
SFR(ESP,	0x9B);
// Not directly accessible Bits DS80C390 specific
#define ESP_0       0x01
#define ESP_1       0x02
#endif

#ifdef EXIF__IE5__IE4__IE3__IE2__x__RGMD__RGSL__BGS
#undef EXIF__IE5__IE4__IE3__IE2__x__RGMD__RGSL__BGS
SFR(EXIF,	0x91);
// Not directly accessible EXIF Bits DS80C320 specific
#define BGS         0x01
#define RGSL        0x02
#define RGMD        0x04
#define IE2         0x10
#define IE3         0x20
#define IE4         0x40
#define IE5         0x80
#endif

#ifdef EXIF__IE5__IE4__IE3__IE2__XT_RG__RGMD__RGSL__BGS
#undef EXIF__IE5__IE4__IE3__IE2__XT_RG__RGMD__RGSL__BGS
SFR(EXIF,	0x91);
// Not directly accessible EXIF Bits DS87C520 specific
#define BGS         0x01
#define RGSL        0x02
#define RGMD        0x04
#define XT_RG       0x08
#define IE2         0x10
#define IE3         0x20
#define IE4         0x40
#define IE5         0x80
#endif

#ifdef EXIF__IE5__IE4__IE3__IE2__CKRY__RGMD__RGSL__BGS
#undef EXIF__IE5__IE4__IE3__IE2__CKRY__RGMD__RGSL__BGS
SFR(EXIF,	0x91);
// Not directly accessible EXIF Bits DS80C390 & DS89C420 specific
#define BGS         0x01
#define RGSL        0x02
#define RGMD        0x04
#define CKRY        0x08
#define IE2         0x10
#define IE3         0x20
#define IE4         0x40
#define IE5         0x80
#endif

#ifdef FCNTL__FBUSY__FERR__x__x__FC3__FC2__FC1__FC0
#undef FCNTL__FBUSY__FERR__x__x__FC3__FC2__FC1__FC0
SFR(FCNTL,	0xD5);
// Not directly accessible DS89C420 specific
#define FC0         0x01
#define FC1         0x02
#define FC2         0x04
#define FC3         0x08
#define FERR        0x40
#define FBUSY       0x80
#endif

#ifdef FCON
#undef FCON
SFR(FCON,	0xD1);
#define FBUSY       0x01
#define FMOD0       0x02
#define FMOD1       0x04
#define FPS         0x08
#define FPL0        0x10
#define FPL1        0x20
#define FPL2        0x40
#define FPL3        0x80
#define FPL         0xF0
#endif

#ifdef FDATA
#undef FDATA
SFR(FDATA,	0xD6);
#endif

#ifdef FMODE
#undef FMODE
SFR(FMODE,	0xB3);
#endif

#ifdef IE__EA__x__x__ES__ET1__EX1__ET0__EX0
#undef IE__EA__x__x__ES__ET1__EX1__ET0__EX0
SFR(IE,	0xA8);
// Bit registers
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES,	0xA8,	4);
SBIT(EA,	0xA8,	7);
#endif

#ifdef IE__EA__x__ET2__ES__ET1__EX1__ET0__EX0
#undef IE__EA__x__ET2__ES__ET1__EX1__ET0__EX0
SFR(IE,	0xA8);
// Bit registers
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES,	0xA8,	4);
SBIT(ET2,	0xA8,	5); // Enable timer2 interrupt
SBIT(EA,	0xA8,	7);
#endif // IE

#ifdef IE__EA__EAD__ES1__ES0__ET1__EX1__ET0__EX0
#undef IE__EA__EAD__ES1__ES0__ET1__EX1__ET0__EX0
SFR(IE,	0xA8); // same as IEN0 - Interrupt enable 0, P80C552 specific
SFR(IEN0,	0xA8); // alternate name
// Bit registers
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES0,	0xA8,	4);
SBIT(ES1,	0xA8,	5);
SBIT(EAD,	0xA8,	6);
SBIT(EEA,	0xA8,	7);
#endif

#ifdef IE__EA__EC__ET2__ES__ET1__EX1__ET0__EX0
#undef IE__EA__EC__ET2__ES__ET1__EX1__ET0__EX0
SFR(IE,	0xA8);
// Bit registers
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES,	0xA8,	4);
SBIT(ET2,	0xA8,	5);
SBIT(EC,	0xA8,	6);
SBIT(EA,	0xA8,	7);
#endif

#ifdef IE__EA__ES1__ET2__ES__ET1__EX1__ET0__EX0
#undef IE__EA__ES1__ET2__ES__ET1__EX1__ET0__EX0
SFR(IE,	0xA8);
// Bit registers
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES,	0xA8,	4);
SBIT(ES0,	0xA8,	4); // Alternate name
SBIT(ET2,	0xA8,	5); // Enable timer2 interrupt
SBIT(ES1,	0xA8,	6);
SBIT(EA,	0xA8,	7);
#endif // IE

#ifdef IE__EA_WDT_ET2_ES_ET1_EX1_ET0_EX0
#undef IE__EA_WDT_ET2_ES_ET1_EX1_ET0_EX0
SFR(IE,	0xA8);
SFR(IEN0,	0xA8); // Alternate name
// Bit registers for the SAB80515 and compatible IE
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES,	0xA8,	4);
SBIT(ES0,	0xA8,	4);
SBIT(ET2,	0xA8,	5); // Enable timer 2 overflow SAB80515 specific
SBIT(WDT,	0xA8,	6); // watchdog timer reset - SAB80515 specific
SBIT(EA,	0xA8,	7);
SBIT(EAL,	0xA8,	7); // EA as called by Infineon / Siemens
#endif

#ifdef IEN0__EA__EC__ES1__ES0__ET1__EX1__ET0__EX0
#undef IEN0__EA__EC__ES1__ES0__ET1__EX1__ET0__EX0
// P89C668 specific
SFR(IEN0,	0xA8);
// Bit registers
SBIT(EX0,	0xA8,	0);
SBIT(ET0,	0xA8,	1);
SBIT(EX1,	0xA8,	2);
SBIT(ET1,	0xA8,	3);
SBIT(ES0,	0xA8,	4);
SBIT(ES1,	0xA8,	5);
SBIT(EC,	0xA8,	6);
SBIT(EA,	0xA8,	7);
#endif

#ifdef IEN1__x__x__x__x__x__x__x__ET2
#undef IEN1__x__x__x__x__x__x__x__ET2
// P89C668 specific bit registers
SFR(IEN1,	0xE8);
// Bit registers
SBIT(ET2,	0xE8,	0);
#endif

#ifdef IEN1__ET2__ECM2__ECM1__ECM0__ECT3__ECT2__ECT1__ECT0
#undef IEN1__ET2__ECM2__ECM1__ECM0__ECT3__ECT2__ECT1__ECT0
SFR(IEN1,	0xE8); // Interrupt enable 1, P80C552 specific
// Bit registers
SBIT(ECT0,	0xE8,	0);
SBIT(ECT1,	0xE8,	1);
SBIT(ECT2,	0xE8,	2);
SBIT(ECT3,	0xE8,	3);
SBIT(ECM0,	0xE8,	4);
SBIT(ECM1,	0xE8,	5);
SBIT(ECM2,	0xE8,	6);
SBIT(ET2,	0xE8,	7);
#endif

#ifdef IEN1__EXEN2__SWDT__EX6__EX5__EX4__EX3__EX2__EADC
#undef IEN1__EXEN2__SWDT__EX6__EX5__EX4__EX3__EX2__EADC
SFR(IEN1,	0xB8); // interrupt enable register - SAB80515 specific
// Bit registers
SBIT(EADC,	0xB8,	0); // A/D converter interrupt enable
SBIT(EX2,	0xB8,	1);
SBIT(EX3,	0xB8,	2);
SBIT(EX4,	0xB8,	3);
SBIT(EX5,	0xB8,	4);
SBIT(EX6,	0xB8,	5);
SBIT(SWDT,	0xB8,	6); // watchdog timer start/reset
SBIT(EXEN2,	0xB8,	7); // timer2 external reload interrupt enable
#endif

#ifdef IEN2__SAB80517
#undef IEN2__SAB80517
SFR(IEN2,	0x9A); // interrupt enable register 2 SAB80517
#endif

#ifdef IEN3
#undef IEN3
SFR(IEN3,	0xBE);
#endif

#ifdef IP__x__x__x__PS__PT1__PX1__PT0__PX0
#undef IP__x__x__x__PS__PT1__PX1__PT0__PX0
SFR(IP,	0xB8);
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS,	0xB8,	4);
#endif

#ifdef IP__x__x__PT2__PS__PT1__PX1__PT0__PX0
#undef IP__x__x__PT2__PS__PT1__PX1__PT0__PX0
SFR(IP,	0xB8);
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS,	0xB8,	4);
SBIT(PS0,	0xB8,	4);  // alternate name
SBIT(PT2,	0xB8,	5);
#endif

#ifdef IP__x__PAD__PS1__PS0__PT1__PX1__PT0__PX0
#undef IP__x__PAD__PS1__PS0__PT1__PX1__PT0__PX0
SFR(IP,	0xB8); // Interrupt priority 0, P80C552 specific
SFR(IP0,	0xB8); // alternate name
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS0,	0xB8,	4);
SBIT(PS1,	0xB8,	5);
SBIT(PAD,	0xB8,	6);
#endif

#ifdef IP__x__PPC__PT2__PS__PT1__PX1__PT0__PX0
#undef IP__x__PPC__PT2__PS__PT1__PX1__PT0__PX0
SFR(IP,	0xB8);
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS,	0xB8,	4);
SBIT(PT2,	0xB8,	5);
SBIT(PPC,	0xB8,	6);
#endif

#ifdef IP__x__PS1__PT2__PS__PT1_PX1__PT0__PX0
#undef IP__x__PS1__PT2__PS__PT1_PX1__PT0__PX0
SFR(IP,	0xB8);
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS,	0xB8,	4);
SBIT(PT2,	0xB8,	5);
SBIT(PS1,	0xB8,	6);
#endif

#ifdef IP__PT2__PPC__PS1__PS0__PT1__PX1__PT0__PX0
#undef IP__PT2__PPC__PS1__PS0__PT1__PX1__PT0__PX0
// P89C668 specific:
SFR(IP,	0xB8);
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS0,	0xB8,	4);
SBIT(PS1,	0xB8,	5);
SBIT(PPC,	0xB8,	6);
SBIT(PT2,	0xB8,	7);
#endif

#ifdef IP__RWT__x__x__PS__PT1__PX1__PT0__PX0
#undef IP__RWT__x__x__PS__PT1__PX1__PT0__PX0
SFR(IP,	0xB8);
// Bit registers
SBIT(PX0,	0xB8,	0);
SBIT(PT0,	0xB8,	1);
SBIT(PX1,	0xB8,	2);
SBIT(PT1,	0xB8,	3);
SBIT(PS,	0xB8,	4);
SBIT(RWT,	0xB8,	7);
#endif

#ifdef IP0__x__WDTS__IP0_5__IP0_4__IP0_3__IP0_2__IP0_1__IP0_0
#undef IP0__x__WDTS__IP0_5__IP0_4__IP0_3__IP0_2__IP0_1__IP0_0
SFR(IP0,	0xA9); // interrupt priority register SAB80515 specific
// Not directly accessible IP0 bits
#define IP0_0       0x01
#define IP0_1       0x02
#define IP0_2       0x04
#define IP0_3       0x08
#define IP0_4       0x10
#define IP0_5       0x20
#define WDTS        0x40
#endif

#ifdef IP0__x__LPS1__LPT2__LPS0__LPT1__LPX1__LPT0__LPX0
#undef IP0__x__LPS1__LPT2__LPS0__LPT1__LPX1__LPT0__LPX0
SFR(IP0,	0xB8); // interrupt priority register DS89C420 specific
// Bit registers
SBIT(LPX0,	0xB8,	0);
SBIT(LPT0,	0xB8,	1);
SBIT(LPX1,	0xB8,	2);
SBIT(LPT1,	0xB8,	3);
SBIT(LPS0,	0xB8,	4);
SBIT(LPT2,	0xB8,	5);
SBIT(LPS1,	0xB8,	6);
#endif

#ifdef IP1__x__x__IP1_5__IP1_4__IP1_3__IP1_2__IP1_1__IP1_0
#undef IP1__x__x__IP1_5__IP1_4__IP1_3__IP1_2__IP1_1__IP1_0
SFR(IP1,	0xB9); // interrupt priority register SAB80515 specific
// Not directly accessible IP1 bits
#define IP1_0       0x01
#define IP1_1       0x02
#define IP1_2       0x04
#define IP1_3       0x08
#define IP1_4       0x10
#define IP1_5       0x20
#endif

#ifdef IP1__x__MPS1__MPT2__MPS0__MPT1__MPX1__MPT0__MPX0
#undef IP1__x__MPS1__MPT2__MPS0__MPT1__MPX1__MPT0__MPX0
SFR(IP1,	0xB1); // interrupt priority register DS89C420 specific
// Not directly accessible IP1 bits
#define MPX0        0x01
#define MPT0        0x02
#define MPX1        0x04
#define MPT1        0x08
#define MPS0        0x10
#define MPT2        0x20
#define MPS1        0x40
#endif

#ifdef IP1__PT2__PCM2__PCM1__PCM0__PCT3__PCT2__PCT1__PCT0
#undef IP1__PT2__PCM2__PCM1__PCM0__PCT3__PCT2__PCT1__PCT0
SFR(IP1,	0xF8); // Interrupt priority 1, P80C552 specific
// Bit registers
SBIT(PCT0,	0xF8,	0);
SBIT(PCT1,	0xF8,	1);
SBIT(PCT2,	0xF8,	2);
SBIT(PCT3,	0xF8,	3);
SBIT(PCM0,	0xF8,	4);
SBIT(PCM1,	0xF8,	5);
SBIT(PCM2,	0xF8,	6);
SBIT(PT2,	0xF8,	7);
#endif

#ifdef IPH__x__PPCH__PT2H__PSH__PT1H__PX1H__PT0H__PX0H
#undef IPH__x__PPCH__PT2H__PSH__PT1H__PX1H__PT0H__PX0H
SFR(IPH,	0xB7);
#define PX0H        0x01
#define PT0H        0x02
#define PX1H        0x04
#define PT1H        0x08
#define PSH         0x10
#define PT2H        0x20
#define PPCH        0x40
#endif

#ifdef IPH__PT2H__PPCH__PS1H__PS0H__PT1H__PX1H__PT0H__PX0H
#undef IPH__PT2H__PPCH__PS1H__PS0H__PT1H__PX1H__PT0H__PX0H
// P89C668 specific:
SFR(IPH,	0xB7);
// not bit addressable:
#define PX0H        0x01
#define PT0H        0x02
#define PX1H        0x04
#define PT1H        0x08
#define PS0H        0x10
#define PS1H        0x20
#define PPCH        0x40
#define PT2H        0x80
#endif

#ifdef IRCON
#undef IRCON
SFR(IRCON,	0xC0); // interrupt control register - SAB80515 specific
// Bit registers
SBIT(IADC,	0xC0,	0); // A/D converter irq flag
SBIT(IEX2,	0xC0,	1); // external interrupt edge detect flag
SBIT(IEX3,	0xC0,	2);
SBIT(IEX4,	0xC0,	3);
SBIT(IEX5,	0xC0,	4);
SBIT(IEX6,	0xC0,	5);
SBIT(TF2,	0xC0,	6); // timer 2 owerflow flag
SBIT(EXF2,	0xC0,	7); // timer2 reload flag
#endif

#ifdef IRCON0
#undef IRCON0
SFR(IRCON0,	0xC0); // interrupt control register - SAB80515 specific
// Bit registers
SBIT(IADC,	0xC0,	0); // A/D converter irq flag
SBIT(IEX2,	0xC0,	1); // external interrupt edge detect flag
SBIT(IEX3,	0xC0,	2);
SBIT(IEX4,	0xC0,	3);
SBIT(IEX5,	0xC0,	4);
SBIT(IEX6,	0xC0,	5);
SBIT(TF2,	0xC0,	6); // timer 2 owerflow flag
SBIT(EXF2,	0xC0,	7); // timer2 reload flag
#endif

#ifdef IRCON1
#undef IRCON1
SFR(IRCON1,	0xD1); // interrupt control register - SAB80515 specific
#endif

#ifdef IRCON2
#undef IRCON2
SFR(IRCON2,	0xBF);
#endif

#ifdef MA
#undef MA
SFR(MA,	0xD3); // DS80C390
#endif

#ifdef MB
#undef MB
SFR(MB,	0xD4); // DS80C390
#endif

#ifdef MC
#undef MC
SFR(MC,	0xD5); // DS80C390
#endif

#ifdef MCNT0
#undef MCNT0
SFR(MCNT0,	0xD1); // DS80C390
#define MAS0        0x01
#define MAS1        0x02
#define MAS2        0x04
#define MAS3        0x08
#define MAS4        0x10
#define SCB         0x20
#define CSE         0x40
#define LSHIFT      0x80
#endif

#ifdef MCNT1
#undef MCNT1
SFR(MCNT1,	0xD2); // DS80C390
#define CLM         0x10
#define MOF         0x40
#define MST         0x80
#endif

#ifdef MCON__IDM1__IDM0__CMA__x__PDCE3__PDCE2__PDCE1__PDCE0
#undef MCON__IDM1__IDM0__CMA__x__PDCE3__PDCE2__PDCE1__PDCE0
SFR(MCON,	0xC6); // DS80C390
#define PDCE0       0x01
#define PDCE1       0x02
#define PDCE2       0x04
#define PDCE3       0x08
#define CMA         0x20
#define IDM0        0x40
#define IDM1        0x80
#endif

#ifdef MCON__PA3__PA2__PA1__PA0__RA32_8__ECE2__PAA__SL
#undef MCON__PA3__PA2__PA1__PA0__RA32_8__ECE2__PAA__SL
SFR(MCON,	0xC6); // DS5000
#define SL          0x01
#define PAA         0x02
#define ECE2        0x04
#define RA32_8      0x08
#define PA0         0x10
#define PA1         0x20
#define PA2         0x40
#define PA3         0x80
#endif

#ifdef MCON__PA3__PA2__PA1__PA0__RG1__PES__PM__SL
#undef MCON__PA3__PA2__PA1__PA0__RG1__PES__PM__SL
SFR(MCON,	0xC6); // DS5001
#define SL          0x01
#define PM          0x02
#define PES         0x04
#define RG1         0x08
#define PA0         0x10
#define PA1         0x20
#define PA2         0x40
#define PA3         0x80
#endif

#ifdef MD0
#undef MD0
SFR(MD0,	0xE9); // MUL / DIV register 0 SAB80517
#endif

#ifdef MD1
#undef MD1
SFR(MD1,	0xEA); // MUL / DIV register 1 SAB80517
#endif

#ifdef MD2
#undef MD2
SFR(MD2,	0xEB); // MUL / DIV register 2 SAB80517
#endif

#ifdef MD3
#undef MD3
SFR(MD3,	0xEC); // MUL / DIV register 3 SAB80517
#endif

#ifdef MD4
#undef MD4
SFR(MD4,	0xED); // MUL / DIV register 4 SAB80517
#endif

#ifdef MD5
#undef MD5
SFR(MD5,	0xEE); // MUL / DIV register 5 SAB80517
#endif

#ifdef MXAX
#undef MXAX
SFR(MXAX,	0xEA); // Dallas DS80C390
#endif

#ifdef P0
#undef P0
SFR(P0,	0x80);
//  Bit Registers
SBIT(P0_0,	0x80,	0);
SBIT(P0_1,	0x80,	1);
SBIT(P0_2,	0x80,	2);
SBIT(P0_3,	0x80,	3);
SBIT(P0_4,	0x80,	4);
SBIT(P0_5,	0x80,	5);
SBIT(P0_6,	0x80,	6);
SBIT(P0_7,	0x80,	7);
#endif

#ifdef P0_EXT__AD7__AD6__AD5__AD4__AD3__AD2__AD1__AD0
#undef P0_EXT__AD7__AD6__AD5__AD4__AD3__AD2__AD1__AD0
// P89C668 alternate names for bits in P0
SBIT(AD0,	0x80,	0);
SBIT(AD1,	0x80,	1);
SBIT(AD2,	0x80,	2);
SBIT(AD3,	0x80,	3);
SBIT(AD4,	0x80,	4);
SBIT(AD5,	0x80,	5);
SBIT(AD6,	0x80,	6);
SBIT(AD7,	0x80,	7);
#endif

#ifdef P1
#undef P1
SFR(P1,	0x90);
// Bit registers
SBIT(P1_0,	0x90,	0);
SBIT(P1_1,	0x90,	1);
SBIT(P1_2,	0x90,	2);
SBIT(P1_3,	0x90,	3);
SBIT(P1_4,	0x90,	4);
SBIT(P1_5,	0x90,	5);
SBIT(P1_6,	0x90,	6);
SBIT(P1_7,	0x90,	7);
#endif

#ifdef P1_EXT__INT5__INT4__INT3__INT2__TXD1__RXD1__T2EX__T2
#undef P1_EXT__INT5__INT4__INT3__INT2__TXD1__RXD1__T2EX__T2
// P1 alternate functions
SBIT(T2,	0x90,	0);
SBIT(T2EX,	0x90,	1);
SBIT(RXD1,	0x90,	2);
SBIT(TXD1,	0x90,	3);
SBIT(INT2,	0x90,	4);
SBIT(INT3,	0x90,	5);
SBIT(INT4,	0x90,	6);
SBIT(INT5,	0x90,	7);
#endif

#ifdef P1_EXT__SDA__SCL__CEX2__CEX1__CEX0__ECI__T2EX__T2
#undef P1_EXT__SDA__SCL__CEX2__CEX1__CEX0__ECI__T2EX__T2
// P89C669 alternate names for bits __at P1
// P1_EXT__SDA__SCL__CEX2__CEX1__CEX0__ECI__T2EX__T2
SBIT(T2,	0x90,	0);
SBIT(T2EX,	0x90,	1);
SBIT(ECI,	0x90,	2);
SBIT(CEX0,	0x90,	3);
SBIT(CEX1,	0x90,	4);
SBIT(CEX2,	0x90,	5);
SBIT(SCL,	0x90,	6);
SBIT(SDA,	0x90,	7);
#endif

#ifdef P1_EXT__T2__CLKOUT__T2EX__INT2__INT6_CC3__INT5_CC2__INT4_CC1__INT3_CC0
SBIT(INT3_CC0,	0x90,	0); // P1 alternate functions - SAB80515 specific
SBIT(INT4_CC1,	0x90,	1);
SBIT(INT5_CC2,	0x90,	2);
SBIT(INT6_CC3,	0x90,	3);
SBIT(INT2,		0x90,	4);
SBIT(T2EX,		0x90,	5);
SBIT(CLKOUT,	0x90,	6);
SBIT(T2,		0x90,	7);
#endif

#ifdef P1_EXT__CT0I__CT1I__CT2I__CT3I__T2__RT2__SCL__SDA
#undef P1_EXT__CT0I__CT1I__CT2I__CT3I__T2__RT2__SCL__SDA
// Bit registers
SBIT(CT0I,	0x90,	0); // Port 1 alternate functions, P80C552 specific
SBIT(CT1I,	0x90,	1);
SBIT(CT2I,	0x90,	2);
SBIT(CT3I,	0x90,	3);
SBIT(T2,	0x90,	4);
SBIT(RT2,	0x90,	5);
SBIT(SCL,	0x90,	6);
SBIT(SDA,	0x90,	7);
#endif

#ifdef P1_EXT__x__x__x__x__x__x__T2EX__T2
#undef P1_EXT__x__x__x__x__x__x__T2EX__T2
// P1 alternate functions
SBIT(T2,	0x90,	0);
SBIT(T2EX,	0x90,	1);
#endif

#ifdef P2
#undef P2
SFR(P2,	0xA0);
// Bit registers
SBIT(P2_0,	0xA0,	0);
SBIT(P2_1,	0xA0,	1);
SBIT(P2_2,	0xA0,	2);
SBIT(P2_3,	0xA0,	3);
SBIT(P2_4,	0xA0,	4);
SBIT(P2_5,	0xA0,	5);
SBIT(P2_6,	0xA0,	6);
SBIT(P2_7,	0xA0,	7);
#endif

#ifdef P2_EXT__AD15__AD14__AD13__AD12__AD11__AD10__AD9__AD8
#undef P2_EXT__AD15__AD14__AD13__AD12__AD11__AD10__AD9__AD8
// P89C668 specific bit registers __at P2:
SBIT(AD8,	0xA0,	0);
SBIT(AD9,	0xA0,	1);
SBIT(AD10,	0xA0,	2);
SBIT(AD11,	0xA0,	3);
SBIT(AD12,	0xA0,	4);
SBIT(AD13,	0xA0,	5);
SBIT(AD14,	0xA0,	6);
SBIT(AD15,	0xA0,	7);
#endif

#ifdef P3
#undef P3
SFR(P3,	0xB0);
// Bit registers
SBIT(P3_0,	0xB0,	0);
SBIT(P3_1,	0xB0,	1);
SBIT(P3_2,	0xB0,	2);
SBIT(P3_3,	0xB0,	3);
SBIT(P3_4,	0xB0,	4);
SBIT(P3_5,	0xB0,	5);
#ifndef MCS51REG_EXTERNAL_RAM
SBIT(P3_6,	0xB0,	6);
SBIT(P3_7,	0xB0,	7);
#endif
// alternate names
SBIT(RXD,	0xB0,	0);
SBIT(RXD0,	0xB0,	0);
SBIT(TXD,	0xB0,	1);
SBIT(TXD0,	0xB0,	1);
SBIT(INT0,	0xB0,	2);
SBIT(INT1,	0xB0,	3);
SBIT(T0,	0xB0,	4);
SBIT(T1,	0xB0,	5);
#ifndef MCS51REG_EXTERNAL_RAM
SBIT(WR,	0xB0,	6);
SBIT(RD,	0xB0,	7);
#endif
#endif

#ifdef P3_EXT__x__x__CEX4__CEX3__x__x__x__x
#undef P3_EXT__x__x__CEX4__CEX3__x__x__x__x
// P89C668 specific bit registers __at P3 (alternate names)
SBIT(CEX4,	0xB0,	5);
SBIT(CEX3,	0xB0,	4);
#endif

#ifdef P4_AT_0X80
#undef P4_AT_0X80
SFR(P4,	0x80); // Port 4 - DS80C390
// Bit registers
SBIT(P4_0,	0x80,	0);
SBIT(P4_1,	0x80,	1);
SBIT(P4_2,	0x80,	2);
SBIT(P4_3,	0x80,	3);
SBIT(P4_4,	0x80,	4);
SBIT(P4_5,	0x80,	5);
SBIT(P4_6,	0x80,	6);
SBIT(P4_7,	0x80,	7);
#endif

#ifdef P4_AT_0XC0__CMT0__CMT1__CMSR5__CMSR4__CMSR3__CMSR2__CMSR1__CMSR0
#undef P4_AT_0XC0__CMT0__CMT1__CMSR5__CMSR4__CMSR3__CMSR2__CMSR1__CMSR0
SFR(P4,	0xC0); // Port 4, P80C552 specific
// Bit registers
SBIT(CMSR0,	0xC0,	0);
SBIT(CMSR1,	0xC0,	1);
SBIT(CMSR2,	0xC0,	2);
SBIT(CMSR3,	0xC0,	3);
SBIT(CMSR4,	0xC0,	4);
SBIT(CMSR5,	0xC0,	5);
SBIT(CMT0,	0xC0,	6);
SBIT(CMT1,	0xC0,	7);
#endif

#ifdef P4_AT_0XC0__P4_7__P4_6__P4_5__P4_3__P4_2__P4_1__P4_0
#undef P4_AT_0XC0__P4_7__P4_6__P4_5__P4_3__P4_2__P4_1__P4_0
SFR(P4,	0xC0); // Port 4, T89C51 specific
// Bit registers
SBIT(P4_0,	0xC0,	0);
SBIT(P4_1,	0xC0,	1);
SBIT(P4_2,	0xC0,	2);
SBIT(P4_3,	0xC0,	3);
SBIT(P4_4,	0xC0,	4);
SBIT(P4_5,	0xC0,	5);
SBIT(P4_6,	0xC0,	6);
SBIT(P4_7,	0xC0,	7);
#endif

#ifdef P4_AT_0XE8
#undef P4_AT_0XE8
SFR(P4,	0xE8); // Port 4 - SAB80515 & compatible microcontrollers
// Bit registers
SBIT(P4_0,	0xE8,	0);
SBIT(P4_1,	0xE8,	1);
SBIT(P4_2,	0xE8,	2);
SBIT(P4_3,	0xE8,	3);
SBIT(P4_4,	0xE8,	4);
SBIT(P4_5,	0xE8,	5);
SBIT(P4_6,	0xE8,	6);
SBIT(P4_7,	0xE8,	7);
#endif

#ifdef P4CNT
#undef P4CNT
SFR(P4CNT,	0x92); // DS80C390
// Not directly accessible bits
#define P4CNT_0     0x01
#define P4CNT_1     0x02
#define P4CNT_2     0x04
#define P4CNT_3     0x08
#define P4CNT_4     0x10
#define P4CNT_5     0x20
#define SBCAN       0x40
#endif

#ifdef P5_AT_0XA1
#undef P5_AT_0XA1
SFR(P5,	0xA1); // Port 5 - DS80C390
#endif

#ifdef P5_AT_0XE8
#undef P5_AT_0XE8
SFR(P5,	0xE8); // Port 5 - T89C51RD2
// Bit registers
SBIT(P5_0,	0xE8,	0);
SBIT(P5_1,	0xE8,	1);
SBIT(P5_2,	0xE8,	2);
SBIT(P5_3,	0xE8,	3);
SBIT(P5_4,	0xE8,	4);
SBIT(P5_5,	0xE8,	5);
SBIT(P5_6,	0xE8,	6);
SBIT(P5_7,	0xE8,	7);
#endif

#ifdef P5CNT
#undef P5CNT
SFR(P5CNT,	0xA2); // DS80C390
// Not directly accessible bits
#define P5CNT_0     0x01
#define P5CNT_1     0x02
#define P5CNT_2     0x04
#define C0_I_O      0x08
#define C1_I_O      0x10
#define SP1EC       0x20
#define SBCAN0BA    0x40
#define SBCAN1BA    0x80
#endif

#ifdef P5_AT_0XC4
#undef P5_AT_0XC4
SFR(P5,	0xC4); // Port 5, P80C552 specific
// Not directly accessible Bits.
#define ADC0        0x01
#define ADC1        0x02
#define ADC2        0x04
#define ADC3        0x08
#define ADC4        0x10
#define ADC5        0x20
#define ADC6        0x40
#define ADC7        0x80
#endif

#ifdef P5_AT_0XF8
#undef P5_AT_0XF8
SFR(P5,	0xF8); // Port 5 - SAB80515 & compatible microcontrollers
// Bit registers
SBIT(P5_0,	0xF8,	0);
SBIT(P5_1,	0xF8,	1);
SBIT(P5_2,	0xF8,	2);
SBIT(P5_3,	0xF8,	3);
SBIT(P5_4,	0xF8,	4);
SBIT(P5_5,	0xF8,	5);
SBIT(P5_6,	0xF8,	6);
SBIT(P5_7,	0xF8,	7);
#endif

#ifdef P6_AT_0XDB
#undef P6_AT_0XDB
SFR(P6,	0xDB); // Port 6 - SAB80515 & compatible microcontrollers
#endif

#ifdef P6_AT_0XFA
#undef P6_AT_0XFA
SFR(P6,	0xFA); // Port 6 - SAB80517 specific
#endif

#ifdef P7_AT_0XDB
#undef P7_AT_0XDB
SFR(P7,	0xDB); // Port 7 - SAB80517 specific
#endif

#ifdef P8_AT_0XDD
#undef P8_AT_0XDD
SFR(P8,	0xDD); // Port 6 - SAB80517 specific
#endif

#ifdef P9_AT_0XF9
#undef P9_AT_0XF9
SFR(P9,	0xF9);
#endif

#ifdef PCON__SMOD__x__x__x__x__x__x__x
#undef PCON__SMOD__x__x__x__x__x__x__x
SFR(PCON,	0x87);
// Not directly accessible PCON bits
#define SMOD        0x80
#endif

#ifdef PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
#undef PCON__SMOD__x__x__x__GF1__GF0__PD__IDL
SFR(PCON,	0x87);
// Not directly accessible PCON bits
#define IDL         0x01
#define PD          0x02
#define GF0         0x04
#define GF1         0x08
#define SMOD        0x80
#endif

#ifdef PCON__SMOD__x__x__WLE__GF1__GF0__PD__IDL
#undef PCON__SMOD__x__x__WLE__GF1__GF0__PD__IDL
SFR(PCON,	0x87); // PCON, P80C552 specific
// Not directly accessible Bits.
#define IDL         0x01
#define IDLE        0x01 /* same as IDL */
#define PD          0x02
#define GF0         0x04
#define GF1         0x08
#define WLE         0x10
#define SMOD        0x80
#endif

#ifdef PCON__SMOD__PDS__IDLS__x__x__x__PD__IDL
#undef PCON__SMOD__PDS__IDLS__x__x__x__PD__IDL
SFR(PCON,	0x87);
// Not directly accessible PCON bits
#define IDL         0x01
#define IDLE        0x01 /* same as IDL */
#define PD          0x02
#define PDE         0x02 /* same as PD */
#define IDLS        0x20
#define PDS         0x40
#define SMOD        0x80
// alternate names
#define PCON_IDLE   0x01
#define PCON_PDE    0x02
#define PCON_GF0    0x04
#define PCON_GF1    0x08
#define PCON_IDLS   0x20
#define PCON_PDS    0x40
#define PCON_SMOD   0x80
#endif

#ifdef PCON__SMOD__POR__PFW__WTR__EPFW__EWT__STOP__IDL
#undef PCON__SMOD__POR__PFW__WTR__EPFW__EWT__STOP__IDL
SFR(PCON,	0x87);
// Not directly accessible PCON bits
#define IDL         0x01
#define IDLE        0x01 /* same as IDL */
#define STOP        0x02
#define EWT         0x04
#define EPFW        0x08
#define WTR         0x10
#define PFW         0x20
#define POR         0x40
#define SMOD        0x80
#endif

#ifdef PCON__SMOD__SMOD0__x__x__GF1__GF0__STOP__IDLE
#undef PCON__SMOD__SMOD0__x__x__GF1__GF0__STOP__IDLE
SFR(PCON,	0x87);
// Not directly accessible PCON bits
#define IDL         0x01
#define IDLE        0x01  /* same as IDL */
#define STOP        0x02
#define GF0         0x04
#define GF1         0x08
#define SMOD0       0x40
#define SMOD        0x80
#endif

#ifdef PCON__SMOD__SMOD0__OFDF__OFDE__GF1__GF0__STOP__IDLE
#undef PCON__SMOD__SMOD0__OFDF__OFDE__GF1__GF0__STOP__IDLE
SFR(PCON,	0x87);
// Not directly accessible PCON bits
#define IDL         0x01
#define IDLE        0x01 /* same as IDL */
#define STOP        0x02
#define GF0         0x04
#define GF1         0x08
#define OFDE        0x10
#define OFDF        0x20
#define SMOD0       0x40
#define SMOD        0x80
#define SMOD_0      0x80 /* same as SMOD */
#endif

#ifdef PCON__SMOD1__SMOD0__x__POF__GF1__GF0__PD__IDL
#undef PCON__SMOD1__SMOD0__x__POF__GF1__GF0__PD__IDL
SFR(PCON,	0x87);
#define IDL         0x01
#define PD          0x02
#define GF0         0x04
#define GF1         0x08
#define POF         0x10
#define SMOD0       0x40
#define SMOD1       0x80
#endif

#ifdef PMR__CD1__CD0__SWB__x__XTOFF__ALEOFF__DME1__DME0
#undef PMR__CD1__CD0__SWB__x__XTOFF__ALEOFF__DME1__DME0
SFR(PMR,	0xC4); // DS87C520, DS83C520
// Not directly accessible bits
#define DME0        0x01
#define DME1        0x02
#define ALEOFF      0x04
#define XTOFF       0x08
#define SWB         0x20
#define CD0         0x40
#define CD1         0x80
#endif

#ifdef PMR__CD1__CD0__SWB__CTM__4X_2X__ALEOFF__x__x
#undef PMR__CD1__CD0__SWB__CTM__4X_2X__ALEOFF__x__x
SFR(PMR,	0xC4); // DS80C390
// Not directly accessible bits
#define ALEOFF      0x04
#define XTOFF       0x08
#define _4X_2X      0x10
#define SWB         0x20
#define CD0         0x40
#define CD1         0x80
#endif

#ifdef PMR__CD1__CD0__SWB__CTM__4X_2X__ALEON__DME1__DME0
#undef PMR__CD1__CD0__SWB__CTM__4X_2X__ALEON__DME1__DME0
SFR(PMR,	0xC4); // DS89C420
// Not directly accessible bits
#define DME0        0x01
#define DME1        0x02
#define ALEON       0x04
#define _4X_2X      0x08
#define CTM         0x10
#define SWB         0x20
#define CD0         0x40
#define CD1         0x80
#endif

#ifdef PRSC
#undef PRSC
SFR(PRSC,	0xB4);
#endif

#ifdef PSW
#undef PSW
SFR(PSW,	0xD0);
// Bit registers
SBIT(P,		0xD0,	0);
SBIT(F1,	0xD0,	1);
SBIT(OV,	0xD0,	2);
SBIT(RS0,	0xD0,	3);
SBIT(RS1,	0xD0,	4);
SBIT(F0,	0xD0,	5);
SBIT(AC,	0xD0,	6);
SBIT(CY,	0xD0,	7);
#endif

#ifdef PWM0_AT_0XFC
#undef PWM0_AT_0XFC
SFR(PWM0,	0xFC); // PWM register 0, P80C552 specific
#endif

#ifdef PWM1_AT_0XFD
#undef PWM1_AT_0XFD
SFR(PWM1,	0xFD); // PWM register 1, P80C552 specific
#endif

#ifdef PWMP_AT_0XFE
#undef PWMP_AT_0XFE
SFR(PWMP,	0xFE); // PWM prescaler, P80C552 specific
#endif

#ifdef RCAP2H
#undef RCAP2H
SFR(RCAP2H,	0xCB);
#endif

#ifdef RCAP2L
#undef RCAP2L
SFR(RCAP2L,	0xCA);
#endif

#ifdef RNR
#undef RNR
SFR(RNR,	0xCF);
#endif

#ifdef ROMSIZE__x__x__x__x__x__RMS2__RMS1__RMS0
#undef ROMSIZE__x__x__x__x__x__RMS2__RMS1__RMS0
SFR(ROMSIZE,	0xC2); // DS87C520, DS83C520
// Not directly accessible bits
#define RSM0        0x01
#define RSM1        0x02
#define RSM2        0x04
#endif

#ifdef ROMSIZE__x__x__x__x__PRAME__RMS2__RMS1__RMS0
#undef ROMSIZE__x__x__x__x__PRAME__RMS2__RMS1__RMS0
SFR(ROMSIZE,	0xC2); // DS89C420
// Not directly accessible bits
#define RSM0        0x01
#define RSM1        0x02
#define RSM2        0x04
#define PRAME       0x08
#endif

#ifdef ROMSIZE__HBPF__BPF__TE__MOVCX__PRAME__RMS2__RMS1__RMS0
#undef ROMSIZE__HBPF__BPF__TE__MOVCX__PRAME__RMS2__RMS1__RMS0
SFR(ROMSIZE,	0xC2); // DS87C520, DS83C520
// Not directly accessible bits
#define RSM0        0x01
#define RSM1        0x02
#define RSM2        0x04
#define PRAME       0x08
#define MOVCX       0x10
#define TE          0x20
#define BPF         0x40
#define HBPF        0x80
#endif

#ifdef RPCTL
#undef RPCTL
SFR(RPCTL,		0xD8); // Dallas DS5001 specific
// Bit registers
SBIT(RG0,		0xD8,	0);
SBIT(RPCON,		0xD8,	1);
SBIT(DMA,		0xD8,	2);
SBIT(IBI,		0xD8,	3);
SBIT(AE,		0xD8,	4);
SBIT(EXBS,		0xD8,	5);
SBIT(RNR_FLAG,	0xD8,	7);
#endif

#ifdef RTE__TP47__TP46__RP45__RP44__RP43__RP42__RP41__RP40
#undef RTE__TP47__TP46__RP45__RP44__RP43__RP42__RP41__RP40
SFR(RTE,	0xEF); // Reset/toggle enable, P80C552 specific
// Not directly accessible Bits.
#define RP40        0x01
#define RP41        0x02
#define RP42        0x04
#define RP43        0x08
#define RP44        0x10
#define RP45        0x20
#define TP46        0x40
#define TP47        0x80
#endif

#ifdef S0BUF
#undef S0BUF
SFR(S0BUF,	0x99); // serial channel 0 buffer register SAB80517 specific
#endif

#ifdef S0CON__SM0__SM1__SM2__REN__TB8__RB8__TI__RI
#undef S0CON__SM0__SM1__SM2__REN__TB8__RB8__TI__RI
SFR(S0CON,	0x98); // serial channel 0 control register P80C552 specific
// Bit registers
// Already defined in SCON
//SBIT(RI0,	0x98,	0);
//SBIT(TI0,	0x98,	1);
//SBIT(RB8,	0x98,	2);
//SBIT(TB8,	0x98,	3);
//SBIT(REN,	0x98,	4);
//SBIT(SM2,	0x98,	5);
//SBIT(SM1,	0x98,	6);
//SBIT(SM0,	0x98,	7);
#endif

#ifdef S0CON__SM0__SM1__SM20__REN0__TB80__RB80__TI0__RI0
#undef S0CON__SM0__SM1__SM20__REN0__TB80__RB80__TI0__RI0
// serial channel 0 buffer register SAB80517 specific(same as stock SCON)
SFR(S0CON,	0x98);
// Bit registers
SBIT(RI0,	0x98,	0);
SBIT(TI0,	0x98,	1);
SBIT(RB80,	0x98,	2);
SBIT(TB80,	0x98,	3);
SBIT(REN0,	0x98,	4);
SBIT(SM20,	0x98,	5);
SBIT(SM1,	0x98,	6);
SBIT(SM0,	0x98,	7);
#endif

#ifdef S0RELL
#undef S0RELL
SFR(S0RELL,	0xAA); // serial channel 0 reload register low byte SAB80517 specific
#endif

#ifdef S0RELH
#undef S0RELH
SFR(S0RELH,	0xBA); // serial channel 0 reload register high byte SAB80517 specific
#endif

#ifdef S1ADR__x__x__x__x__x__x__x__GC
#undef S1ADR__x__x__x__x__x__x__x__GC
SFR(S1ADR,	0xDB); // Serial 1 address, P80C552 specific
// Not directly accessible Bits.
#define GC      0x01
#endif

#ifdef S1BUF
#undef S1BUF
SFR(S1BUF,	0x9C); // serial channel 1 buffer register SAB80517 specific
#endif

#ifdef S1CON_AT_0X9B
#undef S1CON_AT_0X9B
SFR(S1CON,	0x9B); // serial channel 1 control register SAB80517 specific
#endif

#ifdef S1CON__CR2__ENS1__STA__ST0__SI__AA__CR1__CR0
#undef S1CON__CR2__ENS1__STA__ST0__SI__AA__CR1__CR0
SFR(S1CON,	0xD8); // Serial 1 control, P80C552 specific
SFR(SICON,	0xD8); // sometimes called SICON
// Bit registers
SBIT(CR0,	0xD8,	0);
SBIT(CR1,	0xD8,	1);
SBIT(AA,	0xD8,	2);
SBIT(SI,	0xD8,	3);
SBIT(ST0,	0xD8,	4);
SBIT(STA,	0xD8,	5);
SBIT(ENS1,	0xD8,	6);
SBIT(CR2,	0xD8,	7);
#endif

#ifdef S1DAT_AT_0XDA
#undef S1DAT_AT_0XDA
SFR(S1DAT,	0xDA); // Serial 1 data, P80C552 specific
SFR(SIDAT,	0xDA); // sometimes called SIDAT
#endif

#ifdef S1IST_AT_0XDC
#undef S1IST_AT_0XDC
// P89C668 specific
SFR(S1IST,	0xDC);
#endif

#ifdef S1RELL
#undef S1RELL
SFR(S1RELL,	0x9D); // serial channel 1 reload register low byte SAB80517 specific
#endif

#ifdef S1RELH
#undef S1RELH
SFR(S1RELH,	0xBB); // serial channel 1 reload register high byte SAB80517 specific
#endif

#ifdef S1STA__SC4__SC3__SC2__SC1__SC0__x__x__x
#undef S1STA__SC4__SC3__SC2__SC1__SC0__x__x__x
SFR(S1STA,	0xD9); // Serial 1 status, P80C552 specific
// Not directly accessible Bits.
#define SC0         0x08
#define SC1         0x10
#define SC2         0x20
#define SC3         0x40
#define SC4         0x80
#endif

#ifdef SADR_AT_0XA9
#undef SADR_AT_0XA9
SFR(SADDR,	0xA9);
#endif

#ifdef SADDR0
#undef SADDR0
// DS80C320 specific
SFR(SADDR0,	0xA9);
#endif

#ifdef SADDR1
#undef SADDR1
// DS80C320 specific
SFR(SADDR1,	0xAA);
#endif

#ifdef SADEN_AT_0XB9
#undef SADEN_AT_0XB9
SFR(SADEN,	0xB9);
#endif

#ifdef SADEN0
#undef SADEN0
// DS80C320 & DS80C390 specific
SFR(SADEN0,	0xB9);
#endif

#ifdef SADEN1
#undef SADEN1
// DS80C320 & DS80C390 specific
SFR(SADEN1,	0xBA);
#endif

#ifdef SBUF
#undef SBUF
SFR(SBUF,	0x99);
SFR(SBUF0,	0x99);
#endif

#ifdef SBUF1
#undef SBUF1
// DS80C320 & DS80C390 specific
SFR(SBUF1,	0xC1);
#endif

#ifdef SCON
#undef SCON
SFR(SCON,	0x98);
// Bit registers
SBIT(RI,	0x98,	0);
SBIT(TI,	0x98,	1);
SBIT(RB8,	0x98,	2);
SBIT(TB8,	0x98,	3);
SBIT(REN,	0x98,	4);
SBIT(SM2,	0x98,	5);
SBIT(SM1,	0x98,	6);
SBIT(SM0,	0x98,	7);
#endif

#ifdef SCON0
#undef SCON0
SFR(SCON0,	0x98);
// Bit registers
SBIT(RI_0,		0x98,	0);
SBIT(TI_0,		0x98,	1);
SBIT(RB8_0,		0x98,	2);
SBIT(TB8_0,		0x98,	3);
SBIT(REN_0,		0x98,	4);
SBIT(SM2_0,		0x98,	5);
SBIT(SM1_0,		0x98,	6);
SBIT(SM0_0,		0x98,	7);
SBIT(FE_0,		0x98,	7);
SBIT(SM0_FE_0,	0x98,	7);
#endif

#ifdef SCON1
#undef SCON1
// DS80C320 - 80C390 specific
SFR(SCON1,	0xC0);
// Bit registers
SBIT(RI_1,		0xC0,	0);
SBIT(TI_1,		0xC0,	1);
SBIT(RB8_1,		0xC0,	2);
SBIT(TB8_1,		0xC0,	3);
SBIT(REN_1,		0xC0,	4);
SBIT(SM2_1,		0xC0,	5);
SBIT(SM1_1,		0xC0,	6);
SBIT(SM0_1,		0xC0,	7);
SBIT(FE_1,		0xC0,	7);
SBIT(SM0_FE_1,	0xC0,	7);
#endif

#ifdef SETMSK
#undef SETMSK
SFR(SETMSK,	0xA5);
#endif

#ifdef SP
#undef SP
SFR(SP,	0x81);
#endif

#ifdef SPCR
#undef SPCR
SFR(SPCR,	0xD5); // AT89S53 specific
// Not directly accesible bits
#define SPR0        0x01
#define SPR1        0x02
#define CPHA        0x04
#define CPOL        0x08
#define MSTR        0x10
#define DORD        0x20
#define SPE         0x40
#define SPIE        0x80
#endif

#ifdef SPDR
#undef SPDR
SFR(SPDR,	0x86); // AT89S53 specific
// Not directly accesible bits
#define SPD_0       0x01
#define SPD_1       0x02
#define SPD_2       0x04
#define SPD_3       0x08
#define SPD_4       0x10
#define SPD_5       0x20
#define SPD_6       0x40
#define SPD_7       0x80
#endif

#ifdef SPSR
#undef SPSR
SFR(SPSR,	0xAA); // AT89S53 specific
// Not directly accesible bits
#define SPIF        0x40
#define WCOL        0x80
#endif

#ifdef SRELH
#undef SRELH
SFR(SRELH,	0xBA); // Baudrate generator reload high
#endif

#ifdef SRELL
#undef SRELL
SFR(SRELL,	0xAA); // Baudrate generator reload low
#endif

#ifdef STATUS__PIP__HIP__LIP__x__x__x__x__x
#undef STATUS__PIP__HIP__LIP__x__x__x__x__x
// DS80C320 specific
SFR(STATUS,	0xC5);
// Not directly accessible Bits. DS80C320 specific
#define LIP         0x20
#define HIP         0x40
#define PIP         0x80
#endif

#ifdef STATUS__PIP__HIP__LIP__x__SPTA1__SPRA1__SPTA0__SPRA0
#undef STATUS__PIP__HIP__LIP__x__SPTA1__SPRA1__SPTA0__SPRA0
SFR(STATUS,	0xC5); // DS80C390 specific
// Not directly accessible Bits.
#define SPRA0       0x01
#define SPTA0       0x02
#define SPRA1       0x04
#define SPTA1       0x08
#define LIP         0x20
#define HIP         0x40
#define PIP         0x80
#endif

#ifdef STATUS__PIS2__PIS1__PIS0__x__SPTA1__SPRA1__SPTA0__SPRA0
#undef STATUS__PIS2__PIS1__PIS0__x__SPTA1__SPRA1__SPTA0__SPRA0
SFR(STATUS,	0xC5); // DS89C420 specific
// Not directly accessible Bits.
#define SPRA0       0x01
#define SPTA0       0x02
#define SPRA1       0x04
#define SPTA1       0x08
#define PIS0        0x20
#define PIS1        0x40
#define PIS2        0x80
#endif

#ifdef STATUS__PIP__HIP__LIP__x__SPTA1__SPRA1__SPTA0__SPRA0
#undef STATUS__PIP__HIP__LIP__x__SPTA1__SPRA1__SPTA0__SPRA0
SFR(STATUS,	0xC5); // DS80C390 specific
// Not directly accessible Bits.
#define SPRA0       0x01
#define SPTA0       0x02
#define SPRA1       0x04
#define SPTA1       0x08
#define LIP         0x20
#define HIP         0x40
#define PIP         0x80
#endif

#ifdef STATUS__PIP__HIP__LIP__XTUP__SPTA2__SPTA1__SPTA0__SPRA0
#undef STATUS__PIP__HIP__LIP__XTUP__SPTA2__SPTA1__SPTA0__SPRA0
SFR(STATUS,	0xC5); // DS87C520 & DS83520specific
// Not directly accessible Bits.
#define SPRA0       0x01
#define SPTA0       0x02
#define SPTA1       0x04
#define SPTA2       0x08
#define XTUP        0x10
#define LIP         0x20
#define HIP         0x40
#define PIP         0x80
#endif

#ifdef STATUS__ST7__ST6__ST5__ST4__IA0__F0__IBF__OBF
#undef STATUS__ST7__ST6__ST5__ST4__IA0__F0__IBF__OBF
SFR(STATUS,	0xDA); // DS5001specific
// Not directly accessible Bits.
#define OBF         0x01
#define IBF         0x02
#define F0          0x04
#define IA0         0x08
#define ST4         0x10
#define ST5         0x20
#define ST6         0x40
#define ST7         0x80
#endif

#ifdef STE__TG47__TG46__SP45__SP44__SP43__SP42__SP41__SP40
#undef STE__TG47__TG46__SP45__SP44__SP43__SP42__SP41__SP40
SFR(STE,	0xEE); // Set enable, P80C552 specific
// Not directly accessible Bits.
#define SP40        0x01
#define SP41        0x02
#define SP42        0x04
#define SP43        0x08
#define SP44        0x10
#define SP45        0x20
#define TG46        0x40
#define TG47        0x80
#endif

#ifdef SYSCON
#undef SYSCON
SFR(SYSCON,	0xB1); // XRAM Controller Access Control
// SYSCON bits
#define SYSCON_XMAP0 0x01
#define SYSCON_XMAP1 0x02
#define SYSCON_RMAP  0x10
#define SYSCON_EALE  0x20
#endif

#ifdef SYSCON1
#undef SYSCON1
SFR(SYSCON1,	0xB2);
#endif

#ifdef T2
#undef T2
SFR(T2,	0xCC);
#endif

#ifdef T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
#undef T2CON__TF2__EXF2__RCLK__TCLK__EXEN2__TR2__C_T2__CP_RL2
SFR(T2CON,	0xC8);
// Definitions for the 8052 compatible microcontrollers.
// Bit registers
SBIT(CP_RL2,	0xC8,	0);
SBIT(C_T2,		0xC8,	1);
SBIT(TR2,		0xC8,	2);
SBIT(EXEN2,		0xC8,	3);
SBIT(TCLK,		0xC8,	4);
SBIT(RCLK,		0xC8,	5);
SBIT(EXF2,		0xC8,	6);
SBIT(TF2,		0xC8,	7);
// alternate names
SBIT(T2CON_0,	0xC8,	0);
SBIT(T2CON_1,	0xC8,	1);
SBIT(T2CON_2,	0xC8,	2);
SBIT(T2CON_3,	0xC8,	3);
SBIT(T2CON_4,	0xC8,	4);
SBIT(T2CON_5,	0xC8,	5);
SBIT(T2CON_6,	0xC8,	6);
SBIT(T2CON_7,	0xC8,	7);
#endif

#ifdef T2CON__T2PS__I3FR__I2FR__T2R1__T2R0__T2CM__T2I1__T2I0
#undef T2CON__T2PS__I3FR__I2FR__T2R1__T2R0__T2CM__T2I1__T2I0
SFR(T2CON,	0xC8);
// Definitions for the Infineon / Siemens SAB80515, SAB80515A, SAB80517
// Bit registers
SBIT(T2I0,	0xC8,	0);
SBIT(T2I1,	0xC8,	1);
SBIT(T2CM,	0xC8,	2);
SBIT(T2R0,	0xC8,	3);
SBIT(T2R1,	0xC8,	4);
SBIT(I2FR,	0xC8,	5);
SBIT(I3FR,	0xC8,	6);
SBIT(T2PS,	0xC8,	7);
// alternate names
SBIT(T2CON_0,	0xC8,	0);
SBIT(T2CON_1,	0xC8,	1);
SBIT(T2CON_2,	0xC8,	2);
SBIT(T2CON_3,	0xC8,	3);
SBIT(T2CON_4,	0xC8,	4);
SBIT(T2CON_5,	0xC8,	5);
SBIT(T2CON_6,	0xC8,	6);
SBIT(T2CON_7,	0xC8,	7);
#endif

#ifdef T2MOD__x__x__x__D13T1__D13T2__x__T2OE__DCEN
#undef T2MOD__x__x__x__D13T1__D13T2__x__T2OE__DCEN
// Definitions for the timer/counter 2 of the Atmel & Dallas microcontrollers
SFR(T2MOD,	0xC9);
// Not not directly accessible T2MOD bits
#define DCEN        0x01
#define T2OE        0x02
#define D13T2       0x08
#define D13T1       0x10
#endif

#ifdef T2MOD__x__x__x__x__x__x__T2OE__DCEN
#undef T2MOD__x__x__x__x__x__x__T2OE__DCEN
// Definitions for the timer/counter 2 of the Atmel 89x52 microcontroller
SFR(T2MOD,	0xC9);
// Not not directly accessible T2MOD bits
#define DCEN        0x01
#define T2OE        0x02
// Alternate names
#define DCEN_       0x01
#define T2OE_       0x02
#endif

#ifdef T3_AT_0XFF
#undef T3_AT_0XFF
SFR(T3,	0xFF); // Timer 3, P80C552 specific
#endif

#ifdef TA
#undef TA
// DS500x, DS80C320 & DS80C390 specific
SFR(TA,	0xC7);
#endif

#ifdef TCON
#undef TCON
SFR(TCON,	0x88);
//  Bit registers
SBIT(IT0,	0x88,	0);
SBIT(IE0,	0x88,	1);
SBIT(IT1,	0x88,	2);
SBIT(IE1,	0x88,	3);
SBIT(TR0,	0x88,	4);
SBIT(TF0,	0x88,	5);
SBIT(TR1,	0x88,	6);
SBIT(TF1,	0x88,	7);
#endif

#ifdef TH0
#undef TH0
SFR(TH0,	0x8C);
#endif

#ifdef TH1
#undef TH1
SFR(TH1,	0x8D);
#endif

#ifdef TH2
#undef TH2
SFR(TH2,	0xCD);
#endif

#ifdef TL0
#undef TL0
SFR(TL0,	0x8A);
#endif

#ifdef TL1
#undef TL1
SFR(TL1,	0x8B);
#endif

#ifdef TL2
#undef TL2
SFR(TL2,	0xCC);
#endif

#ifdef TMOD
#undef TMOD
SFR(TMOD,	0x89);
// Not directly accessible TMOD bits
#define T0_M0       0x01
#define T0_M1       0x02
#define T0_CT       0x04
#define T0_GATE     0x08
#define T1_M0       0x10
#define T1_M1       0x20
#define T1_CT       0x40
#define T1_GATE     0x80

#define T0_MASK     0x0F
#define T1_MASK     0xF0
#endif

#ifdef TM2CON__T2IS1__T2IS0__T2ER__T2B0__T2P1__T2P0__T2MS1__T2MS0
#undef TM2CON__T2IS1__T2IS0__T2ER__T2B0__T2P1__T2P0__T2MS1__T2MS0
SFR(TM2CON,	0xEA); // Timer 2 control, P80C552 specific
// Not directly accessible Bits.
#define T2MS0       0x01
#define T2MS1       0x02
#define T2P0        0x04
#define T2P1        0x08
#define T2B0        0x10
#define T2ER        0x20
#define T2IS0       0x40
#define T2IS1       0x80
#endif

#ifdef TM2IR__T20V__CMI2__CMI1__CMI0__CTI3__CTI2__CTI1__CTI0
#undef TM2IR__T20V__CMI2__CMI1__CMI0__CTI3__CTI2__CTI1__CTI0
SFR(TM2IR,	0xC8); // Timer 2 int flag reg, P80C552 specific
// Bit register
SBIT(CTI0,	0xC8,	0);
SBIT(CTI1,	0xC8,	1);
SBIT(CTI2,	0xC8,	2);
SBIT(CTI3,	0xC8,	3);
SBIT(CMI0,	0xC8,	4);
SBIT(CMI1,	0xC8,	5);
SBIT(CMI2,	0xC8,	6);
SBIT(T20V,	0xC8,	7);
#endif

#ifdef TMH2_AT_0XED
#undef TMH2_AT_0XED
SFR(TMH2,	0xED); // Timer high 2, P80C552 specific
#endif

#ifdef TML2_AT_0XEC
#undef TML2_AT_0XEC
SFR(TML2,	0xEC); // Timer low 2, P80C552 specific
#endif

#ifdef WCON
#undef WCON
SFR(WCON,	0x96); // AT89S53 specific
// Not directly accesible bits
#define WDTEN       0x01
#define WDTRST      0x02
#define DPS         0x04
#define PS0         0x20
#define PS1         0x40
#define PS2         0x80
#endif

#ifdef WDCON
#undef WDCON
// DS80C320 - 390, DS89C420, etc. specific
SFR(WDCON,	0xD8);
//  Bit registers
SBIT(RWT,		0xD8,	0);
SBIT(EWT,		0xD8,	1);
SBIT(WTRF,		0xD8,	2);
SBIT(WDIF,		0xD8,	3);
SBIT(PFI,		0xD8,	4);
SBIT(EPFI,		0xD8,	5);
SBIT(POR,		0xD8,	6);
SBIT(SMOD_1,	0xD8,	7);
#endif

#ifdef WDTPRG_AT_0XA7
#undef WDTPRG_AT_0XA7
SFR(WDTPRG,	0xA7);
#define WDTRPRG_S0  0x01
#define WDTRPRG_S1  0x02
#define WDTRPRG_S2  0x04
#endif

#ifdef WDTREL
#undef WDTREL
SFR(WDTREL,	0x86); // Watchdof Timer reload register
#endif

#ifdef WDTRST_AT_0XA6
#undef WDTRST_AT_0XA6
SFR(WDTRST,	0xA6);
#endif

#ifdef XPAGE
#undef XPAGE
SFR(XPAGE,	0x91); // Page Address Register for Extended On-Chip Ram - Infineon / Siemens SAB80515A specific
#endif

/////////////////////////
/// Interrupt vectors ///
/////////////////////////

// Interrupt numbers: address = (number * 8) + 3
#define IE0_VECTOR      0       // 0x03 external interrupt 0
#define TF0_VECTOR      1       // 0x0b timer 0
#define IE1_VECTOR      2       // 0x13 external interrupt 1
#define TF1_VECTOR      3       // 0x1b timer 1
#define SI0_VECTOR      4       // 0x23 serial port 0

#ifdef MICROCONTROLLER_AT89S53
#define TF2_VECTOR      5       /* 0x2B timer 2 */
#define EX2_VECTOR      5       /* 0x2B external interrupt 2 */
#endif

#ifdef MICROCONTROLLER_AT89X52
#define TF2_VECTOR      5       /* 0x2B timer 2 */
#define EX2_VECTOR      5       /* 0x2B external interrupt 2 */
#endif

#ifdef MICROCONTROLLER_AT89X55
#define TF2_VECTOR      5       /* 0x2B timer 2 */
#define EX2_VECTOR      5       /* 0x2B external interrupt 2 */
#endif

#ifdef MICROCONTROLLER_DS5000
#define PFW_VECTOR      5       /* 0x2B */
#endif

#ifdef MICROCONTROLLER_DS5001
#define PFW_VECTOR      5       /* 0x2B */
#endif

#ifdef MICROCONTROLLER_DS80C32X
#define TF2_VECTOR      5       /* 0x2B */
#define PFI_VECTOR      6       /* 0x33 */
#define SIO1_VECTOR     7       /* 0x3B */
#define IE2_VECTOR      8       /* 0x43 */
#define IE3_VECTOR      9       /* 0x4B */
#define IE4_VECTOR     10       /* 0x53 */
#define IE5_VECTOR     11       /* 0x5B */
#define WDI_VECTOR     12       /* 0x63 */
#endif

#ifdef MICROCONTROLLER_DS89C420
#define TF2_VECTOR      5       /* 0x2B */
#define PFI_VECTOR      6       /* 0x33 */
#define SIO1_VECTOR     7       /* 0x3B */
#define IE2_VECTOR      8       /* 0x43 */
#define IE3_VECTOR      9       /* 0x4B */
#define IE4_VECTOR     10       /* 0x53 */
#define IE5_VECTOR     11       /* 0x5B */
#define WDI_VECTOR     12       /* 0x63 */
#endif

#ifdef MICROCONTROLLER_DS8XC520
#define TF2_VECTOR      5       /* 0x2B */
#define PFI_VECTOR      6       /* 0x33 */
#define SIO1_VECTOR     7       /* 0x3B */
#define IE2_VECTOR      8       /* 0x43 */
#define IE3_VECTOR      9       /* 0x4B */
#define IE4_VECTOR     10       /* 0x53 */
#define IE5_VECTOR     11       /* 0x5B */
#define WDI_VECTOR     12       /* 0x63 */
#endif

#ifdef MICROCONTROLLER_P80C552
#define SIO1_VECTOR     5       // 0x2B SIO1 (I2C)
#define CT0_VECTOR      6       // 0x33 T2 capture 0
#define CT1_VECTOR      7       // 0x3B T2 capture 1
#define CT2_VECTOR      8       // 0x43 T2 capture 2
#define CT3_VECTOR      9       // 0x4B T2 capture 3
#define ADC_VECTOR     10       // 0x53 ADC completion
#define CM0_VECTOR     11       // 0x5B T2 compare 0
#define CM1_VECTOR     12       // 0x63 T2 compare 1
#define CM2_VECTOR     13       // 0x6B T2 compare 2
#define TF2_VECTOR     14       // 0x73 T2 overflow
#endif

#ifdef MICROCONTROLLER_P89C668
#define SIO1_VECTOR     5       // 0x2b SIO1 (i2c)
#define PCA_VECTOR      6       // 0x33 (Programmable Counter Array)
#define TF2_VECTOR      7       // 0x3B (Timer 2)
#endif

#ifdef MICROCONTROLLER_SAB80509
#define RI0_VECTOR      4       // 0x23 serial port 0
#define TI0_VECTOR      4       // 0x23 serial port 0
#define TF2_VECTOR      5       // 0x2B timer 2
#define EX2_VECTOR      5       // 0x2B external interrupt 2
                                // 0x33
                                // 0x3B
#define IADC_VECTOR     8       // 0x43 A/D converter interrupt
#define IEX2_VECTOR     9       // 0x4B external interrupt 2
#define IEX3_VECTOR    10       // 0x53 external interrupt 3
#define IEX4_VECTOR    11       // 0x5B external interrupt 4
#define IEX5_VECTOR    12       // 0x63 external interrupt 5
#define IEX6_VECTOR    13       // 0x6B external interrupt 6
                                // 0x73 not used
                                // 0x7B not used
#define SI1_VECTOR     16       // 0x83 serial port 1
#define RI1_VECTOR     16       // 0x83 serial port 1
#define TI1_VECTOR     16       // 0x83 serial port 1
                                // 0x8B not used
#define ICM_VECTOR     18       // 0x93 compare registers CM0-CM7
#define CTF_VECTOR     19       // 0x9B compare time overflow
#define ICS_VECTOR     20       // 0xA3 compare register COMSET
#define ICR_VECTOR     21       // 0xAB compare register COMCLR
#define ICC_VECTOR     26       // 0xD3 compare event interrupt ICC10-ICC17
#define CT1_VECTOR     27       // 0xDB compare timer 1 oveflow
#endif

#ifdef MICROCONTROLLER_SAB80515
#define TF2_VECTOR      5       // 0x2B timer 2
#define EX2_VECTOR      5       // 0x2B external interrupt 2
#define IADC_VECTOR     8       // 0x43 A/D converter interrupt
#define IEX2_VECTOR     9       // 0x4B external interrupt 2
#define IEX3_VECTOR    10       // 0x53 external interrupt 3
#define IEX4_VECTOR    11       // 0x5B external interrupt 4
#define IEX5_VECTOR    12       // 0x63 external interrupt 5
#define IEX6_VECTOR    13       // 0x6B external interrupt 6
#endif

#ifdef MICROCONTROLLER_SAB80515A
#define TF2_VECTOR      5       // 0x2B timer 2
#define EX2_VECTOR      5       // 0x2B external interrupt 2
#define IADC_VECTOR     8       // 0x43 A/D converter interrupt
#define IEX2_VECTOR     9       // 0x4B external interrupt 2
#define IEX3_VECTOR    10       // 0x53 external interrupt 3
#define IEX4_VECTOR    11       // 0x5B external interrupt 4
#define IEX5_VECTOR    12       // 0x63 external interrupt 5
#define IEX6_VECTOR    13       // 0x6B external interrupt 6
#endif

#ifdef MICROCONTROLLER_SAB80517
#define TF2_VECTOR      5       // 0x2B timer 2
#define EX2_VECTOR      5       // 0x2B external interrupt 2
#define IADC_VECTOR     8       // 0x43 A/D converter interrupt
#define IEX2_VECTOR     9       // 0x4B external interrupt 2
#define IEX3_VECTOR    10       // 0x53 external interrupt 3
#define IEX4_VECTOR    11       // 0x5B external interrupt 4
#define IEX5_VECTOR    12       // 0x63 external interrupt 5
#define IEX6_VECTOR    13       // 0x6B external interrupt 6
                                // 0x73 not used
                                // 0x7B not used
#define SI1_VECTOR     16       // 0x83 serial port 1
                                // 0x8B not used
                                // 0x93 not used
#define COMPARE_VECTOR 19       // 0x9B compare
#endif

#ifdef MICROCONTORLLER_T89C51RD2
#define TF2_VECTOR      5       /* 0x2B timer 2 */
#define PCA_VECTOR      6       /* 0x33 Programmable Counter Array interrupt */
#endif /* MICROCONTORLLER_T89C51RD2 */

#endif  // End of the header -> #ifndef MCS51REG_H
