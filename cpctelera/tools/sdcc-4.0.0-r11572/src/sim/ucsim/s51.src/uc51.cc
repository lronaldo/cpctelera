/*
 * Simulator of microcontrollers (uc51.cc)
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
//#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
//#include <fcntl.h>
//#include <errno.h>
//#include <sys/types.h>
//#include <sys/time.h>
//#if FD_HEADER_OK
//# include HEADER_FD
//#endif
#include <string.h>
//#include "i_string.h"

// prj
#include "utils.h"
#include "globals.h"

// sim
//#include "optioncl.h"
#include "iwrap.h"

//cmd.src
#include "cmd_uccl.h"

// local
#include "uc51cl.h"
#include "glob.h"
#include "regs51.h"
//#include "timer0cl.h"
#include "timer1cl.h"
#include "serialcl.h"
#include "portcl.h"
//#include "interruptcl.h"
//#include "types51.h"



/*
 * Names of SFR cells
 */

static struct name_entry sfr_tab51[]=
{
  {CPU_F380,			  0xc7, "P4"},
  
  {CPU_251,                       0x84, "DPXL"},
  {CPU_251|CPU_DS390|CPU_DS390F,  0x93, "DPX"},
  {CPU_251,                       0xa8, "IE0"},
  {CPU_251,                       0xb7, "IPH0"},
  {CPU_251,                       0xb8, "IPL0"},
  {CPU_251,                       0xbd, "SPH"},
  {CPU_251,                       0xd1, "PSW1"},

  {CPU_DS390|CPU_DS390F,          0x80, "P4"},
  {CPU_ALL_DS3X0,                 0x84, "DPL1"},
  {CPU_ALL_DS3X0,                 0x85, "DPH1"},
  {CPU_ALL_DS3X0,                 0x86, "DPS"},
  {CPU_ALL_DS3X0,                 0x8e, "CKCON"},
  {CPU_DS390|CPU_DS390F,          0x91, "EXIF"},
  {CPU_DS390|CPU_DS390F,          0x92, "P4CNT"},
  {CPU_DS390|CPU_DS390F,          0x95, "DPX1"},
  {CPU_DS390|CPU_DS390F,          0x96, "C0RMS0"},
  {CPU_DS390|CPU_DS390F,          0x97, "C0RMS1"},
  {CPU_DS390|CPU_DS390F,          0x98, "SCON0"},
  {CPU_DS390|CPU_DS390F,          0x99, "SBUF0"},
  {CPU_DS390|CPU_DS390F,          0x9b, "ESP"},
  {CPU_DS390|CPU_DS390F,          0x9c, "AP"},
  {CPU_DS390|CPU_DS390F,          0x9d, "ACON"},
  {CPU_DS390|CPU_DS390F,          0x9e, "C0TMA0"},
  {CPU_DS390|CPU_DS390F,          0x9f, "C0TMA1"},
  {CPU_DS390|CPU_DS390F,          0xa1, "P5"},
  {CPU_DS390|CPU_DS390F,          0xa2, "P5CNT"},
  {CPU_DS390|CPU_DS390F,          0xa3, "C0C"},
  {CPU_DS390|CPU_DS390F,          0xa4, "C0S"},
  {CPU_DS390|CPU_DS390F,          0xa5, "C0IR"},
  {CPU_DS390|CPU_DS390F,          0xa6, "C0TE"},
  {CPU_DS390|CPU_DS390F,          0xa7, "C0RE"},
  {CPU_DS390|CPU_DS390F,          0xa9, "SADDR0"},
  {CPU_DS390|CPU_DS390F,          0xaa, "SADDR1"},
  {CPU_DS390|CPU_DS390F,          0xab, "C0M1C"},
  {CPU_DS390|CPU_DS390F,          0xac, "C0M2C"},
  {CPU_DS390|CPU_DS390F,          0xad, "C0M3C"},
  {CPU_DS390|CPU_DS390F,          0xae, "C0M4C"},
  {CPU_DS390|CPU_DS390F,          0xaf, "C0M5C"},
  {CPU_DS390|CPU_DS390F,          0xb3, "C0M6C"},
  {CPU_DS390|CPU_DS390F,          0xb4, "C0M7C"},
  {CPU_DS390|CPU_DS390F,          0xb5, "C0M8C"},
  {CPU_DS390|CPU_DS390F,          0xb6, "C0M9C"},
  {CPU_DS390|CPU_DS390F,          0xb7, "C0M10C"},
  {CPU_DS390|CPU_DS390F,          0xb9, "SADEN0"},
  {CPU_DS390|CPU_DS390F,          0xba, "SADEN1"},
  {CPU_DS390|CPU_DS390F,          0xbb, "C0M11C"},
  {CPU_DS390|CPU_DS390F,          0xbc, "C0M12C"},
  {CPU_DS390|CPU_DS390F,          0xbd, "C0M13C"},
  {CPU_DS390|CPU_DS390F,          0xbe, "C0M14C"},
  {CPU_DS390|CPU_DS390F,          0xbf, "C0M15C"},
  {CPU_DS390|CPU_DS390F,          0xc0, "SCON1"},
  {CPU_DS390|CPU_DS390F,          0xc1, "SBUF1"},
  {CPU_DS390|CPU_DS390F,          0xc4, "PMR"},
  {CPU_DS390|CPU_DS390F,          0xc5, "STATUS"},
  {CPU_DS390|CPU_DS390F,          0xc6, "MCON"},
  {CPU_DS390|CPU_DS390F,          0xc7, "TA"},
  {CPU_DS390|CPU_DS390F,          0xce, "COR"},
  {CPU_DS390|CPU_DS390F,          0xd1, "MCNT0"},
  {CPU_DS390|CPU_DS390F,          0xd2, "MCNT1"},
  {CPU_DS390|CPU_DS390F,          0xd3, "MA"},
  {CPU_DS390|CPU_DS390F,          0xd4, "MB"},
  {CPU_DS390|CPU_DS390F,          0xd5, "MC"},
  {CPU_DS390|CPU_DS390F,          0xd6, "C1RMS0"},
  {CPU_DS390|CPU_DS390F,          0xd7, "C1RMS1"},
  {CPU_DS390|CPU_DS390F,          0xd8, "WDCON"},
  {CPU_DS390|CPU_DS390F,          0xde, "C1TMA0"},
  {CPU_DS390|CPU_DS390F,          0xdf, "C1TMA1"},
  {CPU_DS390|CPU_DS390F,          0xe3, "C1C"},
  {CPU_DS390|CPU_DS390F,          0xe4, "C1S"},
  {CPU_DS390|CPU_DS390F,          0xe5, "C11R"},
  {CPU_DS390|CPU_DS390F,          0xe6, "C1TE"},
  {CPU_DS390|CPU_DS390F,          0xe7, "C1RE"},
  {CPU_DS390|CPU_DS390F,          0xe8, "EIE"},
  {CPU_DS390|CPU_DS390F,          0xea, "MXAX"},
  {CPU_DS390|CPU_DS390F,          0xeb, "C1M1C"},
  {CPU_DS390|CPU_DS390F,          0xec, "C1M2C"},
  {CPU_DS390|CPU_DS390F,          0xed, "C1M3C"},
  {CPU_DS390|CPU_DS390F,          0xee, "C1M4C"},
  {CPU_DS390|CPU_DS390F,          0xef, "C1M5C"},
  {CPU_DS390|CPU_DS390F,          0xf3, "C1M6C"},
  {CPU_DS390|CPU_DS390F,          0xf4, "C1M7C"},
  {CPU_DS390|CPU_DS390F,          0xf5, "C1M8C"},
  {CPU_DS390|CPU_DS390F,          0xf6, "C1M9C"},
  {CPU_DS390|CPU_DS390F,          0xf7, "C1M10C"},
  {CPU_DS390|CPU_DS390F,          0xfb, "C1M11C"},
  {CPU_DS390|CPU_DS390F,          0xfc, "C1M12C"},
  {CPU_DS390|CPU_DS390F,          0xfd, "C1M13C"},
  {CPU_DS390|CPU_DS390F,          0xfe, "C1M14C"},
  {CPU_DS390|CPU_DS390F,          0xff, "C1M15C"},

  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x80, "P0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x81, "SP"},
  //{CPU_ALL_51|CPU_ALL_52|CPU_251, 0x82, "DPL"},
  //{CPU_ALL_51|CPU_ALL_52|CPU_251, 0x83, "DPH"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x87, "PCON"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x88, "TCON"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x89, "TMOD"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8a, "TL0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8b, "TL1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8c, "TH0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8d, "TH1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x90, "P1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x98, "SCON"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x99, "SBUF"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xa0, "P2"},
  {CPU_ALL_51|CPU_ALL_52,         0xa8, "IE"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xb0, "P3"},
  {CPU_ALL_51|CPU_ALL_52,         0xb8, "IP"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd0, "PSW"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xe0, "ACC"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xf0, "B"},

  {CPU_ALL_52|CPU_251,            0xc8, "T2CON"},
  {CPU_ALL_52|CPU_251,            0xca, "RCAP2L"},
  {CPU_ALL_52|CPU_251,            0xcb, "RCAP2H"},
  {CPU_ALL_52|CPU_251,            0xcc, "TL2"},
  {CPU_ALL_52|CPU_251,            0xcd, "TH2"},

  {CPU_51R|CPU_89C51R,            0x8e, "AUXR"},
  {CPU_51R|CPU_89C51R|CPU_251,    0xa6, "WDTRST"},
  {CPU_51R|CPU_89C51R|CPU_251,    0xa9, "SADDR"},
  {CPU_51R|CPU_89C51R,            0xb7, "IPH"},
  {CPU_51R|CPU_89C51R|CPU_251,    0xb9, "SADEN"},
  {CPU_51R|CPU_89C51R|CPU_251|\
            CPU_DS390|CPU_DS390F, 0xc9, "T2MOD"}, /* fixme: isn't that CPU_ALL_52? */

  {CPU_89C51R,                    0xa2, "AUXR1"},
  {CPU_89C51R|CPU_251,            0xd8, "CCON"},
  {CPU_89C51R|CPU_251,            0xd9, "CMOD"},
  {CPU_89C51R|CPU_251,            0xda, "CCAPM0"},
  {CPU_89C51R|CPU_251,            0xdb, "CCAPM1"},
  {CPU_89C51R|CPU_251,            0xdc, "CCAPM2"},
  {CPU_89C51R|CPU_251,            0xdd, "CCAPM3"},
  {CPU_89C51R|CPU_251,            0xde, "CCAPM4"},
  {CPU_89C51R|CPU_251,            0xe9, "CL"},
  {CPU_89C51R|CPU_251,            0xea, "CCAP0L"},
  {CPU_89C51R|CPU_251,            0xeb, "CCAP1L"},
  {CPU_89C51R|CPU_251,            0xec, "CCAP2L"},
  {CPU_89C51R|CPU_251,            0xed, "CCAP3L"},
  {CPU_89C51R|CPU_251,            0xee, "CCAP4L"},
  {CPU_89C51R|CPU_251,            0xf9, "CH"},
  {CPU_89C51R|CPU_251,            0xfa, "CCAP0H"},
  {CPU_89C51R|CPU_251,            0xfb, "CCAP1H"},
  {CPU_89C51R|CPU_251,            0xfc, "CCAP2H"},
  {CPU_89C51R|CPU_251,            0xfd, "CCAP3H"},
  {CPU_89C51R|CPU_251,            0xfe, "CCAP4H"},

  {CPU_F380, 0x8e, "CKCON"},
  {CPU_F380, 0x8f, "PSCTL"},
  {CPU_F380, 0x91, "TMR3CN"},
  {CPU_F380, 0x91, "TMR4CN"},
  {CPU_F380, 0x92, "TMR3RLL"},
  {CPU_F380, 0x92, "TMR4RLL"},
  {CPU_F380, 0x93, "TMR3RLH"},
  {CPU_F380, 0x93, "TMR4RLH"},
  {CPU_F380, 0x94, "TMR3L"},
  {CPU_F380, 0x94, "TMR4L"},
  {CPU_F380, 0x95, "TMR3H"},
  {CPU_F380, 0x95, "TMR4H"},
  {CPU_F380, 0x96, "USB0ADR"},
  {CPU_F380, 0x97, "USB0DAT"},
  {CPU_F380, 0x9a, "CPT1CN"},
  {CPU_F380, 0x9b, "CPT0CN"},
  {CPU_F380, 0x9c, "CPT1MD"},
  {CPU_F380, 0x9d, "CPT0MD"},
  {CPU_F380, 0x9e, "CPT1MX"},
  {CPU_F380, 0x9f, "CPT0MX"},
  {CPU_F380, 0xa1, "SPI0CFG"},
  {CPU_F380, 0xa2, "SPI0CKR"},
  {CPU_F380, 0xa3, "SPI0DAT"},
  {CPU_F380, 0xa4, "P0MDOUT"},
  {CPU_F380, 0xa5, "P1MDOUT"},
  {CPU_F380, 0xa6, "P2MDOUT"},
  {CPU_F380, 0xa7, "P3MDOUT"},
  {CPU_F380, 0xa9, "CLKSEL"},
  {CPU_F380, 0xaa, "EMI0CN"},
  {CPU_F380, 0xac, "SBCON1"},
  {CPU_F380, 0xae, "P4MDOUT"},
  {CPU_F380, 0xaf, "PFE0CN"},
  {CPU_F380, 0xb1, "OSCXCN"},
  {CPU_F380, 0xb2, "OSCICN"},
  {CPU_F380, 0xb3, "OSCICL"},
  {CPU_F380, 0xb4, "SBRLL1"},
  {CPU_F380, 0xb5, "SBRLH1"},
  {CPU_F380, 0xb6, "FLSCL"},
  {CPU_F380, 0xb7, "FLKEY"},
  {CPU_F380, 0xb9, "CLKMUL"},
  {CPU_F380, 0xb9, "SMBTC"},
  {CPU_F380, 0xba, "AMX0N"},
  {CPU_F380, 0xbb, "AMX0P"},
  {CPU_F380, 0xbc, "ADC0CF"},
  {CPU_F380, 0xbd, "ADC0L"},
  {CPU_F380, 0xbe, "ADC0H"},
  {CPU_F380, 0xbf, "SFRPAGE"},
  {CPU_F380, 0xc0, "SMB0CN"},
  {CPU_F380, 0xc0, "SMB1CN"},
  {CPU_F380, 0xc1, "SMB0CF"},
  {CPU_F380, 0xc1, "SMB1CF"},
  {CPU_F380, 0xc2, "SMB0DAT"},
  {CPU_F380, 0xc2, "SMB1DAT"},
  {CPU_F380, 0xc3, "ADC0GTL"},
  {CPU_F380, 0xc4, "ADC0GTH"},
  {CPU_F380, 0xc5, "ADC0LTL"},
  {CPU_F380, 0xc6, "ADC0LTH"},
  {CPU_F380, 0xc7, "P4"},
  {CPU_F380, 0xd1, "REF0CN"},
  {CPU_F380, 0xd2, "SCON1"},
  {CPU_F380, 0xd3, "SBUF1"},
  {CPU_F380, 0xd4, "P0SKIP"},
  {CPU_F380, 0xd5, "P1SKIP"},
  {CPU_F380, 0xd6, "P2SKIP"},
  {CPU_F380, 0xd7, "USB0XCN"},
  {CPU_F380, 0xd8, "PCA0CN"},
  {CPU_F380, 0xd9, "PCA0MD"},
  {CPU_F380, 0xda, "PCA0CPM0"},
  {CPU_F380, 0xdb, "PCA0CPM1"},
  {CPU_F380, 0xdc, "PCA0CPM2"},
  {CPU_F380, 0xdd, "PCA0CPM3"},
  {CPU_F380, 0xde, "PCA0CPM4"},
  {CPU_F380, 0xdf, "P3SKIP"},
  {CPU_F380, 0xe1, "XBR0"},
  {CPU_F380, 0xe2, "XBR1"},
  {CPU_F380, 0xe3, "XBR2"},
  {CPU_F380, 0xe4, "IT01CF"},
  {CPU_F380, 0xe4, "CKCON1"},
  {CPU_F380, 0xe5, "SMOD1"},
  {CPU_F380, 0xe6, "EIE1"},
  {CPU_F380, 0xe7, "EIE2"},
  {CPU_F380, 0xe8, "ADC0CN"},
  {CPU_F380, 0xe9, "PCA0CPL1"},
  {CPU_F380, 0xea, "PCA0CPH1"},
  {CPU_F380, 0xeb, "PCA0CPL2"},
  {CPU_F380, 0xec, "PCA0CPH2"},
  {CPU_F380, 0xed, "PCA0CPL3"},
  {CPU_F380, 0xee, "PCA0CPH3"},
  {CPU_F380, 0xef, "RSTSRC"},
  {CPU_F380, 0xf1, "P0MDIN"},
  {CPU_F380, 0xf2, "P1MDIN"},
  {CPU_F380, 0xf3, "P2MDIN"},
  {CPU_F380, 0xf4, "P3MDIN"},
  {CPU_F380, 0xf5, "P4MDIN"},
  {CPU_F380, 0xf6, "EIP1"},
  {CPU_F380, 0xf7, "EIP2"},
  {CPU_F380, 0xf8, "SPI0CN"},
  {CPU_F380, 0xf9, "PCA0L"},
  {CPU_F380, 0xfa, "PCA0H"},
  {CPU_F380, 0xfb, "PCA0CPL0"},
  {CPU_F380, 0xfc, "PCA0CPH0"},
  {CPU_F380, 0xfd, "PCA0CPL4"},
  {CPU_F380, 0xfe, "PCA0CPH4"},
  {CPU_F380, 0xff, "VDM0CN"},
  
  {0, 0, NULL}
};

/*
 * Names of bits
 */

static struct name_entry bit_tab51[]=
{
  /* PSW */
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd7, "CY"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd6, "AC"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd5, "F0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd4, "RS1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd3, "RS0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd2, "OV"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd1, "F1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xd0, "P"},
  /* TCON */
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8f, "TF1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8e, "TR1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8d, "TF0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8c, "TR0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8b, "IE1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x8a, "IT1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x89, "IE0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x88, "IT0"},
  /* IE */
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xaf, "EA"},
  {CPU_DS390|CPU_DS390F,          0xae, "ES1"},
  {CPU_89C51R|CPU_251,            0xae, "EC"},
  {CPU_ALL_52|CPU_251,            0xad, "ET2"},
  {CPU_DS390|CPU_DS390F,          0xac, "ES0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xac, "ES"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xab, "ET1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xaa, "EX1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xa9, "ET0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0xa8, "EX0"},
  /* IP */
  {CPU_89C51R|CPU_251,    0xbe, "PPC"},
  {CPU_DS390|CPU_DS390F,  0xbe, "PS1"},
  {CPU_ALL_52,            0xbd, "PT2"},
  {CPU_DS390|CPU_DS390F,  0xbc, "PS0"},
  {CPU_ALL_51|CPU_ALL_52, 0xbc, "PS"},
  {CPU_ALL_51|CPU_ALL_52, 0xbb, "PT1"},
  {CPU_ALL_51|CPU_ALL_52, 0xba, "PX1"},
  {CPU_ALL_51|CPU_ALL_52, 0xb9, "PT0"},
  {CPU_ALL_51|CPU_ALL_52, 0xb8, "PX0"},
  /* IPL0 */
  {CPU_251, 0xbe, "IPL0.6"},
  {CPU_251, 0xbd, "IPL0.5"},
  {CPU_251, 0xbc, "IPL0.4"},
  {CPU_251, 0xbb, "IPL0.3"},
  {CPU_251, 0xba, "IPL0.2"},
  {CPU_251, 0xb9, "IPL0.1"},
  {CPU_251, 0xb8, "IPL0.0"},
  /* SCON */
  {CPU_DS390|CPU_DS390F,          0x9f, "SM0/FE_0"},
  {CPU_DS390|CPU_DS390F,          0x9e, "SM1_0"},
  {CPU_DS390|CPU_DS390F,          0x9d, "SM2_0"},
  {CPU_DS390|CPU_DS390F,          0x9c, "REN_0"},
  {CPU_DS390|CPU_DS390F,          0x9b, "TB8_0"},
  {CPU_DS390|CPU_DS390F,          0x9a, "RB8_0"},
  {CPU_DS390|CPU_DS390F,          0x99, "TI_0"},
  {CPU_DS390|CPU_DS390F,          0x98, "RI_0"},

  {CPU_51R|CPU_89C51R|CPU_251,    0x9f, "FE/SM0"},
  {CPU_ALL_51|CPU_ALL_52,         0x9f, "SM0"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x9e, "SM1"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x9d, "SM2"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x9c, "REN"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x9b, "TB8"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x9a, "RB8"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x99, "TI"},
  {CPU_ALL_51|CPU_ALL_52|CPU_251, 0x98, "RI"},
  /* SCON 1 */
  {CPU_DS390|CPU_DS390F,          0xc7, "SM0/FE_1"},
  {CPU_DS390|CPU_DS390F,          0xc6, "SM1_1"},
  {CPU_DS390|CPU_DS390F,          0xc5, "SM2_1"},
  {CPU_DS390|CPU_DS390F,          0xc4, "REN_1"},
  {CPU_DS390|CPU_DS390F,          0xc3, "TB8_1"},
  {CPU_DS390|CPU_DS390F,          0xc2, "RB8_1"},
  {CPU_DS390|CPU_DS390F,          0xc1, "TI_1"},
  {CPU_DS390|CPU_DS390F,          0xc0, "RI_1"},
  /* T2CON */
  {CPU_ALL_52|CPU_251, 0xcf, "TF2"},
  {CPU_ALL_52|CPU_251, 0xce, "EXF2"},
  {CPU_ALL_52|CPU_251, 0xcd, "RCLK"},
  {CPU_ALL_52|CPU_251, 0xcc, "TCLK"},
  {CPU_ALL_52|CPU_251, 0xcb, "EXEN2"},
  {CPU_ALL_52|CPU_251, 0xca, "TR2"},
  {CPU_ALL_52|CPU_251, 0xc9, "C/T2"},
  {CPU_ALL_52|CPU_251, 0xc8, "CP/RL2"},
  /* CCON */
  {CPU_89C51R|CPU_251, 0xdf, "CF"},
  {CPU_89C51R|CPU_251, 0xde, "CR"},
  {CPU_89C51R|CPU_251, 0xdc, "CCF4"},
  {CPU_89C51R|CPU_251, 0xdb, "CCF3"},
  {CPU_89C51R|CPU_251, 0xda, "CCF2"},
  {CPU_89C51R|CPU_251, 0xd9, "CCF1"},
  {CPU_89C51R|CPU_251, 0xd8, "CCF0"},
  /* P1 */
  {CPU_89C51R|CPU_251, 0x97, "CEX4"},
  {CPU_89C51R|CPU_251, 0x96, "CEX3"},
  {CPU_89C51R|CPU_251, 0x95, "CEX2"},
  {CPU_89C51R|CPU_251, 0x94, "CEX1"},
  {CPU_89C51R|CPU_251, 0x93, "CEX0"},
  {CPU_89C51R|CPU_251, 0x92, "EXI"},
  {CPU_89C51R|CPU_251, 0x91, "T2EX"},
  {CPU_89C51R|CPU_251, 0x90, "T2"},
  /* WDCON */
  {CPU_DS390|CPU_DS390F, 0xdf, "SMOD_1"},
  {CPU_DS390|CPU_DS390F, 0xde, "POR,"},
  {CPU_DS390|CPU_DS390F, 0xdd, "EPF1"},
  {CPU_DS390|CPU_DS390F, 0xdc, "PF1"},
  {CPU_DS390|CPU_DS390F, 0xdb, "WDIF"},
  {CPU_DS390|CPU_DS390F, 0xda, "WTRF"},
  {CPU_DS390|CPU_DS390F, 0xd9, "EWT"},
  {CPU_DS390|CPU_DS390F, 0xd8, "RWT"},
  /* EIE */
  {CPU_DS390|CPU_DS390F, 0xef, "CANBIE"},
  {CPU_DS390|CPU_DS390F, 0xee, "C0IE"},
  {CPU_DS390|CPU_DS390F, 0xed, "C1IE"},
  {CPU_DS390|CPU_DS390F, 0xec, "EWDI"},
  {CPU_DS390|CPU_DS390F, 0xeb, "EX5"},
  {CPU_DS390|CPU_DS390F, 0xea, "EX4"},
  {CPU_DS390|CPU_DS390F, 0xe9, "EX3"},
  {CPU_DS390|CPU_DS390F, 0xe8, "EX2"},
  /* EIP */
  {CPU_DS390|CPU_DS390F, 0xef, "CANBIP"},
  {CPU_DS390|CPU_DS390F, 0xee, "C0IP"},
  {CPU_DS390|CPU_DS390F, 0xed, "C1IP"},
  {CPU_DS390|CPU_DS390F, 0xec, "PWDI"},
  {CPU_DS390|CPU_DS390F, 0xeb, "PX5"},
  {CPU_DS390|CPU_DS390F, 0xea, "PX4"},
  {CPU_DS390|CPU_DS390F, 0xe9, "PX3"},
  {CPU_DS390|CPU_DS390F, 0xe8, "PX2"},

  {0, 0, NULL}
};


/*
 * Options of uc51
 */

cl_irq_stop_option::cl_irq_stop_option(class cl_51core *the_uc51):
  cl_optref(the_uc51)
{
  uc51= the_uc51;
}

int
cl_irq_stop_option::init(void)
{
  cl_optref::init();
  create(uc51, bool_opt, "irq_stop", "Stop when IRQ accepted");
  
  return(0);
}

void
cl_irq_stop_option::option_changed(void)
{
  if (!uc51)
    return;
  bool b;
  option->get_value(&b);
  uc51->stop_at_it= b;
}

instruction_wrapper_fn itab51[256];

/*
 * Making a new micro-controller and reset it
 */

cl_51core::cl_51core(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc(asim)
{
  type= Itype;

  irq_stop_option= new cl_irq_stop_option(this);
  stop_at_it= false;
}


/*
 * Initializing. Virtual calls go here
 * This method must be called first after object creation.
 */

int
cl_51core::init(void)
{
  fill_def_wrappers(itab51);
  itab51[0x89]= itab51[0x88];
  itab51[0x8a]= itab51[0x88];
  itab51[0x8b]= itab51[0x88];
  itab51[0x8c]= itab51[0x88];
  itab51[0x8d]= itab51[0x88];
  itab51[0x8e]= itab51[0x88];
  itab51[0x8f]= itab51[0x88];

  itab51[0xf9]= itab51[0xf8];
  itab51[0xfa]= itab51[0xf8];
  itab51[0xfb]= itab51[0xf8];
  itab51[0xfc]= itab51[0xf8];
  itab51[0xfd]= itab51[0xf8];
  itab51[0xfe]= itab51[0xf8];
  itab51[0xff]= itab51[0xf8];

  itab51[0x21]= itab51[0x01];
  itab51[0x41]= itab51[0x01];
  itab51[0x61]= itab51[0x01];
  itab51[0x81]= itab51[0x01];
  itab51[0xa1]= itab51[0x01];
  itab51[0xc1]= itab51[0x01];
  itab51[0xe1]= itab51[0x01];

  itab51[0x07]= itab51[0x06];

  itab51[0x09]= itab51[0x08];
  itab51[0x0a]= itab51[0x08];
  itab51[0x0b]= itab51[0x08];
  itab51[0x0c]= itab51[0x08];
  itab51[0x0d]= itab51[0x08];
  itab51[0x0e]= itab51[0x08];
  itab51[0x0f]= itab51[0x08];

  itab51[0x31]= itab51[0x11];
  itab51[0x51]= itab51[0x11];
  itab51[0x71]= itab51[0x11];
  itab51[0x91]= itab51[0x11];
  itab51[0xb1]= itab51[0x11];
  itab51[0xd1]= itab51[0x11];
  itab51[0xf1]= itab51[0x11];

  itab51[0x17]= itab51[0x16];

  itab51[0x19]= itab51[0x18];
  itab51[0x1a]= itab51[0x18];
  itab51[0x1b]= itab51[0x18];
  itab51[0x1c]= itab51[0x18];
  itab51[0x1d]= itab51[0x18];
  itab51[0x1e]= itab51[0x18];
  itab51[0x1f]= itab51[0x18];

  itab51[0x29]= itab51[0x28];
  itab51[0x2a]= itab51[0x28];
  itab51[0x2b]= itab51[0x28];
  itab51[0x2c]= itab51[0x28];
  itab51[0x2d]= itab51[0x28];
  itab51[0x2e]= itab51[0x28];
  itab51[0x2f]= itab51[0x28];

  itab51[0x27]= itab51[0x26];

  itab51[0x37]= itab51[0x36];
  
  itab51[0x39]= itab51[0x38];
  itab51[0x3a]= itab51[0x38];
  itab51[0x3b]= itab51[0x38];
  itab51[0x3c]= itab51[0x38];
  itab51[0x3d]= itab51[0x38];
  itab51[0x3e]= itab51[0x38];
  itab51[0x3f]= itab51[0x38];

  itab51[0x47]= itab51[0x46];

  itab51[0x49]= itab51[0x48];
  itab51[0x4a]= itab51[0x48];
  itab51[0x4b]= itab51[0x48];
  itab51[0x4c]= itab51[0x48];
  itab51[0x4d]= itab51[0x48];
  itab51[0x4e]= itab51[0x48];
  itab51[0x4f]= itab51[0x48];

  itab51[0x97]= itab51[0x96];

  itab51[0x99]= itab51[0x98];
  itab51[0x9a]= itab51[0x98];
  itab51[0x9b]= itab51[0x98];
  itab51[0x9c]= itab51[0x98];
  itab51[0x9d]= itab51[0x98];
  itab51[0x9e]= itab51[0x98];
  itab51[0x9f]= itab51[0x98];

  itab51[0xb7]= itab51[0xb6];

  itab51[0xb9]= itab51[0xb8];
  itab51[0xba]= itab51[0xb8];
  itab51[0xbb]= itab51[0xb8];
  itab51[0xbc]= itab51[0xb8];
  itab51[0xbd]= itab51[0xb8];
  itab51[0xbe]= itab51[0xb8];
  itab51[0xbf]= itab51[0xb8];

  itab51[0xd9]= itab51[0xd8];
  itab51[0xda]= itab51[0xd8];
  itab51[0xdb]= itab51[0xd8];
  itab51[0xdc]= itab51[0xd8];
  itab51[0xdd]= itab51[0xd8];
  itab51[0xde]= itab51[0xd8];
  itab51[0xdf]= itab51[0xd8];

  itab51[0x57]= itab51[0x56];

  itab51[0x59]= itab51[0x58];
  itab51[0x5a]= itab51[0x58];
  itab51[0x5b]= itab51[0x58];
  itab51[0x5c]= itab51[0x58];
  itab51[0x5d]= itab51[0x58];
  itab51[0x5e]= itab51[0x58];
  itab51[0x5f]= itab51[0x58];

  itab51[0x67]= itab51[0x66];

  itab51[0x69]= itab51[0x68];
  itab51[0x6a]= itab51[0x68];
  itab51[0x6b]= itab51[0x68];
  itab51[0x6c]= itab51[0x68];
  itab51[0x6d]= itab51[0x68];
  itab51[0x6e]= itab51[0x68];
  itab51[0x6f]= itab51[0x68];

  itab51[0x77]= itab51[0x76];

  itab51[0x79]= itab51[0x78];
  itab51[0x7a]= itab51[0x78];
  itab51[0x7b]= itab51[0x78];
  itab51[0x7c]= itab51[0x78];
  itab51[0x7d]= itab51[0x78];
  itab51[0x7e]= itab51[0x78];
  itab51[0x7f]= itab51[0x78];

  itab51[0x87]= itab51[0x86];

  itab51[0xa7]= itab51[0xa6];

  itab51[0xa9]= itab51[0xa8];
  itab51[0xaa]= itab51[0xa8];
  itab51[0xab]= itab51[0xa8];
  itab51[0xac]= itab51[0xa8];
  itab51[0xad]= itab51[0xa8];
  itab51[0xae]= itab51[0xa8];
  itab51[0xaf]= itab51[0xa8];

  itab51[0xc7]= itab51[0xc6];

  itab51[0xc9]= itab51[0xc8];
  itab51[0xca]= itab51[0xc8];
  itab51[0xcb]= itab51[0xc8];
  itab51[0xcc]= itab51[0xc8];
  itab51[0xcd]= itab51[0xc8];
  itab51[0xce]= itab51[0xc8];
  itab51[0xcf]= itab51[0xc8];

  itab51[0xd7]= itab51[0xd6];

  itab51[0xe3]= itab51[0xe2];

  itab51[0xe7]= itab51[0xe6];

  itab51[0xe9]= itab51[0xe8];
  itab51[0xea]= itab51[0xe8];
  itab51[0xeb]= itab51[0xe8];
  itab51[0xec]= itab51[0xe8];
  itab51[0xed]= itab51[0xe8];
  itab51[0xee]= itab51[0xe8];
  itab51[0xef]= itab51[0xe8];

  itab51[0xf3]= itab51[0xf2];

  itab51[0xf7]= itab51[0xf6];

  irq_stop_option->init();
  dptr= 0;
  cl_uc::init();
  decode_dptr();
  set_name("mcs51_controller");
  reset();

  make_vars();

  return(0);
}

static char id_string_51[100];

char *
cl_51core::id_string(void)
{
  int i;

  for (i= 0;
       (cpus_51[i].type_str != NULL) &&
	 (cpus_51[i].type != type->type);
       i++) ;
  sprintf(id_string_51, "%s %s",
	  cpus_51[i].type_str?cpus_51[i].type_str:"51",
	  (type->subtype & CPU_HMOS)?"HMOS":"CMOS");
  return(id_string_51);
}

void
cl_51core::make_cpu_hw(void)
{
  cpu= new cl_uc51_cpu(this);
  cpu->init();
}

void
cl_51core::mk_hw_elements(void)
{
  cl_uc::mk_hw_elements();

  class cl_hw *h;

  acc= sfr->get_cell(ACC);
  psw= sfr->get_cell(PSW);

  add_hw(h= new cl_timer0(this, 0, "timer0"));
  h->init();
  add_hw(h= new cl_timer1(this, 1, "timer1"));
  h->init();
  add_hw(h= new cl_serial(this));
  h->init();

  class cl_port_ui *d;
  add_hw(d= new cl_port_ui(this, 0, "dport"));
  d->init();

  class cl_port *p0, *p1, *p2, *p3;
  add_hw(p0= new cl_port(this, 0));
  p0->init();
  add_hw(p1= new cl_port(this, 1));
  p1->init();
  add_hw(p2= new cl_port(this, 2));
  p2->init();
  add_hw(p3= new cl_port(this, 3));
  p3->init();

  class cl_port_data pd;
  pd.init();
  pd.cell_dir= NULL;

  pd.set_name("P0");
  pd.cell_p  = p0->cell_p;
  pd.cell_in = p0->cell_in;
  pd.keyset  = chars(keysets[0]);
  pd.basx    = 1;
  pd.basy    = 4;
  d->add_port(&pd, 0);
  
  pd.set_name("P1");
  pd.cell_p  = p1->cell_p;
  pd.cell_in = p1->cell_in;
  pd.keyset  = chars(keysets[1]);
  pd.basx    = 20;
  pd.basy    = 4;
  d->add_port(&pd, 1);
  
  pd.set_name("P2");
  pd.cell_p  = p2->cell_p;
  pd.cell_in = p2->cell_in;
  pd.keyset  = chars(keysets[2]);
  pd.basx    = 40;
  pd.basy    = 4;
  d->add_port(&pd, 2);
  
  pd.set_name("P3");
  pd.cell_p  = p3->cell_p;
  pd.cell_in = p3->cell_in;
  pd.keyset  = chars(keysets[3]);
  pd.basx    = 60;
  pd.basy    = 4;
  d->add_port(&pd, 3);
  
  add_hw(interrupt= new cl_interrupt(this));
  interrupt->init();
}

void
cl_51core::build_cmdset(class cl_cmdset *cmdset)
{
  class cl_cmd *cmd;
  //class cl_super_cmd *super_cmd;
  //class cl_cmdset *cset;

  cl_uc::build_cmdset(cmdset);

  cmdset->add(cmd= new cl_di_cmd("di", true));
  cmd->init();

  cmdset->add(cmd= new cl_dx_cmd("dx", true));
  cmd->init();

  cmdset->add(cmd= new cl_ds_cmd("ds", true));
  cmd->init();
}


void
cl_51core::make_memories(void)
{
  make_address_spaces();
  make_chips();
  
  acc= sfr->get_cell(ACC);
  psw= sfr->get_cell(PSW);

  decode_regs();
  decode_rom();
  decode_iram();
  decode_sfr();
  decode_xram();
  decode_bits();
}

void
cl_51core::make_address_spaces(void)
{
  rom= new cl_address_space("rom", 0, 0x10000, 8);
  rom->init();
  address_spaces->add(rom);
  
  iram= new cl_address_space("iram", 0, 0x80, 8);
  iram->init();
  address_spaces->add(iram);

  sfr= new cl_address_space("sfr", 0x80, 0x80, 8);
  sfr->init();
  address_spaces->add(sfr);

  xram= new cl_address_space("xram", 0, 0x10000, 8);
  xram->init();
  address_spaces->add(xram);

  regs= new cl_address_space("regs", 0, 8, 8);
  regs->init();
  address_spaces->add(regs);

  bits= new cl_address_space("bits", 0, 0x100, 1);
  bits->init();
  address_spaces->add(bits);

  dptr= new cl_address_space("dptr", 0, 2, 8);
  dptr->init();
  address_spaces->add(dptr);
}

void
cl_51core::make_chips(void)
{
  rom_chip= new cl_memory_chip("rom_chip", 0x10000, 8, 0/*, 0xff*/);
  rom_chip->init();
  memchips->add(rom_chip);
  
  iram_chip= new cl_memory_chip("iram_chip", 0x100, 8);
  iram_chip->init();
  memchips->add(iram_chip);

  xram_chip= new cl_memory_chip("xram_chip", 0x10000, 8);
  xram_chip->init();
  memchips->add(xram_chip);

  sfr_chip= new cl_memory_chip("sfr_chip", 0x80, 8);
  sfr_chip->init();
  memchips->add(sfr_chip);
}

void
cl_51core::decode_rom(void)
{
  class cl_address_decoder *ad;
  ad= new cl_address_decoder(rom, rom_chip, 0, 0xffff, 0);
  ad->init();
  ad->set_name("def_rom_decoder");
  rom->decoders->add(ad);
  ad->activate(0);
}

void
cl_51core::decode_regs(void)
{
  int i;
  cl_banker *b= new cl_banker(sfr, 0xd0, 0x18, //0,
			      regs, 0, 7);
  b->init();
  b->set_name("def_regs_banker");
  regs->decoders->add(b);
  b->add_bank(0, memory("iram_chip"), 0);
  b->add_bank(1, memory("iram_chip"), 8);
  b->add_bank(2, memory("iram_chip"), 16);
  b->add_bank(3, memory("iram_chip"), 24);
  psw->write(0);
  for (i= 0; i < 8; i++)
    R[i]= regs->get_cell(i);
}

void
cl_51core::decode_bits(void)
{
  class cl_address_decoder *ad;
  
  ad= new cl_bander(bits, 0, 127,
		    iram_chip, 32,
		    8, 1);
  ad->init();
  ad->set_name("def_bits_bander_0-7f");
  bits->decoders->add(ad);
  ad->activate(0);

  ad= new cl_bander(bits, 128, 255,
		    sfr_chip, 0,
		    8, 8);
  ad->init();
  ad->set_name("def_bits_bander_80-ff");
  bits->decoders->add(ad);
  ad->activate(0);
}

void
cl_51core::decode_iram(void)
{
  class cl_address_decoder *ad;
  
  ad= new cl_address_decoder(iram, iram_chip, 0, 0x7f, 0);
  ad->init();
  ad->set_name("def_iram_decoder");
  iram->decoders->add(ad);
  ad->activate(0);
}

void
cl_51core::decode_sfr(void)
{
  class cl_address_decoder *ad;
  
  ad= new cl_address_decoder(sfr, sfr_chip, 0x80, 0xff, 0);
  ad->init();
  ad->set_name("def_sfr_decoder");
  sfr->decoders->add(ad);
  ad->activate(0);
}

void
cl_51core::decode_xram(void)
{
  class cl_address_decoder *ad;
  
  ad= new cl_address_decoder(xram, xram_chip, 0, 0xffff, 0);
  ad->init();
  ad->set_name("def_xram_decoder");
  xram->decoders->add(ad);
  ad->activate(0);
}

void
cl_51core::decode_dptr(void)
{
  class cl_address_decoder *ad;
  t_mem adps= 0, mdps, dpl1, dph1, mdpc, adpc;
  class cl_banker *banker;

  dptr->undecode_area(NULL, 0, 1, NULL);
  
  if (cpu)
    {
      adps= cpu->cfg_get(uc51cpu_aof_mdps);
      mdps= cpu->cfg_get(uc51cpu_mask_mdps);
      dpl1= cpu->cfg_get(uc51cpu_aof_mdps1l);
      dph1= cpu->cfg_get(uc51cpu_aof_mdps1h);

      adpc= cpu->cfg_get(uc51cpu_aof_mdpc);
      mdpc= cpu->cfg_get(uc51cpu_mask_mdpc);
      
      if ((adps > 0x7f) &&
	  (dpl1 > 0x7f) &&
	  (dph1 > 0x7f))
	{
	  // multi DPTR sfr style
	  //printf("MDPS %x %x %x %x\n", adps, mdps, dpl1, dph1);
	  banker= new cl_banker(sfr, adps, mdps, //0,
				dptr, 0, 0);
	  banker->init();
	  dptr->decoders->add(banker);
	  banker->add_bank(0, memory("sfr_chip"), DPL-0x80);
	  banker->add_bank(1, memory("sfr_chip"), dpl1-0x80);
	  banker->activate(0);

	  banker= new cl_banker(sfr, adps, mdps, //0,
				dptr, 1, 1);
	  banker->init();
	  dptr->decoders->add(banker);
	  banker->add_bank(0, memory("sfr_chip"), DPH-0x80);
	  banker->add_bank(1, memory("sfr_chip"), dph1-0x80);
	  banker->activate(0);

	  sfr->write(adps, sfr->get(adps));
	}
      else if (adpc > 0x7f)
	{
	  // multi DPTR chip style
	  adps=0x80;
	  class cl_memory_chip *dptr_chip= (cl_memory_chip*)memory("dptr_chip");
	  if (dptr_chip == 0)
	    {
	      dptr_chip= new cl_memory_chip("dptr_chip", 3*8, 8);
	      dptr_chip->init();
	      memchips->add(dptr_chip);
	    }
	  if (dptr_chip &&
	      (mdpc != 0))
	    {
	      int a, m= mdpc;
	      //printf("MDPC %x %x\n", adpc, mdpc);
	      while ((m&1) == 0)
		m>>= 1;
	      
	      banker= new cl_banker(sfr, adpc, mdpc, //0,
			    dptr, 0, 1);
	      banker->init();
	      dptr->decoders->add(banker);
	      for (a= 0; a <= m; a++)
		banker->add_bank(a, dptr_chip, a*2);
	      banker->activate(0);

	      banker= new cl_banker(sfr, adpc, mdpc, //0,
			    sfr, DPL, DPH);
	      banker->init();
	      sfr->decoders->add(banker);
	      for (a= 0; a <= m; a++)
		banker->add_bank(a, dptr_chip, a*2);
	      banker->activate(0);
	      
	      sfr->write(adpc, sfr->get(adpc));
	    }
	}
      else
	adps= 0;
    }
  if (adps == 0)
    {
      //printf("DPTR\n");
      ad= new cl_address_decoder(dptr, sfr_chip, 0, 1, DPL-0x80);
      ad->init();
      dptr->decoders->add(ad);
      ad->activate(0);
    }
  
  cl_var *v;
  vars->add(v= new cl_var(chars("dpl"), dptr, 0, ""));
  v->init();
  vars->add(v= new cl_var(chars("DPL"), dptr, 0, ""));
  v->init();
  vars->add(v= new cl_var(chars("dph"), dptr, 1, ""));
  v->init();
  vars->add(v= new cl_var(chars("DPH"), dptr, 1, ""));
  v->init();
}

void
cl_51core::make_vars(void)
{
  cl_var *v;

  vars->add(v= new cl_var(cchars("R0"), regs, 0, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R1"), regs, 1, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R2"), regs, 2, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R3"), regs, 3, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R4"), regs, 4, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R5"), regs, 5, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R6"), regs, 6, ""));
  v->init();
  vars->add(v= new cl_var(cchars("R7"), regs, 7, ""));
  v->init();

  int i;
  for (i= 0; sfr_tab51[i].name != NULL; i++)
    {
      if (type->type & sfr_tab51[i].cpu_type)
	{
	  vars->add(v= new cl_var(chars(sfr_tab51[i].name),
				  sfr,
				  sfr_tab51[i].addr, ""));
	  v->init();
	}
    }
  for (i= 0; bit_tab51[i].name != NULL; i++)
    {
      if (type->type & bit_tab51[i].cpu_type)
	{
	  vars->add(v= new cl_var(chars(bit_tab51[i].name),
				  bits,
				  bit_tab51[i].addr, ""));
	  v->init();
	}
    }
}

/*
 * Destroying the micro-controller object
 */

cl_51core::~cl_51core(void)
{
  delete irq_stop_option;
}


/*
 * Disassembling an instruction
 */

struct dis_entry *
cl_51core::dis_tbl(void)
{
  return(disass_51);
}

struct name_entry *
cl_51core::bit_tbl(void)
{
  return(bit_tab51);
}

char *
cl_51core::disass(t_addr addr, const char *sep)
{
  char work[256], temp[200]/*, c[2]*/;
  char *buf, *p, *t, *s;
  const char *b;
  t_mem code= rom->get(addr);

  p= work;
  b= dis_tbl()[code].mnemonic;
  while (*b)
    {
      if (*b == '%')
	{
	  b++;
	  switch (*(b++))
	    {
	    case 'A': // absolute address
	      sprintf(temp, "%04x",
		      /*t_addr*/int((addr&0xf800)|
				    (((code>>5)&0x07)*256 +
				     rom->get(addr+1))));
	      break;
	    case 'l': // long address
	      sprintf(temp, "%04x",
		      /*t_addr*/int(rom->get(addr+1)*256 +
				    rom->get(addr+2)));
	      break;
	    case 'a': // addr8 (direct address) at 2nd byte
	      daddr_name(rom->get(addr+1), temp);
	      break;
	    case '8': // addr8 (direct address) at 3rd byte
	      daddr_name(rom->get(addr+2), temp);
	      break;
	    case 'b': // bitaddr at 2nd byte
	      {
		t_addr ba= rom->get(addr+1);
		/*if (get_name(ba, bit_tbl(), temp))
		  break;
		if (ba<128)
		  addr_name((ba/8)+32,iram,temp);
		else
		  addr_name(ba&0xf8,sfr,temp);
		strcat(temp, ".");
		sprintf(c, "%1d", (int)(ba & 0x07));
		strcat(temp, c);*/
		baddr_name(ba, temp);
		break;
	      }
	    case 'r': // rel8 address at 2nd byte
	      sprintf(temp, "%04x",
		      /*t_addr*/int(addr+2+(signed char)(rom->get(addr+1))));
	      break;
	    case 'R': // rel8 address at 3rd byte
	      sprintf(temp, "%04x",
		      /*t_addr*/int(addr+3+(signed char)(rom->get(addr+2))));
	      break;
	    case 'd': // data8 at 2nd byte
	      sprintf(temp, "%02x", (int)rom->get(addr+1));
	      break;
	    case 'D': // data8 at 3rd byte
	      sprintf(temp, "%02x", (int)rom->get(addr+2));
	      break;
	    case '6': // data16 at 2nd(H)-3rd(L) byte
	      sprintf(temp, "%04x",
		      /*t_addr*/int(rom->get(addr+1)*256 +
				    rom->get(addr+2)));
	      break;
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
cl_51core::print_regs(class cl_console_base *con)
{
  t_addr start, stop;
  t_mem data;
  t_mem dp;
  
  // show regs
  start= psw->get() & 0x18;
  con->dd_printf("     R0 R1 R2 R3 R4 R5 R6 R7\n");
  iram->dump(start, start+7, 8, con/*->get_fout()*/);
  con->dd_color("answer");
  // show indirectly addressed IRAM and some basic regs
  data= iram->get(iram->get(start));
  con->dd_printf("@R0 %02x %c", data, isprint(data) ? data : '.');

  con->dd_printf("  ACC= 0x%02x %3d %c  B= 0x%02x\n", sfr->get(ACC), sfr->get(ACC),
              isprint(sfr->get(ACC))?(sfr->get(ACC)):'.', sfr->get(B));
  data= iram->get(iram->get(start+1));
  con->dd_printf("@R1 %02x %c", data, isprint(data) ? data : '.');
  data= psw->get();
  con->dd_printf("  PSW= 0x%02x CY=%c AC=%c OV=%c P=%c\n", data,
              (data&bmCY)?'1':'0', (data&bmAC)?'1':'0',
              (data&bmOV)?'1':'0', (data&bmP)?'1':'0');
  /* show stack pointer */
  start= sfr->get (SP);
  if (start >= 7)
    stop = start-7;
  else
    stop= 0;
  con->dd_printf ("SP ");
  iram->dump (start, stop, 8, con/*->get_fout()*/);
  con->dd_color("answer");
  // show DPTR(s)
  if (dptr)
    {
      int act;
      int mask;
	      int i;
      if (cpu &&
	  (cpu->cfg_get(uc51cpu_aof_mdpc) > 0x7f))
	{
	  // multi DPTR chip style
	  act= sfr->get(cpu->cfg_get(uc51cpu_aof_mdpc));
	  mask= cpu->cfg_get(uc51cpu_mask_mdpc);
	  while ((mask&1) == 0)
	    {
	      act>>= 1;
	      mask>>= 1;
	    }
	  act&= mask;
	  class cl_memory *dptr_chip= memory("dptr_chip");
	  if (dptr_chip)
	    {
	      for (i= 0; i <= mask; i++)
		{
		  int a= i*dptr->get_size();
		  dp= 0;
		  int di;
		  for (di= dptr->get_size()-1; di >= 0; di--)
		    dp= (dp*256) + dptr_chip->get(a+di);
		  con->dd_printf("  %cDPTR%d= ", (i==act)?'*':' ', i);
		  con->dd_printf(xram->addr_format, dp);
		  data= xram->read(dp);
		  con->dd_printf(" @DPTR%d= ", i);
		  con->dd_printf("0x%02x %3d %c\n", data, data,
				 isprint(data)?data:'.');
		}
	    }
	}
      else if (cpu &&
	  (cpu->cfg_get(uc51cpu_aof_mdps) > 0x7f))
	{
	  // multi DPTR sfr style
	  act= sfr->get(cpu->cfg_get(uc51cpu_aof_mdps));
	  mask= cpu->cfg_get(uc51cpu_mask_mdps);
	  while ((mask&1) == 0)
	    {
	      act>>= 1;
	      mask>>= 1;
	    }
	  act&= mask;
	  i= 0;
	  dp= (sfr_chip->get(DPL-0x80) +
	       sfr_chip->get(DPH-0x80) * 256) & 0xffff;
	  con->dd_printf("  %cDPTR%d= ", (i==act)?'*':' ', i);
	  con->dd_printf(xram->addr_format, dp);
	  data= xram->read(dp);
	  con->dd_printf(" @DPTR%d= ", i);
	  con->dd_printf("0x%02x %3d %c\n", data, data,
			 isprint(data)?data:'.');
	  i= 1;
	  dp= sfr_chip->get(cpu->cfg_get(uc51cpu_aof_mdps1l) - 0x80) +
	    sfr_chip->get(cpu->cfg_get(uc51cpu_aof_mdps1h) - 0x80) * 256;
	  con->dd_printf("  %cDPTR%d= ", (i==act)?'*':' ', i);
	  con->dd_printf(xram->addr_format, dp);
	  data= xram->read(dp);
	  con->dd_printf(" @DPTR%d= ", i);
	  con->dd_printf("0x%02x %3d %c\n", data, data,
			 isprint(data)?data:'.');
	}
      else
	{
	  // non-multi DPTR
	  int a= dptr->get_size();
	  dp= 0;
	  int di;
	  chars f="";
	  for (di= a-1; di >= 0; di--)
	    {
	      dp= (dp*256) + dptr->get(di);
	    }
	  f.format("0x%%0%dx",a*2);
	  data= xram->get(dp);
	  con->dd_printf("   DPTR= ");
	  con->dd_printf(/*xram->addr_format*/(char*)f, dp);
	  con->dd_printf(" @DPTR= 0x%02x %3d %c\n",
			 data, data, isprint(data)?data:'.');
	}
    }
  else
    {
      // no dptr address space, read SFR directly
      data= xram->get(sfr->read(DPH)*256+sfr->read(DPL));
      con->dd_printf("   DPTR= 0x%02x%02x @DPTR= 0x%02x %3d %c\n",
		     sfr->get(DPH),
		     sfr->get(DPL),
		     data, data, isprint(data)?data:'.');
    }
  
  print_disass(PC, con);
}


/*
 * Converting bit address into real memory
 */

class cl_address_space *
cl_51core::bit2mem(t_addr bitaddr, t_addr *memaddr, t_mem *bitmask)
{
  class cl_address_space *m;
  t_addr ma;

  bitaddr&= 0xff;
  if (bitaddr < 128)
    {
      m= iram;
      ma= bitaddr/8 + 0x20;
    }
  else
    {
      m= sfr;
      ma= bitaddr & 0xf8;
    }
  if (memaddr)
    *memaddr= ma;
  if (bitmask)
    *bitmask= 1 << (bitaddr & 0x7);
  return(m);
}

t_addr
cl_51core::bit_address(class cl_memory *mem,
		       t_addr mem_address, int bit_number)
{
  if (bit_number < 0 ||
      bit_number > 7 ||
      mem_address < 0)
    return(-1);
  class cl_memory *sfrchip= memory("sfr_chip");
  if (mem == sfrchip)
    {
      mem= sfr;
      mem_address+= sfr->start_address;
    }
  if (mem == sfr)
    {
      if (mem_address < 128 ||
	  mem_address % 8 != 0 ||
	  mem_address > 255)
	return(-1);
      return(128 + (mem_address-128) + bit_number);
    }
  if (mem == iram)
    {
      if (mem_address < 0x20 ||
	  mem_address >= 0x20+32)
	return(-1);
      return((mem_address-0x20)*8 + bit_number);
    }
  return(-1);
}

/* Get name of directly addressed iram/sfr cell */

void
cl_51core::daddr_name(t_addr addr, char *buf)
{
  if (!buf)
    return;
  if (addr < 128)
    {
      // register?
      if (addr_name(addr, regs, buf))
	return;
      // variale?
      if (addr_name(addr, iram, buf))
	return;
    }
  else
    {
      // dptr?
      if (addr_name(addr-0x82, dptr, buf))
	return;
      // sfr?
      if (addr_name(addr, sfr, buf))
	return;
    }
  unsigned int a= addr;
  sprintf(buf, "%02x", a);
}

/* Get name of a bit cell */

void
cl_51core::baddr_name(t_addr addr, char *buf)
{
  t_addr ma;
  
  if (!buf)
    return;
  if (addr_name(addr, bits, buf))
    return;
  if (addr < 128)
    ma= 32+(addr/8);
  else
    ma= addr&0xf8;
  daddr_name(ma, buf);
  chars c= chars("", "%s.%d", buf, (int)(addr & 7));
  strcpy(buf, (char*)c);
}


/*
 * Resetting the micro-controller
 */

void
cl_51core::reset(void)
{
  cl_uc::reset();

  clear_sfr();

  result= resGO;

  //was_reti= false;
}


/*
 * Setting up SFR area to reset value
 */

void
cl_51core::clear_sfr(void)
{
  int i;

  for (i= 0x80; i <= 0xff; i++)
    sfr->set(i, 0);
  sfr->/*set*/write(P0, 0xff);
  sfr->/*set*/write(P1, 0xff);
  sfr->/*set*/write(P2, 0xff);
  sfr->/*set*/write(P3, 0xff);
  prev_p1= /*port_pins[1] &*/ sfr->/*get*/read(P1);
  prev_p3= /*port_pins[3] &*/ sfr->/*get*/read(P3);
  sfr->write(ACC, 0);
  sfr->write(B, 0);
  sfr->write(PSW, 0);
  sfr->write(SP, 7);
  sfr->write(DPL, 0);
  sfr->write(DPH, 0);
  sfr->write(IP, 0);
  sfr->write(IE, 0);
  sfr->write(TMOD, 0);
  sfr->write(TCON, 0);
  sfr->write(TH0, 0);
  sfr->write(TL0, 0);
  sfr->write(TH1, 0);
  sfr->write(TL1, 0);
  sfr->write(SCON, 0);
  sfr->write(PCON, 0);

  sfr->set_nuof_writes(0);
  sfr->set_nuof_reads(0);
}


/*
 * Analyzing code and settig up instruction map
 */

void
cl_51core::analyze(t_addr addr)
{
  uint code;
  struct dis_entry *tabl;
  t_addr a;
  
  code= rom->get(addr);
  tabl= &(dis_tbl()[code]);
  while (!inst_at(addr) &&
	 code != 0xa5 /* break point */)
    {
      set_inst_at(addr);
      switch (tabl->branch)
	{
	case 'a': // acall
	  a= (addr & 0xf800)|
	    ((rom->get(addr+1)&0x07)*256+
	     rom->get(addr+2));
	  analyze(a);
	  addr= addr+tabl->length;
	  break;
	case 'A': // ajmp
	  a= (addr & 0xf800)|
	    (((rom->get(addr)>>5) & 0x07)*256 + rom->get(addr+1));
	  addr= a;
	  break;
	case 'l': // lcall
	  a= rom->get(addr+1)*256 + rom->get(addr+2);
	  analyze(a);
	  addr= addr+tabl->length;
	  break;
	case 'L': // ljmp
	  a= rom->get(addr+1)*256 + rom->get(addr+2);
	  addr= a;
	  break;
	case 'r': // reljmp (2nd byte)
	  a= rom->validate_address(addr+2+(signed char)(rom->get(addr+1)));
	  analyze(a);
	  addr= addr+tabl->length;
	  break;
	case 'R': // reljmp (3rd byte)
	  analyze(rom->validate_address(addr+3+(signed char)(rom->get(addr+2))));
	  addr= addr+tabl->length;
	  break;
	case 's': // sjmp
	  {
	    signed char target;
	    target= rom->get(addr+1);
	    addr+= 2;
	    addr= rom->validate_address(addr+target);
	    break;
	  }
	case '_':
	  return;
	default:
	  addr= rom->validate_address(addr+tabl->length);
	  break;
	}
      code= rom->get(addr);
      tabl= &(dis_tbl()[code]);
    }
}


/*
 * Inform hardware elements that `cycles' machine cycles have elapsed
 */

/*int
cl_51core::tick_hw(int cycles)
{
  cl_uc::tick_hw(cycles);
  //do_hardware(cycles);
  return(0);
}*/

/*int
cl_51core::tick(int cycles)
{
  cl_uc::tick(cycles);
  //do_hardware(cycles);
  return(0);
}*/


/*
 * Correcting direct address
 *
 * This function returns address of addressed element which can be an IRAM
 * or an SFR.
 */

class cl_memory_cell *
cl_51core::get_direct(t_mem addr)
{
  if (addr < sfr->start_address)
    return(iram->get_cell(addr));
  else
    return(sfr->get_cell(addr));
}


/*
 * Fetching one instruction and executing it
 */


int
cl_51core::exec_inst(void)
{
  t_mem code;
  int res= resGO;

  if ((res= exec_inst_tab(itab51)) != resNOT_DONE)
    return res;

  instPC= PC;
  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  res= inst_unknown();
  return(res);
}


/*
 * Simulating execution of next instruction
 *
 * This is an endless loop if requested number of steps is negative.
 * In this case execution is stopped if an instruction results other
 * status than GO. Execution can be stopped if `cmd_in' is not NULL
 * and there is input available on that file. It is usefull if the
 * command console is on a terminal. If input is available then a
 * complete line is read and dropped out because input is buffered
 * (inp_avail will be TRUE if ENTER is pressed) and it can confuse
 * command interepter.
 */
//static class cl_console *c= NULL;
int
cl_51core::do_inst(int step)
{
  result= resGO;
  while ((result == resGO) &&
	 (state != stPD) &&
	 (step != 0))
    {
      if (step > 0)
	step--;
      if (state == stGO)
	{
	  interrupt->was_reti= false;
	  pre_inst();
	  result= exec_inst();
	  post_inst();
	}
      else
	{
	  // tick hw in idle state
	  inst_ticks= 1;
	  post_inst();
	  tick(1);
	}
      if (result == resGO)
	{
	  int res;
	  if ((res= do_interrupt()) != resGO)
	    result= res;
	  else
	    result= idle_pd();
	}
      if (((result == resINTERRUPT) &&
	   stop_at_it) ||
	  result >= resSTOP)
	{
	  sim->stop(result);
	  break;
	}
    }
  if (state == stPD)
    {
      //FIXME: tick outsiders eg. watchdog
    }
  return(result);
}

/*
 * Abstract method to handle WDT
 */

/*int
cl_51core::do_wdt(int cycles)
{
  return(resGO);
}*/


/*
 * Checking for interrupt requests and accept one if needed
 */

int
cl_51core::do_interrupt(void)
{
  int i, ie= 0;

  if (interrupt->was_reti)
    {
      interrupt->was_reti= false;
      return(resGO);
    }
  if (!((ie= sfr->get(IE)) & bmEA))
    return(resGO);
  class it_level *il= (class it_level *)(it_levels->top()), *IL= 0;
  for (i= 0; i < it_sources->count; i++)
    {
      class cl_it_src *is= (class cl_it_src *)(it_sources->at(i));
      if (is->is_active() &&
	  is->enabled() &&
	  is->pending())
	{
	  int pr= priority_of(is->ie_mask);
	  if (il->level >= 0 &&
	      pr <= il->level)
	    continue;
	  if (state == stIDLE)
	    {
	      state= stGO;
	      sfr->set_bit0(PCON, bmIDL);
	      interrupt->was_reti= true;
	      return(resGO);
	    }
	  is->clear();
	  sim->app->get_commander()->
	    debug("%g sec (%d clks): Accepting interrupt `%s' PC= 0x%06x\n",
			  get_rtime(), ticks->ticks, object_name(is), PC);
	  IL= new it_level(pr, is->addr, PC, is);
	  return(accept_it(IL));
	}
    }
  return(resGO);
}

int
cl_51core::priority_of(uchar nuof_it)
{
  if (sfr->get(IP) & /*ie_mask*/nuof_it)
    return(1);
  return(0);
}


/*
 * Accept an interrupt
 */

int
cl_51core::accept_it(class it_level *il)
{
  state= stGO;
  sfr->set_bit0(PCON, bmIDL);
  it_levels->push(il);
  tick(1);
  int res= inst_lcall(0, il->addr, true);
  if (res != resGO)
    return(res);
  else
    return(resINTERRUPT);
}


/* check if interrupts are enabled (globally)
 */

bool
cl_51core::it_enabled(void)
{
  return sfr->get(IE) & bmEA;
}


/* 
 * Check SP validity after stack (write) poeration
 */

void
cl_51core::stack_check_overflow(class cl_stack_op *op)
{
  t_addr b, a;
  b= op->get_before();
  a= op->get_after();
  if (a < b)
    {
      class cl_error_stack_overflow *e=
	new cl_error_stack_overflow(op);
      e->init();
      error(e);
    }
}


/*
 * Checking if Idle or PowerDown mode should be activated
 */

int
cl_51core::idle_pd(void)
{
  uint pcon= sfr->get(PCON);

  if (!(type->subtype & CPU_CMOS))
    return(resGO);
  if (pcon & bmIDL)
    {
      if (state != stIDLE)
	sim->app->get_commander()->
	  debug("%g sec (%d clks): CPU in Idle mode (PC=0x%x, PCON=0x%x)\n",
		get_rtime(), ticks->ticks, PC, pcon);
      state= stIDLE;
      //was_reti= 1;
    }
  if (pcon & bmPD)
    {
      if (state != stPD)
	sim->app->get_commander()->
	  debug("%g sec (%d clks): CPU in PowerDown mode\n",
			get_rtime(), ticks->ticks);
      state= stPD;
    }
  return(resGO);
}


/*
 * Simulating an unknown instruction
 *
 * Normally this function is called for unimplemented instructions, because
 * every instruction must be known!
 */

int
cl_51core::inst_unknown(void)
{
  //PC--;
  class cl_error_unknown_code *e= new cl_error_unknown_code(this);
  error(e);
  return(resGO);
}


/*
 * 0x00 1 12 NOP
 */

int
cl_51core::instruction_00/*inst_nop*/(t_mem/*uchar*/ code)
{
  return(resGO);
}


/*
 * 0xe4 1 12 CLR A
 */

int
cl_51core::instruction_e4/*inst_clr_a*/(t_mem/*uchar*/ code)
{
  acc->write(0);
  return(resGO);
}


/*
 * 0xc4 1 1 SWAP A
 */

int
cl_51core::instruction_c4/*inst_swap*/(t_mem/*uchar*/ code)
{
  uchar temp;

  temp= (acc->read() >> 4) & 0x0f;
  sfr->write(ACC, (acc->get() << 4) | temp);
  return(resGO);
}


/*
 */

cl_uc51_cpu::cl_uc51_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU/*DUMMY*/, 0, "cpu")
{
}

int
cl_uc51_cpu::init(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID),
    *bas= uc->address_space("bits");
  int i;
  cl_hw::init();
  if (!sfr)
    {
      fprintf(stderr, "No SFR to register %s[%d] into\n", id_string, id);
    }
  cell_psw= sfr->get_cell(PSW);//use_cell(sfr, PSW);
  cell_acc= register_cell(sfr, ACC);
  cell_sp= register_cell(sfr, SP);
  for (i= 0; i < 8; i++)
    acc_bits[i]= register_cell(bas, ACC+i);

  cl_var *v;
  uc->vars->add(v= new cl_var(cchars("cpu_aof_mdps"), cfg, uc51cpu_aof_mdps,
			      cfg_help(uc51cpu_aof_mdps)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("cpu_mask_mdps"), cfg, uc51cpu_mask_mdps,
			      cfg_help(uc51cpu_mask_mdps)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("cpu_aof_mdps1l"), cfg, uc51cpu_aof_mdps1l,
			      cfg_help(uc51cpu_aof_mdps1l)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("cpu_aof_mdps1h"), cfg, uc51cpu_aof_mdps1h,
			      cfg_help(uc51cpu_aof_mdps1h)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("cpu_aof_mdpc"), cfg, uc51cpu_aof_mdpc,
			      cfg_help(uc51cpu_aof_mdpc)));
  v->init();
  uc->vars->add(v= new cl_var(cchars("cpu_mask_mdpc"), cfg, uc51cpu_mask_mdpc,
			      cfg_help(uc51cpu_mask_mdpc)));
  v->init();
  
  return(0);
}

char *
cl_uc51_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case uc51cpu_aof_mdps:
      return (char*)"Address of multi_DPTR_sfr selector, WR selects this style of multi_DPTR (int, RW)";
    case uc51cpu_mask_mdps:
      return (char*)"Mask in multi_DPTR_srf selector (int, RW)";
    case uc51cpu_aof_mdps1l:
      return (char*)"Address of multi_DPTR_sfr DPL1 (int, RW)";
    case uc51cpu_aof_mdps1h:
      return (char*)"Address of multi_DPTR_sfr DPH1 (int, RW)";
    case uc51cpu_aof_mdpc:
      return (char*)"Address of multi_DPTR_chip selector, WR selects this stlye of multi_DPTR (int, RW)";
    case uc51cpu_mask_mdpc:
      return (char*)"Mask in multi_DPTR_chip selector (int, RW)";
    }
  return (char*)"Not used";
}

void
cl_uc51_cpu::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  if (cell == cell_sp)
    {
      if (*val > uc->sp_max)
	uc->sp_max= *val;
      uc->sp_avg= (uc->sp_avg+(*val))/2;
    }
  else 
    {
      bool p;
      int i;
      uchar uc, n= *val;

      if (cell != cell_acc)
	{
	  cell->set(*val);
	  n= cell_acc->get();
	}
      p = false;
      uc= n;
      for (i= 0; i < 8; i++)
	{
	  if (uc & 1)
	    p= !p;
	  uc>>= 1;
	}
      if (p)
	cell_psw->set_bit1(bmP);
      else
	cell_psw->set_bit0(bmP);
    }
  /*else if (cell == cell_pcon)
    {
      printf("PCON write 0x%x (PC=0x%x)\n", *val, uc->PC);
      uc->sim->stop(0);
      }*/
}

t_mem
cl_uc51_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  if (val)
    cell->set(*val);
  switch ((enum uc51cpu_cfg)addr)
    {
    case uc51cpu_aof_mdps: // addr of multi_DPTR_sfr selector
      if (val)
	((cl_51core *)uc)->decode_dptr();
      break;
    case uc51cpu_mask_mdps: // mask in mutli_DPTR_sfr selector
      break;
    case uc51cpu_aof_mdps1l: // addr of multi_DPTR_sfr DPL1
      break;
    case uc51cpu_aof_mdps1h: // addr of multi_DPTR_sfr DPH1
      break;

    case uc51cpu_aof_mdpc: // addr of multi_DPTR_chip selector
      if (val)
	((cl_51core *)uc)->decode_dptr();
      break;
    case uc51cpu_mask_mdpc: // mask in multi_DPTR_chip selector
      break;
  
    case uc51cpu_nuof:
      break;
    }
  return cell->get();
}

/* End of s51.src/uc51.cc */
