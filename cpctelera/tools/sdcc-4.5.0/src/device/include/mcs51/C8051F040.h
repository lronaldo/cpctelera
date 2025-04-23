/*-------------------------------------------------------------------------
   C8051F040.h - Register Declarations for the Cygnal/SiLabs C8051F04x
   Processor Range

   Copyright (C) 2004, Maarten Brock, sourceforge.brock@dse.nl

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

#ifndef C8051F040_H
#define C8051F040_H

#include <compiler.h>

/*  BYTE Registers  */

/*  All Pages */
SFR(P0,			0x80);  /* PORT 0                                        */
SFR(SP,			0x81);  /* STACK POINTER                                 */
SFR(DPL,		0x82);  /* DATA POINTER - LOW BYTE                       */
SFR(DPH,		0x83);  /* DATA POINTER - HIGH BYTE                      */
SFR(SFRPAGE,	0x84);  /* SFR PAGE SELECT                               */
SFR(SFRNEXT,	0x85);  /* SFR STACK NEXT PAGE                           */
SFR(SFRLAST,	0x86);  /* SFR STACK LAST PAGE                           */
SFR(PCON,		0x87);  /* POWER CONTROL                                 */
SFR(P1,			0x90);  /* PORT 1                                        */
SFR(P2,			0xA0);  /* PORT 2                                        */
SFR(IE,			0xA8);  /* INTERRUPT ENABLE                              */
SFR(P3,			0xB0);  /* PORT 3                                        */
SFR(IP,			0xB8);  /* INTERRUPT PRIORITY                            */
SFR(PSW,		0xD0);  /* PROGRAM STATUS WORD                           */
SFR(ACC,		0xE0);  /* ACCUMULATOR                                   */
SFR(EIE1,		0xE6);  /* EXTERNAL INTERRUPT ENABLE 1                   */
SFR(EIE2,		0xE7);  /* EXTERNAL INTERRUPT ENABLE 2                   */
SFR(B,			0xF0);  /* B REGISTER                                    */
SFR(EIP1,		0xF6);  /* EXTERNAL INTERRUPT PRIORITY REGISTER 1        */
SFR(EIP2,		0xF7);  /* EXTERNAL INTERRUPT PRIORITY REGISTER 2        */
SFR(WDTCN,		0xFF);  /* WATCHDOG TIMER CONTROL                        */

/*  Page 0x00 */
SFR(TCON,		0x88);  /* TIMER CONTROL                                 */
SFR(TMOD,		0x89);  /* TIMER MODE                                    */
SFR(TL0,		0x8A);  /* TIMER 0 - LOW BYTE                            */
SFR(TL1,		0x8B);  /* TIMER 1 - LOW BYTE                            */
SFR(TH0,		0x8C);  /* TIMER 0 - HIGH BYTE                           */
SFR(TH1,		0x8D);  /* TIMER 1 - HIGH BYTE                           */
SFR(CKCON,		0x8E);  /* TIMER 0/1 CLOCK CONTROL                       */
SFR(PSCTL,		0x8F);  /* FLASH WRITE/ERASE CONTROL                     */
SFR(SSTA0,		0x91);  /* UART 0 STATUS                                 */
SFR(SCON0,		0x98);  /* UART 0 CONTROL                                */
SFR(SCON,		0x98);  /* UART 0 CONTROL                                */
SFR(SBUF0,		0x99);  /* UART 0 BUFFER                                 */
SFR(SBUF,		0x99);  /* UART 0 BUFFER                                 */
SFR(SPI0CFG,	0x9A);  /* SPI 0 CONFIGURATION                           */
SFR(SPI0DAT,	0x9B);  /* SPI 0 DATA                                    */
SFR(SPI0CKR,	0x9D);  /* SPI 0 CLOCK RATE CONTROL                      */
SFR(EMI0TC,		0xA1);  /* EMIF TIMING CONTROL                           */
SFR(EMI0CN,		0xA2);  /* EMIF CONTROL                                  */
SFR(_XPAGE,		0xA2);  /* XDATA/PDATA PAGE                              */
SFR(EMI0CF,		0xA3);  /* EMIF CONFIGURATION                            */
SFR(SADDR0,		0xA9);  /* UART 0 SLAVE ADDRESS                          */
SFR(FLSCL,		0xB7);  /* FLASH SCALE                                   */
SFR(SADEN0,		0xB9);  /* UART 0 SLAVE ADDRESS MASK                     */
SFR(AMX0CF,		0xBA);  /* ADC 0 MUX CONFIGURATION                       */
SFR(AMX0SL,		0xBB);  /* ADC 0 MUX CHANNEL SELECTION                   */
SFR(ADC0CF,		0xBC);  /* ADC 0 CONFIGURATION                           */
SFR(AMX0PRT,	0xBD);  /* ADC 0 PORT 3 I/O PIN SELECT                   */
SFR(ADC0L,		0xBE);  /* ADC 0 DATA - LOW BYTE                         */
SFR(ADC0H,		0xBF);  /* ADC 0 DATA - HIGH BYTE                        */
SFR(SMB0CN,		0xC0);  /* SMBUS 0 CONTROL                               */
SFR(SMB0STA,	0xC1);  /* SMBUS 0 STATUS                                */
SFR(SMB0DAT,	0xC2);  /* SMBUS 0 DATA                                  */
SFR(SMB0ADR,	0xC3);  /* SMBUS 0 SLAVE ADDRESS                         */
SFR(ADC0GTL,	0xC4);  /* ADC 0 GREATER-THAN REGISTER - LOW BYTE        */
SFR(ADC0GTH,	0xC5);  /* ADC 0 GREATER-THAN REGISTER - HIGH BYTE       */
SFR(ADC0LTL,	0xC6);  /* ADC 0 LESS-THAN REGISTER - LOW BYTE           */
SFR(ADC0LTH,	0xC7);  /* ADC 0 LESS-THAN REGISTER - HIGH BYTE          */
SFR(TMR2CN,		0xC8);  /* TIMER 2 CONTROL                               */
SFR(TMR2CF,		0xC9);  /* TIMER 2 CONFIGURATION                         */
SFR(RCAP2L,		0xCA);  /* TIMER 2 CAPTURE REGISTER - LOW BYTE           */
SFR(RCAP2H,		0xCB);  /* TIMER 2 CAPTURE REGISTER - HIGH BYTE          */
SFR(TMR2L,		0xCC);  /* TIMER 2 - LOW BYTE                            */
SFR(TL2,		0xCC);  /* TIMER 2 - LOW BYTE                            */
SFR(TMR2H,		0xCD);  /* TIMER 2 - HIGH BYTE                           */
SFR(TH2,		0xCD);  /* TIMER 2 - HIGH BYTE                           */
SFR(SMB0CR,		0xCF);  /* SMBUS 0 CLOCK RATE                            */
SFR(REF0CN,		0xD1);  /* VOLTAGE REFERENCE 0 CONTROL                   */
SFR(DAC0L,		0xD2);  /* DAC 0 REGISTER - LOW BYTE                     */
SFR(DAC0H,		0xD3);  /* DAC 0 REGISTER - HIGH BYTE                    */
SFR(DAC0CN,		0xD4);  /* DAC 0 CONTROL                                 */
SFR(HVA0CN,		0xD6);  /* HIGH VOLTAGE DIFFERENTIAL AMP CONTROL         */
SFR(PCA0CN,		0xD8);  /* PCA 0 COUNTER CONTROL                         */
SFR(PCA0MD,		0xD9);  /* PCA 0 COUNTER MODE                            */
SFR(PCA0CPM0,	0xDA);  /* PCA 0 MODULE 0 CONTROL                        */
SFR(PCA0CPM1,	0xDB);  /* PCA 0 MODULE 1 CONTROL                        */
SFR(PCA0CPM2,	0xDC);  /* PCA 0 MODULE 2 CONTROL                        */
SFR(PCA0CPM3,	0xDD);  /* PCA 0 MODULE 3 CONTROL                        */
SFR(PCA0CPM4,	0xDE);  /* PCA 0 MODULE 4 CONTROL                        */
SFR(PCA0CPM5,	0xDF);  /* PCA 0 MODULE 5 CONTROL                        */
SFR(PCA0CPL5,	0xE1);  /* PCA 0 MODULE 5 CAPTURE/COMPARE - LOW BYTE     */
SFR(PCA0CPH5,	0xE2);  /* PCA 0 MODULE 5 CAPTURE/COMPARE - HIGH BYTE    */
SFR(ADC0CN,		0xE8);  /* ADC 0 CONTROL                                 */
SFR(PCA0CPL2,	0xE9);  /* PCA 0 MODULE 2 CAPTURE/COMPARE - LOW BYTE     */
SFR(PCA0CPH2,	0xEA);  /* PCA 0 MODULE 2 CAPTURE/COMPARE - HIGH BYTE    */
SFR(PCA0CPL3,	0xEB);  /* PCA 0 MODULE 3 CAPTURE/COMPARE - LOW BYTE     */
SFR(PCA0CPH3,	0xEC);  /* PCA 0 MODULE 3 CAPTURE/COMPARE - HIGH BYTE    */
SFR(PCA0CPL4,	0xED);  /* PCA 0 MODULE 4 CAPTURE/COMPARE - LOW BYTE     */
SFR(PCA0CPH4,	0xEE);  /* PCA 0 MODULE 4 CAPTURE/COMPARE - HIGH BYTE    */
SFR(RSTSRC,		0xEF);  /* RESET SOURCE                                  */
SFR(SPI0CN,		0xF8);  /* SPI 0 CONTROL                                 */
SFR(PCA0L,		0xF9);  /* PCA 0 TIMER - LOW BYTE                        */
SFR(PCA0H,		0xFA);  /* PCA 0 TIMER - HIGH BYTE                       */
SFR(PCA0CPL0,	0xFB);  /* PCA 0 MODULE 0 CAPTURE/COMPARE - LOW BYTE     */
SFR(PCA0CPH0,	0xFC);  /* PCA 0 MODULE 0 CAPTURE/COMPARE - HIGH BYTE    */
SFR(PCA0CPL1,	0xFD);  /* PCA 0 MODULE 1 CAPTURE/COMPARE - LOW BYTE     */
SFR(PCA0CPH1,	0xFE);  /* PCA 0 MODULE 1 CAPTURE/COMPARE - HIGH BYTE    */

/*  Page 0x01 */
SFR(CPT0CN,		0x88);  /* COMPARATOR 0 CONTROL                          */
SFR(CPT0MD,		0x89);  /* COMPARATOR 0 CONFIGURATION                    */
SFR(SCON1,		0x98);  /* UART 1 CONTROL                                */
SFR(SBUF1,		0x99);  /* UART 1 BUFFER                                 */
SFR(CAN0STA,	0xC0);  /* CAN 0 STATUS                                  */
SFR(TMR3CN,		0xC8);  /* TIMER 3 CONTROL                               */
SFR(TMR3CF,		0xC9);  /* TIMER 3 CONFIGURATION                         */
SFR(RCAP3L,		0xCA);  /* TIMER 3 CAPTURE REGISTER - LOW BYTE           */
SFR(RCAP3H,		0xCB);  /* TIMER 3 CAPTURE REGISTER - HIGH BYTE          */
SFR(TMR3L,		0xCC);  /* TIMER 3 - LOW BYTE                            */
SFR(TMR3H,		0xCD);  /* TIMER 3 - HIGH BYTE                           */
SFR(DAC1L,		0xD2);  /* DAC 1 REGISTER - LOW BYTE                     */
SFR(DAC1H,		0xD3);  /* DAC 1 REGISTER - HIGH BYTE                    */
SFR(DAC1CN,		0xD4);  /* DAC 1 CONTROL                                 */
SFR(CAN0DATL,	0xD8);  /* CAN 0 DATA REGISTER LOW                       */
SFR(CAN0DATH,	0xD9);  /* CAN 0 DATA REGISTER HIGH                      */
SFR(CAN0ADR,	0xDA);  /* CAN 0 ADDRESS                                 */
SFR(CAN0TST,	0xDB);  /* CAN 0 TEST REGISTER                           */
SFR(CAN0CN,		0xF8);  /* CAN 0 CONTROL                                 */

/*  Page 0x02 */
SFR(CPT1CN,		0x88);  /* COMPARATOR 1 CONTROL                          */
SFR(CPT1MD,		0x89);  /* COMPARATOR 1 CONFIGURATION                    */
SFR(AMX2CF,		0xBA);  /* ADC 2 MUX CONFIGURATION                       */
SFR(AMX2SL,		0xBB);  /* ADC 2 MUX CHANNEL SELECTION                   */
SFR(ADC2CF,		0xBC);  /* ADC 2 CONFIGURATION                           */
SFR(ADC2,		0xBE);  /* ADC 2 DATA                                    */
SFR(ADC2GT,		0xC4);  /* ADC 2 GREATER-THAN REGISTER                   */
SFR(ADC2LT,		0xC6);  /* ADC 2 LESS-THAN REGISTER                      */
SFR(TMR4CN,		0xC8);  /* TIMER 4 CONTROL                               */
SFR(TMR4CF,		0xC9);  /* TIMER 4 CONFIGURATION                         */
SFR(RCAP4L,		0xCA);  /* TIMER 4 CAPTURE REGISTER - LOW BYTE           */
SFR(RCAP4H,		0xCB);  /* TIMER 4 CAPTURE REGISTER - HIGH BYTE          */
SFR(TMR4L,		0xCC);  /* TIMER 4 - LOW BYTE                            */
SFR(TMR4H,		0xCD);  /* TIMER 4 - HIGH BYTE                           */
SFR(ADC2CN,		0xE8);  /* ADC 2 CONTROL                                 */

/*  Page 0x03 */
SFR(CPT2CN,		0x88);  /* COMPARATOR 2 CONTROL                          */
SFR(CPT2MD,		0x89);  /* COMPARATOR 2 CONFIGURATION                    */

/*  Page 0x0F */
SFR(OSCICN,		0x8A);  /* INTERNAL OSCILLATOR CONTROL                   */
SFR(OSCICL,		0x8B);  /* INTERNAL OSCILLATOR CALIBRATION               */
SFR(OSCXCN,		0x8C);  /* EXTERNAL OSCILLATOR CONTROL                   */
SFR(SFRPGCN,	0x96);  /* SFR PAGE CONTROL                              */
SFR(CLKSEL,		0x97);  /* SYSTEM CLOCK SELECT                           */
SFR(P4MDOUT,	0x9C);  /* PORT 4 OUTPUT MODE                            */
SFR(P5MDOUT,	0x9D);  /* PORT 5 OUTPUT MODE                            */
SFR(P6MDOUT,	0x9E);  /* PORT 6 OUTPUT MODE                            */
SFR(P7MDOUT,	0x9F);  /* PORT 7 OUTPUT MODE                            */
SFR(P0MDOUT,	0xA4);  /* PORT 0 OUTPUT MODE                            */
SFR(P1MDOUT,	0xA5);  /* PORT 1 OUTPUT MODE                            */
SFR(P2MDOUT,	0xA6);  /* PORT 2 OUTPUT MODE CONFIGURATION              */
SFR(P3MDOUT,	0xA7);  /* PORT 3 OUTPUT MODE CONFIGURATION              */
SFR(P1MDIN,		0xAD);  /* PORT 1 INPUT MODE                             */
SFR(P2MDIN,		0xAE);  /* PORT 2 INPUT MODE                             */
SFR(P3MDIN,		0xAF);  /* PORT 3 INPUT MODE                             */
SFR(FLACL,		0xB7);  /* FLASH ACCESS LIMIT                            */
SFR(P4,			0xC8);  /* PORT 4                                        */
SFR(P5,			0xD8);  /* PORT 5                                        */
SFR(XBR0,		0xE1);  /* CROSSBAR CONFIGURATION REGISTER 0             */
SFR(XBR1,		0xE2);  /* CROSSBAR CONFIGURATION REGISTER 1             */
SFR(XBR2,		0xE3);  /* CROSSBAR CONFIGURATION REGISTER 2             */
SFR(XBR3,		0xE4);  /* CROSSBAR CONFIGURATION REGISTER 3             */
SFR(P6,			0xE8);  /* PORT 6                                        */
SFR(P7,			0xF8);  /* PORT 7                                        */

/*
Do NOT use sfr16 for CAN0DAT !
*/

/*  BIT Registers  */

/*  P0  0x80 */
SBIT(P0_0,		0x80,	0);
SBIT(P0_1,		0x80,	1);
SBIT(P0_2,		0x80,	2);
SBIT(P0_3,		0x80,	3);
SBIT(P0_4,		0x80,	4);
SBIT(P0_5,		0x80,	5);
SBIT(P0_6,		0x80,	6);
SBIT(P0_7,		0x80,	7);

/*  TCON  0x88 */
SBIT(IT0,		0x88,	0);  /* EXT. INTERRUPT 0 TYPE                         */
SBIT(IE0,		0x88,	1);  /* EXT. INTERRUPT 0 EDGE FLAG                    */
SBIT(IT1,		0x88,	2);  /* EXT. INTERRUPT 1 TYPE                         */
SBIT(IE1,		0x88,	3);  /* EXT. INTERRUPT 1 EDGE FLAG                    */
SBIT(TR0,		0x88,	4);  /* TIMER 0 ON/OFF CONTROL                        */
SBIT(TF0,		0x88,	5);  /* TIMER 0 OVERFLOW FLAG                         */
SBIT(TR1,		0x88,	6);  /* TIMER 1 ON/OFF CONTROL                        */
SBIT(TF1,		0x88,	7);  /* TIMER 1 OVERFLOW FLAG                         */

/*  CPT0CN  0x88 */
SBIT(CP0HYN0,	0x88,	0);  /* COMPARATOR 0 NEGATIVE HYSTERESIS 0            */
SBIT(CP0HYN1,	0x88,	1);  /* COMPARATOR 0 NEGATIVE HYSTERESIS 1            */
SBIT(CP0HYP0,	0x88,	2);  /* COMPARATOR 0 POSITIVE HYSTERESIS 0            */
SBIT(CP0HYP1,	0x88,	3);  /* COMPARATOR 0 POSITIVE HYSTERESIS 1            */
SBIT(CP0FIF,	0x88,	4);  /* COMPARATOR 0 FALLING EDGE INTERRUPT           */
SBIT(CP0RIF,	0x88,	5);  /* COMPARATOR 0 RISING EDGE INTERRUPT            */
SBIT(CP0OUT,	0x88,	6);  /* COMPARATOR 0 OUTPUT                           */
SBIT(CP0EN,		0x88,	7);  /* COMPARATOR 0 ENABLE                           */

/*  CPT1CN  0x88 */
SBIT(CP1HYN0,	0x88,	0);  /* COMPARATOR 1 NEGATIVE HYSTERESIS 0            */
SBIT(CP1HYN1,	0x88,	1);  /* COMPARATOR 1 NEGATIVE HYSTERESIS 1            */
SBIT(CP1HYP0,	0x88,	2);  /* COMPARATOR 1 POSITIVE HYSTERESIS 0            */
SBIT(CP1HYP1,	0x88,	3);  /* COMPARATOR 1 POSITIVE HYSTERESIS 1            */
SBIT(CP1FIF,	0x88,	4);  /* COMPARATOR 1 FALLING EDGE INTERRUPT           */
SBIT(CP1RIF,	0x88,	5);  /* COMPARATOR 1 RISING EDGE INTERRUPT            */
SBIT(CP1OUT,	0x88,	6);  /* COMPARATOR 1 OUTPUT                           */
SBIT(CP1EN,		0x88,	7);  /* COMPARATOR 1 ENABLE                           */

/*  CPT2CN  0x88 */
SBIT(CP2HYN0,	0x88,	0);  /* COMPARATOR 2 NEGATIVE HYSTERESIS 0            */
SBIT(CP2HYN1,	0x88,	1);  /* COMPARATOR 2 NEGATIVE HYSTERESIS 1            */
SBIT(CP2HYP0,	0x88,	2);  /* COMPARATOR 2 POSITIVE HYSTERESIS 0            */
SBIT(CP2HYP1,	0x88,	3);  /* COMPARATOR 2 POSITIVE HYSTERESIS 1            */
SBIT(CP2FIF,	0x88,	4);  /* COMPARATOR 2 FALLING EDGE INTERRUPT           */
SBIT(CP2RIF,	0x88,	5);  /* COMPARATOR 2 RISING EDGE INTERRUPT            */
SBIT(CP2OUT,	0x88,	6);  /* COMPARATOR 2 OUTPUT                           */
SBIT(CP2EN,		0x88,	7);  /* COMPARATOR 2 ENABLE                           */

/*  P1  0x90 */
SBIT(P1_0,		0x90,	0);
SBIT(P1_1,		0x90,	1);
SBIT(P1_2,		0x90,	2);
SBIT(P1_3,		0x90,	3);
SBIT(P1_4,		0x90,	4);
SBIT(P1_5,		0x90,	5);
SBIT(P1_6,		0x90,	6);
SBIT(P1_7,		0x90,	7);

/*  SCON0  0x98 */
SBIT(RI0,		0x98,	0);  /* UART 0 RX INTERRUPT FLAG                      */
SBIT(RI,		0x98,	0);  /* UART 0 RX INTERRUPT FLAG                      */
SBIT(TI0,		0x98,	1);  /* UART 0 TX INTERRUPT FLAG                      */
SBIT(TI,		0x98,	1);  /* UART 0 TX INTERRUPT FLAG                      */
SBIT(RB80,		0x98,	2);  /* UART 0 RX BIT 8                               */
SBIT(TB80,		0x98,	3);  /* UART 0 TX BIT 8                               */
SBIT(REN0,		0x98,	4);  /* UART 0 RX ENABLE                              */
SBIT(REN,		0x98,	4);  /* UART 0 RX ENABLE                              */
SBIT(SM20,		0x98,	5);  /* UART 0 MULTIPROCESSOR EN                      */
SBIT(SM10,		0x98,	6);  /* UART 0 MODE 1                                 */
SBIT(SM00,		0x98,	7);  /* UART 0 MODE 0                                 */

/*  SCON1  0x98 */
SBIT(RI1,		0x98,	0);  /* UART 1 RX INTERRUPT FLAG                      */
SBIT(TI1,		0x98,	1);  /* UART 1 TX INTERRUPT FLAG                      */
SBIT(RB81,		0x98,	2);  /* UART 1 RX BIT 8                               */
SBIT(TB81,		0x98,	3);  /* UART 1 TX BIT 8                               */
SBIT(REN1,		0x98,	4);  /* UART 1 RX ENABLE                              */
SBIT(MCE1,		0x98,	5);  /* UART 1 MCE                                    */
SBIT(S1MODE,	0x98,	7);  /* UART 1 MODE                                   */

/*  P2  0xA0 */
SBIT(P2_0,		0xA0,	0);
SBIT(P2_1,		0xA0,	1);
SBIT(P2_2,		0xA0,	2);
SBIT(P2_3,		0xA0,	3);
SBIT(P2_4,		0xA0,	4);
SBIT(P2_5,		0xA0,	5);
SBIT(P2_6,		0xA0,	6);
SBIT(P2_7,		0xA0,	7);

/*  IE  0xA8 */
SBIT(EX0,		0xA8,	0);  /* EXTERNAL INTERRUPT 0 ENABLE                   */
SBIT(ET0,		0xA8,	1);  /* TIMER 0 INTERRUPT ENABLE                      */
SBIT(EX1,		0xA8,	2);  /* EXTERNAL INTERRUPT 1 ENABLE                   */
SBIT(ET1,		0xA8,	3);  /* TIMER 1 INTERRUPT ENABLE                      */
SBIT(ES0,		0xA8,	4);  /* UART0 INTERRUPT ENABLE                        */
SBIT(ES,		0xA8,	4);  /* UART0 INTERRUPT ENABLE                        */
SBIT(ET2,		0xA8,	5);  /* TIMER 2 INTERRUPT ENABLE                      */
SBIT(EA,		0xA8,	7);  /* GLOBAL INTERRUPT ENABLE                       */

/*  P3  0xB0 */
SBIT(P3_0,		0xB0,	0);
SBIT(P3_1,		0xB0,	1);
SBIT(P3_2,		0xB0,	2);
SBIT(P3_3,		0xB0,	3);
SBIT(P3_4,		0xB0,	4);
SBIT(P3_5,		0xB0,	5);
SBIT(P3_6,		0xB0,	6);
SBIT(P3_7,		0xB0,	7);

/*  IP  0xB8 */
SBIT(PX0,		0xB8,	0);  /* EXTERNAL INTERRUPT 0 PRIORITY                 */
SBIT(PT0,		0xB8,	1);  /* TIMER 0 PRIORITY                              */
SBIT(PX1,		0xB8,	2);  /* EXTERNAL INTERRUPT 1 PRIORITY                 */
SBIT(PT1,		0xB8,	3);  /* TIMER 1 PRIORITY                              */
SBIT(PS0,		0xB8,	4);  /* SERIAL PORT PRIORITY                          */
SBIT(PS,		0xB8,	4);  /* SERIAL PORT PRIORITY                          */
SBIT(PT2,		0xB8,	5);  /* TIMER 2 PRIORITY                              */

/*  SMB0CN  0xC0 */
SBIT(SMBTOE,	0xC0,	0);  /* SMBUS 0 TIMEOUT ENABLE                        */
SBIT(SMBFTE,	0xC0,	1);  /* SMBUS 0 FREE TIMER ENABLE                     */
SBIT(AA,		0xC0,	2);  /* SMBUS 0 ASSERT/ACKNOWLEDGE FLAG               */
SBIT(SI,		0xC0,	3);  /* SMBUS 0 INTERRUPT PENDING FLAG                */
SBIT(STO,		0xC0,	4);  /* SMBUS 0 STOP FLAG                             */
SBIT(STA,		0xC0,	5);  /* SMBUS 0 START FLAG                            */
SBIT(ENSMB,		0xC0,	6);  /* SMBUS 0 ENABLE                                */
SBIT(BUSY,		0xC0,	7);  /* SMBUS 0 BUSY                                  */

/*  CAN0STA  0xC0 */
SBIT(CANTXOK,	0xC0,	3);  /* CAN TRANSMITTED A MESSAGE SUCCESSFULLY        */
SBIT(CANRXOK,	0xC0,	4);  /* CAN RECEIVED A MESSAGE SUCCESSFULLY           */
SBIT(CANEPASS,	0xC0,	5);  /* CAN ERROR PASSIVE                             */
SBIT(CANEWARN,	0xC0,	6);  /* CAN WARNING STATUS                            */
SBIT(CANBOFF,	0xC0,	7);  /* CAN BUSOFF STATUS                             */

/*  TMR2CN  0xC8 */
SBIT(CPRL2,		0xC8,	0);  /* TIMER 2 CAPTURE SELECT                        */
SBIT(CT2,		0xC8,	1);  /* TIMER 2 COUNTER SELECT                        */
SBIT(TR2,		0xC8,	2);  /* TIMER 2 ON/OFF CONTROL                        */
SBIT(EXEN2,		0xC8,	3);  /* TIMER 2 EXTERNAL ENABLE FLAG                  */
SBIT(EXF2,		0xC8,	6);  /* TIMER 2 EXTERNAL FLAG                         */
SBIT(TF2,		0xC8,	7);  /* TIMER 2 OVERFLOW FLAG                         */

/*  TMR3CN  0xC8 */
SBIT(CPRL3,		0xC8,	0);  /* TIMER 3 CAPTURE SELECT                        */
SBIT(CT3,		0xC8,	1);  /* TIMER 3 COUNTER SELECT                        */
SBIT(TR3,		0xC8,	2);  /* TIMER 3 ON/OFF CONTROL                        */
SBIT(EXEN3,		0xC8,	3);  /* TIMER 3 EXTERNAL ENABLE FLAG                  */
SBIT(EXF3,		0xC8,	6);  /* TIMER 3 EXTERNAL FLAG                         */
SBIT(TF3,		0xC8,	7);  /* TIMER 3 OVERFLOW FLAG                         */

/*  TMR4CN  0xC8 */
SBIT(CPRL4,		0xC8,	0);  /* TIMER 4 CAPTURE SELECT                        */
SBIT(CT4,		0xC8,	1);  /* TIMER 4 COUNTER SELECT                        */
SBIT(TR4,		0xC8,	2);  /* TIMER 4 ON/OFF CONTROL                        */
SBIT(EXEN4,		0xC8,	3);  /* TIMER 4 EXTERNAL ENABLE FLAG                  */
SBIT(EXF4,		0xC8,	6);  /* TIMER 4 EXTERNAL FLAG                         */
SBIT(TF4,		0xC8,	7);  /* TIMER 4 OVERFLOW FLAG                         */

/*  P4  0xC8 */
SBIT(P4_0,		0xC8,	0);
SBIT(P4_1,		0xC8,	1);
SBIT(P4_2,		0xC8,	2);
SBIT(P4_3,		0xC8,	3);
SBIT(P4_4,		0xC8,	4);
SBIT(P4_5,		0xC8,	5);
SBIT(P4_6,		0xC8,	6);
SBIT(P4_7,		0xC8,	7);

/*  PSW  0xD0 */
SBIT(P,			0xD0,	0);  /* ACCUMULATOR PARITY FLAG                       */
SBIT(F1,		0xD0,	1);  /* USER FLAG 1                                   */
SBIT(OV,		0xD0,	2);  /* OVERFLOW FLAG                                 */
SBIT(RS0,		0xD0,	3);  /* REGISTER BANK SELECT 0                        */
SBIT(RS1,		0xD0,	4);  /* REGISTER BANK SELECT 1                        */
SBIT(F0,		0xD0,	5);  /* USER FLAG 0                                   */
SBIT(AC,		0xD0,	6);  /* AUXILIARY CARRY FLAG                          */
SBIT(CY,		0xD0,	7);  /* CARRY FLAG                                    */

/*  PCA0CN  0xD8 */
SBIT(CCF0,		0xD8,	0);  /* PCA 0 MODULE 0 INTERRUPT FLAG                 */
SBIT(CCF1,		0xD8,	1);  /* PCA 0 MODULE 1 INTERRUPT FLAG                 */
SBIT(CCF2,		0xD8,	2);  /* PCA 0 MODULE 2 INTERRUPT FLAG                 */
SBIT(CCF3,		0xD8,	3);  /* PCA 0 MODULE 3 INTERRUPT FLAG                 */
SBIT(CCF4,		0xD8,	4);  /* PCA 0 MODULE 4 INTERRUPT FLAG                 */
SBIT(CCF5,		0xD8,	5);  /* PCA 0 MODULE 5 INTERRUPT FLAG                 */
SBIT(CR,		0xD8,	6);  /* PCA 0 COUNTER RUN CONTROL BIT                 */
SBIT(CF,		0xD8,	7);  /* PCA 0 COUNTER OVERFLOW FLAG                   */

/*  P5  0xD8 */
SBIT(P5_0,		0xD8,	0);
SBIT(P5_1,		0xD8,	1);
SBIT(P5_2,		0xD8,	2);
SBIT(P5_3,		0xD8,	3);
SBIT(P5_4,		0xD8,	4);
SBIT(P5_5,		0xD8,	5);
SBIT(P5_6,		0xD8,	6);
SBIT(P5_7,		0xD8,	7);

/*  ADC0CN  0xE8 */
SBIT(AD0LJST,	0xE8,	0);  /* ADC 0 RIGHT JUSTIFY DATA BIT                  */
SBIT(AD0WINT,	0xE8,	1);  /* ADC 0 WINDOW INTERRUPT FLAG                   */
SBIT(AD0CM0,	0xE8,	2);  /* ADC 0 CONVERT START MODE BIT 0                */
SBIT(AD0CM1,	0xE8,	3);  /* ADC 0 CONVERT START MODE BIT 1                */
SBIT(AD0BUSY,	0xE8,	4);  /* ADC 0 BUSY FLAG                               */
SBIT(AD0INT,	0xE8,	5);  /* ADC 0 EOC INTERRUPT FLAG                      */
SBIT(AD0TM,		0xE8,	6);  /* ADC 0 TRACK MODE                              */
SBIT(AD0EN,		0xE8,	7);  /* ADC 0 ENABLE                                  */

/*  ADC2CN  0xE8 */
SBIT(AD2WINT,	0xE8,	0);  /* ADC 2 WINDOW INTERRUPT FLAG                   */
SBIT(AD2CM0,	0xE8,	1);  /* ADC 2 CONVERT START MODE BIT 0                */
SBIT(AD2CM1,	0xE8,	2);  /* ADC 2 CONVERT START MODE BIT 1                */
SBIT(AD2CM2,	0xE8,	3);  /* ADC 2 CONVERT START MODE BIT 2                */
SBIT(AD2BUSY,	0xE8,	4);  /* ADC 2 BUSY FLAG                               */
SBIT(AD2INT,	0xE8,	5);  /* ADC 2 EOC INTERRUPT FLAG                      */
SBIT(AD2TM,		0xE8,	6);  /* ADC 2 TRACK MODE                              */
SBIT(AD2EN,		0xE8,	7);  /* ADC 2 ENABLE                                  */

/*  P6  0xE8 */
SBIT(P6_0,		0xE8,	0);
SBIT(P6_1,		0xE8,	1);
SBIT(P6_2,		0xE8,	2);
SBIT(P6_3,		0xE8,	3);
SBIT(P6_4,		0xE8,	4);
SBIT(P6_5,		0xE8,	5);
SBIT(P6_6,		0xE8,	6);
SBIT(P6_7,		0xE8,	7);

/*  SPI0CN  0xF8 */
SBIT(SPIEN,		0xF8,	0);  /* SPI 0 SPI ENABLE                              */
SBIT(TXBMT,		0xF8,	1);  /* SPI 0 TX BUFFER EMPTY FLAG                    */
SBIT(NSSMD0,	0xF8,	2);  /* SPI 0 SLAVE SELECT MODE 0                     */
SBIT(NSSMD1,	0xF8,	3);  /* SPI 0 SLAVE SELECT MODE 1                     */
SBIT(RXOVRN,	0xF8,	4);  /* SPI 0 RX OVERRUN FLAG                         */
SBIT(MODF,		0xF8,	5);  /* SPI 0 MODE FAULT FLAG                         */
SBIT(WCOL,		0xF8,	6);  /* SPI 0 WRITE COLLISION FLAG                    */
SBIT(SPIF,		0xF8,	7);  /* SPI 0 INTERRUPT FLAG                          */

/*  CAN0CN  0xF8 */
SBIT(CANINIT,	0xF8,	0);  /* CAN INITIALIZATION                            */
SBIT(CANIE,		0xF8,	1);  /* CAN MODULE INTERRUPT ENABLE                   */
SBIT(CANSIE,	0xF8,	2);  /* CAN STATUS CHANGE INTERRUPT ENABLE            */
SBIT(CANEIE,	0xF8,	3);  /* CAN ERROR INTERRUPT ENABLE                    */
SBIT(CANIF,		0xF8,	4);  /* CAN INTERRUPT FLAG                            */
SBIT(CANDAR,	0xF8,	5);  /* CAN DISABLE AUTOMATIC RETRANSMISSION          */
SBIT(CANCCE,	0xF8,	6);  /* CAN CONFIGURATION CHANGE ENABLE               */
SBIT(CANTEST,	0xF8,	7);  /* CAN TEST MODE ENABLE                          */

/*  P7  0xF8 */
SBIT(P7_0,		0xF8,	0);
SBIT(P7_1,		0xF8,	1);
SBIT(P7_2,		0xF8,	2);
SBIT(P7_3,		0xF8,	3);
SBIT(P7_4,		0xF8,	4);
SBIT(P7_5,		0xF8,	5);
SBIT(P7_6,		0xF8,	6);
SBIT(P7_7,		0xF8,	7);


/* Predefined SFR Bit Masks */

#define IDLE              0x01    /* PCON                                */
#define STOP              0x02    /* PCON                                */
#define ECCF              0x01    /* PCA0CPMn                            */
#define PWM               0x02    /* PCA0CPMn                            */
#define TOG               0x04    /* PCA0CPMn                            */
#define MAT               0x08    /* PCA0CPMn                            */
#define CAPN              0x10    /* PCA0CPMn                            */
#define CAPP              0x20    /* PCA0CPMn                            */
#define ECOM              0x40    /* PCA0CPMn                            */
#define PWM16             0x80    /* PCA0CPMn                            */
#define PORSF             0x02    /* RSTSRC                              */
#define SWRSF             0x10    /* RSTSRC                              */


/* SFR PAGE DEFINITIONS */

#define CONFIG_PAGE       0x0F     /* SYSTEM AND PORT CONFIGURATION PAGE */
#define LEGACY_PAGE       0x00     /* LEGACY SFR PAGE                    */
#define TIMER01_PAGE      0x00     /* TIMER 0 AND TIMER 1                */
#define CPT0_PAGE         0x01     /* COMPARATOR 0                       */
#define CPT1_PAGE         0x02     /* COMPARATOR 1                       */
#define CPT2_PAGE         0x03     /* COMPARATOR 2                       */
#define UART0_PAGE        0x00     /* UART 0                             */
#define UART1_PAGE        0x01     /* UART 1                             */
#define SPI0_PAGE         0x00     /* SPI 0                              */
#define EMI0_PAGE         0x00     /* EXTERNAL MEMORY INTERFACE          */
#define ADC0_PAGE         0x00     /* ADC 0                              */
#define ADC2_PAGE         0x02     /* ADC 2                              */
#define SMB0_PAGE         0x00     /* SMBUS 0                            */
#define TMR2_PAGE         0x00     /* TIMER 2                            */
#define TMR3_PAGE         0x01     /* TIMER 3                            */
#define TMR4_PAGE         0x02     /* TIMER 4                            */
#define DAC0_PAGE         0x00     /* DAC 0                              */
#define DAC1_PAGE         0x01     /* DAC 1                              */
#define PCA0_PAGE         0x00     /* PCA 0                              */
#define CAN0_PAGE         0x01     /* CAN 0                              */

#endif
