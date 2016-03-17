/*
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Jean-François DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : track_generator.c
// Contains: ISO/IBM/Amiga track builder/encoder
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "crc.h"
#include "track_generator.h"
#include "floppy_utils.h"
#include "math.h"


unsigned short MFM_tab[]=
{
	0xAAAA,0xAAA9,0xAAA4,0xAAA5,0xAA92,0xAA91,0xAA94,0xAA95,
	0xAA4A,0xAA49,0xAA44,0xAA45,0xAA52,0xAA51,0xAA54,0xAA55,
	0xA92A,0xA929,0xA924,0xA925,0xA912,0xA911,0xA914,0xA915,
	0xA94A,0xA949,0xA944,0xA945,0xA952,0xA951,0xA954,0xA955,
	0xA4AA,0xA4A9,0xA4A4,0xA4A5,0xA492,0xA491,0xA494,0xA495,
	0xA44A,0xA449,0xA444,0xA445,0xA452,0xA451,0xA454,0xA455,
	0xA52A,0xA529,0xA524,0xA525,0xA512,0xA511,0xA514,0xA515,
	0xA54A,0xA549,0xA544,0xA545,0xA552,0xA551,0xA554,0xA555,
	0x92AA,0x92A9,0x92A4,0x92A5,0x9292,0x9291,0x9294,0x9295,
	0x924A,0x9249,0x9244,0x9245,0x9252,0x9251,0x9254,0x9255,
	0x912A,0x9129,0x9124,0x9125,0x9112,0x9111,0x9114,0x9115,
	0x914A,0x9149,0x9144,0x9145,0x9152,0x9151,0x9154,0x9155,
	0x94AA,0x94A9,0x94A4,0x94A5,0x9492,0x9491,0x9494,0x9495,
	0x944A,0x9449,0x9444,0x9445,0x9452,0x9451,0x9454,0x9455,
	0x952A,0x9529,0x9524,0x9525,0x9512,0x9511,0x9514,0x9515,
	0x954A,0x9549,0x9544,0x9545,0x9552,0x9551,0x9554,0x9555,
	0x4AAA,0x4AA9,0x4AA4,0x4AA5,0x4A92,0x4A91,0x4A94,0x4A95,
	0x4A4A,0x4A49,0x4A44,0x4A45,0x4A52,0x4A51,0x4A54,0x4A55,
	0x492A,0x4929,0x4924,0x4925,0x4912,0x4911,0x4914,0x4915,
	0x494A,0x4949,0x4944,0x4945,0x4952,0x4951,0x4954,0x4955,
	0x44AA,0x44A9,0x44A4,0x44A5,0x4492,0x4491,0x4494,0x4495,
	0x444A,0x4449,0x4444,0x4445,0x4452,0x4451,0x4454,0x4455,
	0x452A,0x4529,0x4524,0x4525,0x4512,0x4511,0x4514,0x4515,
	0x454A,0x4549,0x4544,0x4545,0x4552,0x4551,0x4554,0x4555,
	0x52AA,0x52A9,0x52A4,0x52A5,0x5292,0x5291,0x5294,0x5295,
	0x524A,0x5249,0x5244,0x5245,0x5252,0x5251,0x5254,0x5255,
	0x512A,0x5129,0x5124,0x5125,0x5112,0x5111,0x5114,0x5115,
	0x514A,0x5149,0x5144,0x5145,0x5152,0x5151,0x5154,0x5155,
	0x54AA,0x54A9,0x54A4,0x54A5,0x5492,0x5491,0x5494,0x5495,
	0x544A,0x5449,0x5444,0x5445,0x5452,0x5451,0x5454,0x5455,
	0x552A,0x5529,0x5524,0x5525,0x5512,0x5511,0x5514,0x5515,
	0x554A,0x5549,0x5544,0x5545,0x5552,0x5551,0x5554,0x5555
};


unsigned short CLK_tab[]=
{
	0x5555,0x5557,0x555D,0x555F,0x5575,0x5577,0x557D,0x557F,
	0x55D5,0x55D7,0x55DD,0x55DF,0x55F5,0x55F7,0x55FD,0x55FF,
	0x5755,0x5757,0x575D,0x575F,0x5775,0x5777,0x577D,0x577F,
	0x57D5,0x57D7,0x57DD,0x57DF,0x57F5,0x57F7,0x57FD,0x57FF,
	0x5D55,0x5D57,0x5D5D,0x5D5F,0x5D75,0x5D77,0x5D7D,0x5D7F,
	0x5DD5,0x5DD7,0x5DDD,0x5DDF,0x5DF5,0x5DF7,0x5DFD,0x5DFF,
	0x5F55,0x5F57,0x5F5D,0x5F5F,0x5F75,0x5F77,0x5F7D,0x5F7F,
	0x5FD5,0x5FD7,0x5FDD,0x5FDF,0x5FF5,0x5FF7,0x5FFD,0x5FFF,
	0x7555,0x7557,0x755D,0x755F,0x7575,0x7577,0x757D,0x757F,
	0x75D5,0x75D7,0x75DD,0x75DF,0x75F5,0x75F7,0x75FD,0x75FF,
	0x7755,0x7757,0x775D,0x775F,0x7775,0x7777,0x777D,0x777F,
	0x77D5,0x77D7,0x77DD,0x77DF,0x77F5,0x77F7,0x77FD,0x77FF,
	0x7D55,0x7D57,0x7D5D,0x7D5F,0x7D75,0x7D77,0x7D7D,0x7D7F,
	0x7DD5,0x7DD7,0x7DDD,0x7DDF,0x7DF5,0x7DF7,0x7DFD,0x7DFF,
	0x7F55,0x7F57,0x7F5D,0x7F5F,0x7F75,0x7F77,0x7F7D,0x7F7F,
	0x7FD5,0x7FD7,0x7FDD,0x7FDF,0x7FF5,0x7FF7,0x7FFD,0x7FFF,
	0xD555,0xD557,0xD55D,0xD55F,0xD575,0xD577,0xD57D,0xD57F,
	0xD5D5,0xD5D7,0xD5DD,0xD5DF,0xD5F5,0xD5F7,0xD5FD,0xD5FF,
	0xD755,0xD757,0xD75D,0xD75F,0xD775,0xD777,0xD77D,0xD77F,
	0xD7D5,0xD7D7,0xD7DD,0xD7DF,0xD7F5,0xD7F7,0xD7FD,0xD7FF,
	0xDD55,0xDD57,0xDD5D,0xDD5F,0xDD75,0xDD77,0xDD7D,0xDD7F,
	0xDDD5,0xDDD7,0xDDDD,0xDDDF,0xDDF5,0xDDF7,0xDDFD,0xDDFF,
	0xDF55,0xDF57,0xDF5D,0xDF5F,0xDF75,0xDF77,0xDF7D,0xDF7F,
	0xDFD5,0xDFD7,0xDFDD,0xDFDF,0xDFF5,0xDFF7,0xDFFD,0xDFFF,
	0xF555,0xF557,0xF55D,0xF55F,0xF575,0xF577,0xF57D,0xF57F,
	0xF5D5,0xF5D7,0xF5DD,0xF5DF,0xF5F5,0xF5F7,0xF5FD,0xF5FF,
	0xF755,0xF757,0xF75D,0xF75F,0xF775,0xF777,0xF77D,0xF77F,
	0xF7D5,0xF7D7,0xF7DD,0xF7DF,0xF7F5,0xF7F7,0xF7FD,0xF7FF,
	0xFD55,0xFD57,0xFD5D,0xFD5F,0xFD75,0xFD77,0xFD7D,0xFD7F,
	0xFDD5,0xFDD7,0xFDDD,0xFDDF,0xFDF5,0xFDF7,0xFDFD,0xFDFF,
	0xFF55,0xFF57,0xFF5D,0xFF5F,0xFF75,0xFF77,0xFF7D,0xFF7F,
	0xFFD5,0xFFD7,0xFFDD,0xFFDF,0xFFF5,0xFFF7,0xFFFD,0xFFFF
};

unsigned char even_tab[]=
{
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x00,0x01,0x00,0x01,0x02,0x03,0x02,0x03,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x04,0x05,0x04,0x05,0x06,0x07,0x06,0x07,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x08,0x09,0x08,0x09,0x0A,0x0B,0x0A,0x0B,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
	0x0C,0x0D,0x0C,0x0D,0x0E,0x0F,0x0E,0x0F,
};

unsigned char odd_tab[]=
{
	0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x01,
	0x02,0x02,0x03,0x03,0x02,0x02,0x03,0x03,
	0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x01,
	0x02,0x02,0x03,0x03,0x02,0x02,0x03,0x03,
	0x04,0x04,0x05,0x05,0x04,0x04,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x07,0x07,
	0x04,0x04,0x05,0x05,0x04,0x04,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x07,0x07,
	0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x01,
	0x02,0x02,0x03,0x03,0x02,0x02,0x03,0x03,
	0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x01,
	0x02,0x02,0x03,0x03,0x02,0x02,0x03,0x03,
	0x04,0x04,0x05,0x05,0x04,0x04,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x07,0x07,
	0x04,0x04,0x05,0x05,0x04,0x04,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x07,0x07,
	0x08,0x08,0x09,0x09,0x08,0x08,0x09,0x09,
	0x0A,0x0A,0x0B,0x0B,0x0A,0x0A,0x0B,0x0B,
	0x08,0x08,0x09,0x09,0x08,0x08,0x09,0x09,
	0x0A,0x0A,0x0B,0x0B,0x0A,0x0A,0x0B,0x0B,
	0x0C,0x0C,0x0D,0x0D,0x0C,0x0C,0x0D,0x0D,
	0x0E,0x0E,0x0F,0x0F,0x0E,0x0E,0x0F,0x0F,
	0x0C,0x0C,0x0D,0x0D,0x0C,0x0C,0x0D,0x0D,
	0x0E,0x0E,0x0F,0x0F,0x0E,0x0E,0x0F,0x0F,
	0x08,0x08,0x09,0x09,0x08,0x08,0x09,0x09,
	0x0A,0x0A,0x0B,0x0B,0x0A,0x0A,0x0B,0x0B,
	0x08,0x08,0x09,0x09,0x08,0x08,0x09,0x09,
	0x0A,0x0A,0x0B,0x0B,0x0A,0x0A,0x0B,0x0B,
	0x0C,0x0C,0x0D,0x0D,0x0C,0x0C,0x0D,0x0D,
	0x0E,0x0E,0x0F,0x0F,0x0E,0x0E,0x0F,0x0F,
	0x0C,0x0C,0x0D,0x0D,0x0C,0x0C,0x0D,0x0D,
	0x0E,0x0E,0x0F,0x0F,0x0E,0x0E,0x0F,0x0F
};

typedef struct gap3conf_
{
	unsigned char  trackmode;
	unsigned short sectorsize;
	unsigned char numberofsector;
	unsigned char gap3;
}gap3conf;



static gap3conf std_gap3_tab[]=
{ 
	{IBMFORMAT_DD, 256 ,0x12,0x0C},
	{IBMFORMAT_DD, 256 ,0x10,0x32},
	{IBMFORMAT_DD, 512 ,0x08,0x50},
	{IBMFORMAT_DD, 512 ,0x09,0x50},
	{IBMFORMAT_DD, 1024,0x04,0xF0},
	{IBMFORMAT_DD, 2048,0x02,0xF0},
	{IBMFORMAT_DD, 4096,0x01,0xF0},
	{IBMFORMAT_DD, 256 ,0x1A,0x36},
	{IBMFORMAT_DD, 512 ,0x0F,0x54},
	{IBMFORMAT_DD, 512 ,0x12,0x6C},
	{IBMFORMAT_DD, 1024,0x08,0x74},
	{IBMFORMAT_DD, 2048,0x04,0xF0},
	{IBMFORMAT_DD, 4096,0x02,0xF0},
	{IBMFORMAT_DD, 8192,0x01,0xF0},
	{IBMFORMAT_DD, 512 ,0x24,0x53},

	//8"FM
	{IBMFORMAT_SD, 128 ,0x1A,0x1B},
	{IBMFORMAT_SD, 256 ,0x0F,0x2A},
	{IBMFORMAT_SD, 512 ,0x08,0x3A},
	{IBMFORMAT_SD,1024 ,0x04,0x8A},
	{IBMFORMAT_SD,2048 ,0x02,0xF8},
	{IBMFORMAT_SD,4096 ,0x01,0xF8},

	//8"MFM
	{IBMFORMAT_DD, 256 ,0x1A,0x36},
	{IBMFORMAT_DD, 512 ,0x0F,0x54},
	{IBMFORMAT_DD,1024 ,0x08,0x74},
	{IBMFORMAT_DD,2048 ,0x04,0xF8},
	{IBMFORMAT_DD,4096 ,0x02,0xF8},
	{IBMFORMAT_DD,8192 ,0x01,0xF8},

	//5"FM
	{IBMFORMAT_SD, 128 ,0x12,0x09},
	{IBMFORMAT_SD, 128 ,0x10,0x19},
	{IBMFORMAT_SD, 256 ,0x08,0x30},
	{IBMFORMAT_SD, 512 ,0x04,0x87},
	{IBMFORMAT_SD,1024 ,0x02,0xF8},
	{IBMFORMAT_SD,2048 ,0x01,0xF8},

	//5"MFM
	{IBMFORMAT_DD, 256 ,0x12,0x0C},
	{IBMFORMAT_DD, 256 ,0x10,0x32},
	{IBMFORMAT_DD, 512 ,0x08,0x50},
	{IBMFORMAT_DD, 512 ,0x09,0x40},
	{IBMFORMAT_DD, 512 ,0x0A,0x10},
	{IBMFORMAT_DD,1024 ,0x04,0xF0},
	{IBMFORMAT_DD,2048 ,0x02,0xF8},
	{IBMFORMAT_DD,4096 ,0x01,0xF8},
///////

	{ISOFORMAT_DD, 256 ,0x12,0x0C},
	{ISOFORMAT_DD, 256 ,0x10,0x32},
	{ISOFORMAT_DD, 512 ,0x08,0x50},
	{ISOFORMAT_DD, 512 ,0x09,0x50},
	{ISOFORMAT_DD, 1024,0x04,0xF0},
	{ISOFORMAT_DD, 2048,0x02,0xF0},
	{ISOFORMAT_DD, 4096,0x01,0xF0},
	{ISOFORMAT_DD, 256 ,0x1A,0x36},
	{ISOFORMAT_DD, 512 ,0x0F,0x54},
	{ISOFORMAT_DD, 512 ,0x12,0x6C},
	{ISOFORMAT_DD, 1024,0x08,0x74},
	{ISOFORMAT_DD, 2048,0x04,0xF0},
	{ISOFORMAT_DD, 4096,0x02,0xF0},
	{ISOFORMAT_DD, 8192,0x01,0xF0},
	{ISOFORMAT_DD, 512, 0x24,0x53},

	//8"FM
	{ISOFORMAT_SD, 128 ,0x1A,0x1B},
	{ISOFORMAT_SD, 256 ,0x0F,0x2A},
	{ISOFORMAT_SD, 512 ,0x08,0x3A},
	{ISOFORMAT_SD,1024 ,0x04,0x8A},
	{ISOFORMAT_SD,2048 ,0x02,0xF8},
	{ISOFORMAT_SD,4096 ,0x01,0xF8},

	//8"MFM
	{ISOFORMAT_DD, 256 ,0x1A,0x36},
	{ISOFORMAT_DD, 512 ,0x0F,0x54},
	{ISOFORMAT_DD,1024 ,0x08,0x74},
	{ISOFORMAT_DD,2048 ,0x04,0xF8},
	{ISOFORMAT_DD,4096 ,0x02,0xF8},
	{ISOFORMAT_DD,8192 ,0x01,0xF8},

	//5"FM
	{ISOFORMAT_SD, 128 ,0x12,0x09},
	{ISOFORMAT_SD, 128 ,0x10,0x19},
	{ISOFORMAT_SD, 256 ,0x08,0x30},
	{ISOFORMAT_SD, 512 ,0x04,0x87},
	{ISOFORMAT_SD,1024 ,0x02,0xF8},
	{ISOFORMAT_SD,2048 ,0x01,0xF8},

	//5"MFM
	{ISOFORMAT_DD, 256 ,0x12,0x0C},
	{ISOFORMAT_DD, 256 ,0x10,0x32},
	{ISOFORMAT_DD, 512 ,0x08,0x50},
	{ISOFORMAT_DD, 512 ,0x09,0x40},
	{ISOFORMAT_DD, 512 ,0x0A,0x10},
	{ISOFORMAT_DD,1024 ,0x04,0xF0},
	{ISOFORMAT_DD,2048 ,0x02,0xF8},
	{ISOFORMAT_DD,4096 ,0x01,0xF8},

	{0xFF,0xFFFF,0xFF,0xFF}
};

void getMFMcode(track_generator *tg,unsigned char data,unsigned char clock,unsigned char * dstbuf)
{
	unsigned short mfm_code;
	mfm_code = MFM_tab[data] & CLK_tab[clock] & tg->mfm_last_bit;
	tg->mfm_last_bit=~(MFM_tab[data]<<15);		
	dstbuf[1]=mfm_code&0xFF;
	dstbuf[0]=mfm_code>>8;

	return;
}

void getFMcode(track_generator *tg,unsigned char data,unsigned char clock,unsigned char * dstbuf)
{
	unsigned long * fm_code;
	unsigned char k,i;
			
	fm_code=(unsigned long *)dstbuf;

	*fm_code=0;
	for(k=0;k<4;k++)
	{
		*fm_code=*fm_code>>8;

		////////////////////////////////////
		// data
		for(i=0;i<2;i++)
		{
			if(data&(0x80>>(i+(k*2)) ))
			{	// 0x10 
				// 00010001)
				*fm_code=*fm_code | ((0x10>>(i*4))<<24);
			}
		}

		// clock
		for(i=0;i<2;i++)
		{
			if(clock&(0x80>>(i+(k*2)) ))
			{	// 0x40 
				// 01000100)
				*fm_code=*fm_code | ((0x40>>(i*4))<<24);
			}
		}
	}
	
	return;
}


int pushTrackCode(track_generator *tg,unsigned char data,unsigned char clock,SIDE * side,unsigned char trackencoding)
{
	
	switch(trackencoding)
	{
		case IBMFORMAT_SD:
		case ISOFORMAT_SD:
			getFMcode(tg,data,clock,&side->databuffer[tg->last_bit_offset/8]);
			tg->last_bit_offset=tg->last_bit_offset+(4*8);
		break;

		case IBMFORMAT_DD:
		case ISOFORMAT_DD:
		case ISOFORMAT_DD11S:
		case AMIGAFORMAT_DD:
			getMFMcode(tg,data,clock,&side->databuffer[tg->last_bit_offset/8]);
			tg->last_bit_offset=tg->last_bit_offset+(2*8);
		break;

		default:
		break;
	}
	return 0;
}


// Fast Bin to MFM converter
int BuildCylinder(unsigned char * mfm_buffer,int mfm_size,unsigned char * track_clk,unsigned char * track_data,int track_size)
{
	int i,l;
	unsigned char byte,clock;
	unsigned short lastbit;
	unsigned short mfm_code;
	
	if(track_size*2>mfm_size)
	{
		track_size=mfm_size/2;
	}

	// MFM Encoding
	lastbit=0xFFFF;
	i=0;
	for(l=0;l<track_size;l++)
	{
		byte =track_data[l];
		clock=track_clk[l];

		mfm_code = MFM_tab[byte] & CLK_tab[clock] & lastbit;

		mfm_buffer[i++]=mfm_code>>8;
		mfm_buffer[i++]=mfm_code&0xFF;

		lastbit=~(MFM_tab[byte]<<15);		
	}

	// Clear the remaining buffer bytes
	while(i<mfm_size)
	{
		mfm_buffer[i++]=0x00;
	}

	return track_size;
}


// Fast Bin to MFM converter
void FastMFMgenerator(track_generator *tg,SIDE * side,unsigned char * track_data,int size)
{
	unsigned short i,l;
	unsigned char  byte;
	unsigned short lastbit;
	unsigned short mfm_code;
	unsigned char * mfm_buffer;

	mfm_buffer=&side->databuffer[tg->last_bit_offset/8];

	// MFM Encoding
	lastbit=tg->mfm_last_bit;
	i=0;
	for(l=0;l<size;l++)
	{
		byte =track_data[l];
		mfm_code = MFM_tab[byte] & lastbit;

		mfm_buffer[i++]=mfm_code>>8;
		mfm_buffer[i++]=mfm_code&0xFF;

		lastbit=~(MFM_tab[byte]<<15);		
	}

	tg->mfm_last_bit=lastbit;
	tg->last_bit_offset=tg->last_bit_offset+(i*8);

	return;
}

// Fast Amiga Bin to MFM converter
void FastAmigaMFMgenerator(track_generator *tg,SIDE * side,unsigned char * track_data,int size)
{
	unsigned short i,l;
	unsigned char  byte;
	unsigned short lastbit;
	unsigned short mfm_code;
	unsigned char * mfm_buffer;

	mfm_buffer=&side->databuffer[tg->last_bit_offset/8];

	// MFM Encoding
	lastbit=tg->mfm_last_bit;
	i=0;

	for(l=0;l<size;l=l+2)
	{
		byte =(odd_tab[track_data[l]]<<4) | odd_tab[track_data[l+1]];
		mfm_code = MFM_tab[byte] & lastbit;

		mfm_buffer[i++]=mfm_code>>8;
		mfm_buffer[i++]=mfm_code&0xFF;

		lastbit=~(MFM_tab[byte]<<15);		
	}


	for(l=0;l<size;l=l+2)
	{
		byte =(even_tab[track_data[l]]<<4) | even_tab[track_data[l+1]];
		mfm_code = MFM_tab[byte] & lastbit;

		mfm_buffer[i++]=mfm_code>>8;
		mfm_buffer[i++]=mfm_code&0xFF;

		lastbit=~(MFM_tab[byte]<<15);		
	}

	tg->mfm_last_bit=lastbit;
	tg->last_bit_offset=tg->last_bit_offset+(i*8);

	return;
}


// Fast Bin to FM converter
void FastFMgenerator(track_generator *tg,SIDE * side,unsigned char * track_data,int size)
{
	unsigned short j,l;
	unsigned char  i,k;
	unsigned char  byte;
	unsigned char * fm_buffer;

	fm_buffer=&side->databuffer[tg->last_bit_offset/8];

	j=0;
	for(l=0;l<size;l++)
	{
		byte=track_data[l];
			
		for(k=0;k<4;k++)
		{
			fm_buffer[j]=0x44;
			////////////////////////////////////
			// data
			for(i=0;i<2;i++)
			{
				if(byte&(0x80>>(i+(k*2)) ))
				{	// 0x10 
					// 00010001)
					fm_buffer[j]=fm_buffer[j] | (0x10>>(i*4));
				}
			}

			j++;
		}
	}
	
	tg->last_bit_offset=tg->last_bit_offset+(j*8);
	
	return;
}



void FastMFMFMgenerator(track_generator *tg,SIDE * side,unsigned char * track_data,int size,unsigned char trackencoding)
{
	
	switch(trackencoding)
	{
		case IBMFORMAT_SD:
		case ISOFORMAT_SD:
			FastFMgenerator(tg,side,track_data,size);
		break;

		case IBMFORMAT_DD:
		case ISOFORMAT_DD:
		case ISOFORMAT_DD11S:
			FastMFMgenerator(tg,side,track_data,size);
		break;

		case AMIGAFORMAT_DD:
			FastAmigaMFMgenerator(tg,side,track_data,size);
		break;

		default:
		break;
	}
	return;
}


// FM encoder
void BuildFMCylinder(char * buffer,int fmtracksize,char * bufferclk,char * track,int size)
{
	int i,j,k,l;
	unsigned char byte,clock;
	
	// Clean up
	for(i=0;i<(fmtracksize);i++)
	{
		buffer[i]=0x00;
	}

	j=0;

	// FM Encoding
	j=0;
	for(l=0;l<size;l++)
	{
		byte=track[l];
		clock=bufferclk[l];
			
		for(k=0;k<4;k++)
		{
			buffer[j]=0x00;
			////////////////////////////////////
			// data
			for(i=0;i<2;i++)
			{
				if(byte&(0x80>>(i+(k*2)) ))
				{	// 0x10 
					// 00010001)
					buffer[j]=buffer[j] | (0x10>>(i*4));
				}
			}

			// clock
			for(i=0;i<2;i++)
			{
				if(clock&(0x80>>(i+(k*2)) ))
				{	// 0x40 
					// 01000100)
					buffer[j]=buffer[j] | (0x40>>(i*4));
				}
			}

			j++;
		}
	}
}

int ISOIBMGetTrackSize(int TRACKTYPE,unsigned int numberofsector,unsigned int sectorsize,unsigned int gap3len,SECTORCONFIG * sectorconfigtab)
{
	unsigned int i,j;
	isoibm_config * configptr;
	unsigned long finalsize;
	unsigned long totaldatasize;


	configptr=NULL;
	i=0;
	while (formatstab[i].indexformat!=TRACKTYPE &&  formatstab[i].indexformat!=0)
	{
		i++;
	};

	configptr=&formatstab[i];
	totaldatasize=0;
	if(sectorconfigtab)
	{
		sectorsize=0;
		for(j=0;j<numberofsector;j++)
		{
			totaldatasize=totaldatasize+(sectorconfigtab[j].sectorsize);
		}
	}

	finalsize=configptr->len_gap4a+configptr->len_isync+configptr->len_indexmarkp1+configptr->len_indexmarkp2 + \
			  configptr->len_gap1 +  \
			  numberofsector*(configptr->len_ssync+configptr->len_addrmarkp1+configptr->len_addrmarkp2 +6 +configptr->len_gap2 +configptr->len_dsync+configptr->len_datamarkp1+configptr->len_datamarkp2+sectorsize+2+gap3len) +\
			  totaldatasize;

	return finalsize;

}

unsigned char* compute_interleave_tab(unsigned char interleave,unsigned char skew,unsigned short numberofsector)
{

	int i,j;
	unsigned char *interleave_tab;
	unsigned char *allocated_tab;

	interleave_tab=(unsigned char *)malloc(numberofsector*sizeof(unsigned char));
	if(numberofsector)
	{
		if(interleave_tab)
		{
			allocated_tab=(unsigned char *)malloc(numberofsector*sizeof(unsigned char));
			memset(interleave_tab,0,numberofsector*sizeof(unsigned char));
			memset(allocated_tab,0,numberofsector*sizeof(unsigned char));

			j=skew%(numberofsector);
			i=0;
			do
			{
				while(allocated_tab[j])
				{
					j=(j+1)%(numberofsector);
				}
				interleave_tab[j]=i;
				allocated_tab[j]=0xFF;
				j=(j+interleave)%(numberofsector);
				i++;
			}while(i<numberofsector);		

			free(allocated_tab);
		}
	}
	
	return interleave_tab;
}


int searchgap3value(unsigned int numberofsector,SECTORCONFIG * sectorconfigtab)
{
	unsigned int i,j;
	unsigned char gap3;

	gap3=0xFF;
	j=0;
	do
	{
		i=0;
		while( (i<numberofsector) && ( (std_gap3_tab[j].trackmode==sectorconfigtab[i].trackencoding) && (std_gap3_tab[j].sectorsize==sectorconfigtab[i].sectorsize) && (std_gap3_tab[j].numberofsector==numberofsector)) )
		{
			i++;
		}

		if(i==numberofsector)
		{
			gap3=std_gap3_tab[j].gap3;
			return gap3;
		}
		j++;
	}while((i<numberofsector) && (std_gap3_tab[j].trackmode!=0xFF) && gap3==0xFF);

	return -1;
}
void tg_initTrackEncoder(track_generator *tg)
{
	memset(tg,0,sizeof(track_generator));
	tg->mfm_last_bit=0xFFFF;
}

unsigned long tg_computeMinTrackSize(track_generator *tg,unsigned char trackencoding,unsigned int bitrate,unsigned int numberofsector,SECTORCONFIG * sectorconfigtab,unsigned long * track_period)
{
	unsigned int i,j;
	unsigned long tck_period;
	isoibm_config * configptr;
	unsigned long total_track_size,sector_size;
	unsigned char gap3;

	configptr=0;
	tck_period=0;
	i=0;

	configptr=&formatstab[trackencoding-1];

	total_track_size=configptr->len_gap4a+configptr->len_isync+configptr->len_indexmarkp1+configptr->len_indexmarkp2 + \
					 configptr->len_gap1;

	switch(trackencoding)
	{
		case IBMFORMAT_SD:
		case ISOFORMAT_SD:
			total_track_size=total_track_size*4;
			break;

		case IBMFORMAT_DD:
		case ISOFORMAT_DD:
		case ISOFORMAT_DD11S:
			total_track_size=total_track_size*2;
			break;

		default:
			total_track_size=total_track_size*2;
			break;
	}
	
	if(total_track_size)
 		tck_period=tck_period+(100000/(bitrate/(total_track_size*4)));


	for(j=0;j<numberofsector;j++)
	{
		// if gap3 is set to "to be computed" we consider it as zero for the moment...
		if(sectorconfigtab[j].gap3==255)
		{
			gap3=0;
		}
		else
		{
			gap3=sectorconfigtab[j].gap3;
		}
		configptr=&formatstab[sectorconfigtab[j].trackencoding-1];
		sector_size=(configptr->len_ssync+configptr->len_addrmarkp1+configptr->len_addrmarkp2 +6 +configptr->len_gap2 +configptr->len_dsync+configptr->len_datamarkp1+configptr->len_datamarkp2+2+gap3);
		sector_size=sector_size+sectorconfigtab[j].sectorsize+2;

		switch(sectorconfigtab[j].trackencoding)
		{
			case IBMFORMAT_SD:
			case ISOFORMAT_SD:
				sector_size=sector_size*4;
				break;

			case IBMFORMAT_DD:
			case ISOFORMAT_DD:
			case ISOFORMAT_DD11S:
				sector_size=sector_size*2;
				break;

			default:
				sector_size=sector_size*2;
				break;
		}

		total_track_size=total_track_size+sector_size;

		tck_period=tck_period+(10000000/((sectorconfigtab[0].bitrate*100)/(sector_size*4)));

	}
	
	if(track_period)
		*track_period=tck_period;

	total_track_size=total_track_size*8;

	return total_track_size;
}

SIDE * tg_initTrack(track_generator *tg,unsigned long tracksize,unsigned short numberofsector,unsigned char trackencoding,unsigned int bitrate,SECTORCONFIG * sectorconfigtab)
{
	SIDE * currentside;
	int variable_param,tracklen;
	unsigned int i;
	unsigned long   startindex;

	startindex=tg->last_bit_offset/8;

	currentside=(SIDE*)malloc(sizeof(SIDE));
	memset(currentside,0,sizeof(SIDE));
				
	currentside->number_of_sector=numberofsector;

	tracklen=tracksize/8;
	if(tracksize&7) tracklen++;

	currentside->tracklen=tracksize;

	if(numberofsector)
	{
		//////////////////////////////
		// bitrate buffer allocation
		variable_param=0;
		currentside->bitrate=sectorconfigtab[0].bitrate;
		i=0;
		while(i<currentside->number_of_sector && !variable_param)
		{
			if(sectorconfigtab[i].bitrate!=sectorconfigtab[0].bitrate)
			{
				variable_param=1;
				currentside->bitrate=VARIABLEBITRATE;

				currentside->timingbuffer=malloc(tracklen*sizeof(unsigned long));
				memset(currentside->timingbuffer,0,tracklen*sizeof(unsigned long));
			}
			i++;
		}
		///////////////////////////////////////////
		// track encoding code buffer allocation
		variable_param=0;
		currentside->track_encoding=sectorconfigtab[0].trackencoding;
		i=0;
		while(i<currentside->number_of_sector && !variable_param)
		{
			if(sectorconfigtab[i].trackencoding!=sectorconfigtab[0].trackencoding)
			{
				variable_param=1;
				currentside->track_encoding=VARIABLEENCODING;

				currentside->track_encoding_buffer=malloc(tracklen*sizeof(unsigned char));
				memset(currentside->track_encoding_buffer,0,tracklen*sizeof(unsigned char));
			}
			i++;
		}
	}
	else
	{
		currentside->bitrate=bitrate;
		currentside->track_encoding=trackencoding;
	}

	/////////////////////////////
	// data buffer allocation
	currentside->databuffer=malloc(tracklen);
	memset(currentside->databuffer,0,tracklen);
					
	currentside->flakybitsbuffer=0;
					
	/////////////////////////////
	// index buffer allocation
	currentside->indexbuffer=malloc(tracklen);
	memset(currentside->indexbuffer,0,tracklen);
	
	if(numberofsector)
	{
		//gap4a (post index gap4)
		for(i=0;i<formatstab[trackencoding-1].len_gap4a;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding-1].data_gap4a,0xFF,currentside,trackencoding);
		}

		//i sync
		for(i=0;i<formatstab[trackencoding-1].len_isync;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding-1].data_isync,0xFF,currentside,trackencoding);
		}

		// index mark
		for(i=0;i<formatstab[trackencoding-1].len_indexmarkp1;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding-1].data_indexmarkp1,formatstab[trackencoding-1].clock_indexmarkp1,currentside,trackencoding);
		}
		
		for(i=0;i<formatstab[trackencoding-1].len_indexmarkp2;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding-1].data_indexmarkp2,formatstab[trackencoding-1].clock_indexmarkp2,currentside,trackencoding);
		}

		// gap1
		for(i=0;i<formatstab[trackencoding-1].len_gap1;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding-1].data_gap1,0xFF,currentside,trackencoding);
		}
	}

	currentside->tracklen=tracksize;

	switch(trackencoding)
	{
		case IBMFORMAT_SD:
		case ISOFORMAT_SD:
			currentside->track_encoding=ISOIBM_FM_ENCODING;
		break;

		case ISOFORMAT_DD11S:
		case IBMFORMAT_DD:
		case ISOFORMAT_DD:
			currentside->track_encoding=ISOIBM_MFM_ENCODING;
		break;
		
		case AMIGAFORMAT_DD:
			currentside->track_encoding=AMIGA_MFM_ENCODING;
		break;

		default:
			currentside->track_encoding=ISOIBM_MFM_ENCODING;
		break;
	}

	// fill timing & encoding buffer
	if(currentside->timingbuffer)
	{
		for(i=startindex;i<(tg->last_bit_offset/8);i++)
		{
			currentside->timingbuffer[i]=sectorconfigtab[0].bitrate;
		}
	}

	if(currentside->track_encoding_buffer)
	{
		for(i=startindex;i<(tg->last_bit_offset/8);i++)
		{
			currentside->track_encoding_buffer[i]=currentside->track_encoding;
		}
	}

	return currentside;
}

void tg_addISOSectorToTrack(track_generator *tg,SECTORCONFIG * sectorconfig,SIDE * currentside)
{

	unsigned short  i;
	unsigned char   c,trackencoding,trackenc;
	unsigned char   CRC16_High;
	unsigned char   CRC16_Low;
	unsigned char   crctable[32];
	unsigned long   startindex,j;

	startindex=tg->last_bit_offset/8;
	
	sectorconfig->startsectorindex=tg->last_bit_offset/8;
	trackencoding=sectorconfig->trackencoding-1;
			
	// sync
	for(i=0;i<formatstab[trackencoding].len_ssync;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_ssync,0xFF,currentside,sectorconfig->trackencoding);
	}

	CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)&crctable,0x1021,0xFFFF);
	// add mark
	for(i=0;i<formatstab[trackencoding].len_addrmarkp1;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_addrmarkp1,formatstab[trackencoding].clock_addrmarkp1,currentside,sectorconfig->trackencoding);
		CRC16_Update(&CRC16_High,&CRC16_Low, formatstab[trackencoding].data_addrmarkp1,(unsigned char*)&crctable);
	}

	if(sectorconfig->use_alternate_addressmark)
	{
		for(i=0;i<formatstab[trackencoding].len_addrmarkp2;i++)
		{
			pushTrackCode(tg,sectorconfig->alternate_addressmark,formatstab[trackencoding].clock_addrmarkp2,currentside,sectorconfig->trackencoding);
			CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->alternate_addressmark,(unsigned char*)&crctable );
		}
	}
	else
	{
		for(i=0;i<formatstab[trackencoding].len_addrmarkp2;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding].data_addrmarkp2,formatstab[trackencoding].clock_addrmarkp2,currentside,sectorconfig->trackencoding);
			CRC16_Update(&CRC16_High,&CRC16_Low, formatstab[trackencoding].data_addrmarkp2,(unsigned char*)&crctable );
		}
	}

	// track number
	pushTrackCode(tg,sectorconfig->cylinder,0xFF,currentside,sectorconfig->trackencoding);
	CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->cylinder,(unsigned char*)&crctable );

	//01 Side # The side number this is (0 or 1) 
	pushTrackCode(tg,sectorconfig->head,  0xFF,currentside,sectorconfig->trackencoding);
	CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->head,(unsigned char*)&crctable );
			
	//01 Sector # The sector number 
	pushTrackCode(tg,sectorconfig->sector,0xFF,currentside,sectorconfig->trackencoding);
	CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->sector,(unsigned char*)&crctable );
			
	//01 Sector size: 02=512. (00=128, 01=256, 02=512, 03=1024) 
	if(sectorconfig->use_alternate_sector_size_id)
	{
		c=sectorconfig->alternate_sector_size_id;
	}
	else
	{	
		c=0;
		while(((unsigned int)(128<<(unsigned int)c)!=sectorconfig->sectorsize) && c<8)
		{
			c++;
		}
	}
	pushTrackCode(tg,c,0xFF,currentside,sectorconfig->trackencoding);
	CRC16_Update(&CRC16_High,&CRC16_Low, c,(unsigned char*)&crctable );

	//02 CRC The sector Header CRC
	if(sectorconfig->use_alternate_header_crc&0x2)
	{
		pushTrackCode(tg, (unsigned char) (sectorconfig->header_crc&0xFF),    0xFF,currentside,sectorconfig->trackencoding);
		pushTrackCode(tg, (unsigned char)((sectorconfig->header_crc>>8)&0xFF),0xFF,currentside,sectorconfig->trackencoding);
	}
	else
	{
		//bad crc
		if(sectorconfig->use_alternate_header_crc&0x1)
		{
			pushTrackCode(tg,(unsigned char)(CRC16_High^0x13),0xFF,currentside,sectorconfig->trackencoding);
			pushTrackCode(tg,(unsigned char)(CRC16_Low ^0x17),0xFF,currentside,sectorconfig->trackencoding);
		}
		else
		{
			pushTrackCode(tg,CRC16_High,0xFF,currentside,sectorconfig->trackencoding);
			pushTrackCode(tg,CRC16_Low,0xFF,currentside,sectorconfig->trackencoding);
		}

	}
	
	// gap2
	for(i=0;i<formatstab[trackencoding].len_gap2;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_gap2,0xFF,currentside,sectorconfig->trackencoding);
	}

	// sync
	for(i=0;i<formatstab[trackencoding].len_dsync;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_dsync,0xFF,currentside,sectorconfig->trackencoding);
	}
			
	//02 CRC The CRC of the data 
	CRC16_Init(&CRC16_High,&CRC16_Low,(unsigned char*)&crctable,0x1021,0xFFFF);

	// data mark
	for(i=0;i<formatstab[trackencoding].len_datamarkp1;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_datamarkp1,formatstab[trackencoding].clock_datamarkp1,currentside,sectorconfig->trackencoding);
		CRC16_Update(&CRC16_High,&CRC16_Low, formatstab[trackencoding].data_datamarkp1,(unsigned char*)&crctable );
	}

	if(sectorconfig->use_alternate_datamark)
	{
		for(i=0;i<formatstab[trackencoding].len_datamarkp2;i++)
		{
			pushTrackCode(tg,sectorconfig->alternate_datamark,formatstab[trackencoding].clock_datamarkp2,currentside,sectorconfig->trackencoding);
			CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->alternate_datamark,(unsigned char*)&crctable );
		}
	}
	else
	{
		for(i=0;i<formatstab[trackencoding].len_datamarkp2;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding].data_datamarkp2,formatstab[trackencoding].clock_datamarkp2,currentside,sectorconfig->trackencoding);
			CRC16_Update(&CRC16_High,&CRC16_Low, formatstab[trackencoding].data_datamarkp2,(unsigned char*)&crctable );
		}
	}
		
	sectorconfig->startdataindex=tg->last_bit_offset/8;
	if(sectorconfig->input_data)
	{
		FastMFMFMgenerator(tg,currentside,sectorconfig->input_data,sectorconfig->sectorsize,sectorconfig->trackencoding);

		// data crc			
		for(i=0;i<sectorconfig->sectorsize;i++)
		{
			CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->input_data[i],(unsigned char*)&crctable );
		}
	}
	else
	{
		for(i=0;i<sectorconfig->sectorsize;i++)
		{
			pushTrackCode(tg,sectorconfig->fill_byte,0xFF,currentside,sectorconfig->trackencoding);
			CRC16_Update(&CRC16_High,&CRC16_Low, sectorconfig->fill_byte,(unsigned char*)&crctable );
		}
	}


	if(sectorconfig->use_alternate_data_crc&0x2)
	{
		// alternate crc
		pushTrackCode(tg,(unsigned char)(sectorconfig->data_crc&0xFF),     0xFF,currentside,sectorconfig->trackencoding);
		pushTrackCode(tg,(unsigned char)((sectorconfig->data_crc>>8)&0xFF),0xFF,currentside,sectorconfig->trackencoding);
	}
	else
	{	
		//bad crc
		if(sectorconfig->use_alternate_data_crc&0x1)
		{
			pushTrackCode(tg,(unsigned char)(CRC16_High^0x21),0xFF,currentside,sectorconfig->trackencoding);
			pushTrackCode(tg,(unsigned char)(CRC16_Low ^0x20),0xFF,currentside,sectorconfig->trackencoding);
		}
		else
		{
			pushTrackCode(tg,CRC16_High,0xFF,currentside,sectorconfig->trackencoding);
			pushTrackCode(tg,CRC16_Low ,0xFF,currentside,sectorconfig->trackencoding);
		}
	}
					
	//gap3
	if(sectorconfig->gap3!=255)
	{
		for(i=0;i<sectorconfig->gap3;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding].data_gap3,0xFF,currentside,sectorconfig->trackencoding);
		}
	}


	// fill timing & encoding buffer
	if(currentside->timingbuffer)
	{
		for(j=startindex;j<(tg->last_bit_offset/8);j++)
		{
			currentside->timingbuffer[j]=sectorconfig->bitrate;
		}
	}

	switch(trackencoding)
	{
		case IBMFORMAT_SD:
		case ISOFORMAT_SD:
			trackenc=ISOIBM_FM_ENCODING;
		break;

		case ISOFORMAT_DD11S:
		case IBMFORMAT_DD:
		case ISOFORMAT_DD:
			trackenc=ISOIBM_MFM_ENCODING;
		break;
		
		default:
			trackenc=ISOIBM_MFM_ENCODING;
		break;
	}

	if(currentside->track_encoding_buffer)
	{
		for(j=startindex;j<(tg->last_bit_offset/8);j++)
		{
			currentside->track_encoding_buffer[j]=trackenc;
		}
	}

	currentside->number_of_sector++;
}
////////////////////////
//
// Amiga Sector
// Gap :  0xFF 0xFF
// Sync : 0xA1 0xA1 (Clock 0x0A 0x0A)
//  ->Sector ID : 0xFF(B3) TR(B2) SE(B1) 11-SE(B0)
//  Sector ID (even B3) 
//  Sector ID (even B2)
//  Sector ID (even B1)
//  Sector ID (even B0)
//  Sector ID (odd  B3)
//  Sector ID (odd  B2)
//  Sector ID (odd  B1)
//  Sector ID (odd  B0)
//  Gap - 16 bytes (0x00)
//  -> Header CRC
//  Header CRC (odd B3)
//  Header CRC (odd B2)
//  Header CRC (odd B1)
//  Header CRC (odd B0)
//  Header CRC (even B3)
//  Header CRC (even B2)
//  Header CRC (even B1)
//  Header CRC (even B0)
//  Data CRC (odd B3)
//  Data CRC (odd B2)
//  Data CRC (odd B1)
//  Data CRC (odd B0)
//  Data CRC (even B3)
//  Data CRC (even B2)
//  Data CRC (even B1)
//  Data CRC (even B0)
//  Data ( even and odd) 

void tg_addAmigaSectorToTrack(track_generator *tg,SECTORCONFIG * sectorconfig,SIDE * currentside)
{

	unsigned short  i;
	unsigned char   trackencoding,trackenc;
	unsigned char   CRC16_High;
	unsigned char   CRC16_Low;
	unsigned char   crctable[32];
	unsigned long   startindex,j;
	unsigned char   header[4];
	unsigned char   headerparity[2];
	unsigned char   sectorparity[2];

	startindex=tg->last_bit_offset/8;
	
	sectorconfig->startsectorindex=tg->last_bit_offset/8;
	trackencoding=sectorconfig->trackencoding-1;
			
	// sync
	for(i=0;i<formatstab[trackencoding].len_ssync;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_ssync,0xFF,currentside,sectorconfig->trackencoding);
	}

	// add mark
	for(i=0;i<formatstab[trackencoding].len_addrmarkp1;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_addrmarkp1,formatstab[trackencoding].clock_addrmarkp1,currentside,sectorconfig->trackencoding);
		CRC16_Update(&CRC16_High,&CRC16_Low, formatstab[trackencoding].data_addrmarkp1,(unsigned char*)&crctable);
	}

	headerparity[0]=0;
	headerparity[1]=0;

	header[0]=0xFF;
	header[1]=(sectorconfig->cylinder<<1) | (sectorconfig->head&1);
	header[2]=sectorconfig->sector;
	header[3]=sectorconfig->sectorsleft;
	
	pushTrackCode(tg,(unsigned char)(( odd_tab[header[0]]<<4)|( odd_tab[header[1]])),0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)(( odd_tab[header[2]]<<4)|( odd_tab[header[3]])),0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)((even_tab[header[0]]<<4)|(even_tab[header[1]])),0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)((even_tab[header[2]]<<4)|(even_tab[header[3]])),0xFF,currentside,sectorconfig->trackencoding);

	headerparity[0]^=( odd_tab[header[0]]<<4)|( odd_tab[header[1]]);
	headerparity[1]^=( odd_tab[header[2]]<<4)|( odd_tab[header[3]]);
	headerparity[0]^=(even_tab[header[0]]<<4)|(even_tab[header[1]]);
	headerparity[1]^=(even_tab[header[2]]<<4)|(even_tab[header[3]]);

	// gap2
	for(i=0;i<formatstab[trackencoding].len_gap2;i++)
	{
		pushTrackCode(tg,formatstab[trackencoding].data_gap2,0xFF,currentside,sectorconfig->trackencoding);
	}

	for(i=0;i<formatstab[trackencoding].len_gap2;i=i+2)
	{
		headerparity[0]^=formatstab[trackencoding].data_gap2;
		headerparity[1]^=formatstab[trackencoding].data_gap2;
	}

	pushTrackCode(tg,0x00,0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,0x00,0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)headerparity[0],0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)headerparity[1],0xFF,currentside,sectorconfig->trackencoding);

	sectorparity[0]=0;
	sectorparity[1]=0;
	if(sectorconfig->input_data)
	{
		for(i=0;i<sectorconfig->sectorsize;i=i+4)
		{
			sectorparity[0]^=(odd_tab[sectorconfig->input_data[i]]<<4) | odd_tab[sectorconfig->input_data[i+1]];
			sectorparity[1]^=(odd_tab[sectorconfig->input_data[i+2]]<<4) | odd_tab[sectorconfig->input_data[i+3]];
		}

		for(i=0;i<sectorconfig->sectorsize;i=i+4)
		{
			sectorparity[0]^=(even_tab[sectorconfig->input_data[i]]<<4) | even_tab[sectorconfig->input_data[i+1]];
			sectorparity[1]^=(even_tab[sectorconfig->input_data[i+2]]<<4) | even_tab[sectorconfig->input_data[i+3]];
		}		
	}

	pushTrackCode(tg,0x00,0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,0x00,0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)sectorparity[0],0xFF,currentside,sectorconfig->trackencoding);
	pushTrackCode(tg,(unsigned char)sectorparity[1],0xFF,currentside,sectorconfig->trackencoding);

	sectorconfig->startdataindex=tg->last_bit_offset/8;
	if(sectorconfig->input_data)
	{
		FastMFMFMgenerator(tg,currentside,sectorconfig->input_data,sectorconfig->sectorsize,sectorconfig->trackencoding);
	}
	else
	{
		for(i=0;i<sectorconfig->sectorsize;i++)
		{
			pushTrackCode(tg,sectorconfig->fill_byte,0xFF,currentside,sectorconfig->trackencoding);
		}
	}
					
	//gap3
	if(sectorconfig->gap3!=255)
	{
		for(i=0;i<sectorconfig->gap3;i++)
		{
			pushTrackCode(tg,formatstab[trackencoding].data_gap3,0xFF,currentside,sectorconfig->trackencoding);
		}
	}

	// fill timing & encoding buffer
	if(currentside->timingbuffer)
	{
		for(j=startindex;j<(tg->last_bit_offset/8);j++)
		{
			currentside->timingbuffer[j]=sectorconfig->bitrate;
		}
	}

	trackenc=AMIGA_MFM_ENCODING;

	if(currentside->track_encoding_buffer)
	{
		for(j=startindex;j<(tg->last_bit_offset/8);j++)
		{
			currentside->track_encoding_buffer[j]=trackenc;
		}
	}

	currentside->number_of_sector++;
}

void tg_addSectorToTrack(track_generator *tg,SECTORCONFIG * sectorconfig,SIDE * currentside)
{

	switch(sectorconfig->trackencoding)
	{

		case IBMFORMAT_SD:
		case IBMFORMAT_DD:
		case ISOFORMAT_SD:
		case ISOFORMAT_DD:
		case ISOFORMAT_DD11S:
			tg_addISOSectorToTrack(tg,sectorconfig,currentside);
			break;

		case AMIGAFORMAT_DD:
			tg_addAmigaSectorToTrack(tg,sectorconfig,currentside);
			break;

	}
}

void tg_completeTrack(track_generator *tg, SIDE * currentside,unsigned char trackencoding)
{
	int tracklen,trackoffset;
	unsigned int startindex,i;

	tracklen=currentside->tracklen/8;
	if(currentside->tracklen&7) tracklen++;

	startindex=tg->last_bit_offset/8;
	trackoffset=startindex;
	while(trackoffset<tracklen)
	{
		pushTrackCode(tg,formatstab[trackencoding-1].data_gap4b,0xFF,currentside,trackencoding);
		trackoffset=tg->last_bit_offset/8;
	}
	// fill timing & encoding buffer
	if(currentside->timingbuffer)
	{
		for(i=startindex;i<(tg->last_bit_offset/8);i++)
		{
			currentside->timingbuffer[i]=currentside->timingbuffer[startindex-1];
		}
	}

	if(currentside->track_encoding_buffer)
	{
		for(i=startindex;i<(tg->last_bit_offset/8);i++)
		{
			currentside->track_encoding_buffer[i]=currentside->track_encoding_buffer[startindex-1];
		}
	}
}

SIDE * tg_generatetrackEx(unsigned short number_of_sector,SECTORCONFIG * sectorconfigtab,unsigned char interleave,unsigned char skew,unsigned int bitrate,unsigned short rpm,unsigned char trackencoding,int indexlen,int indexpos)
{
	unsigned short i;
	unsigned long tracksize;
	unsigned long track_period,wanted_trackperiod,indexperiod;
	unsigned char * interleavetab;
	unsigned char gap3tocompute;
	unsigned long gap3period,computedgap3;
	int gap3;

	track_generator tg;
	SIDE * currentside;

	// compute the sectors interleaving.
	interleavetab=compute_interleave_tab(interleave,skew,number_of_sector);

	tg_initTrackEncoder(&tg);

	// get minimum track size
	tracksize=tg_computeMinTrackSize(&tg,trackencoding,bitrate,number_of_sector,sectorconfigtab,&track_period);
	
	wanted_trackperiod=(100000*60)/rpm;
	
	
	// compute the adjustable gap3 lenght
	// how many gap3 we need to compute ?
	gap3tocompute=0;
	for(i=0;i<number_of_sector;i++)
	{
		// if gap3 not set...
		if(sectorconfigtab[i].gap3==0xFF)
		{
			gap3tocompute++;
		}

	}

	indexperiod=0;
	if(indexlen&NO_SECTOR_UNDER_INDEX)
	{
		indexperiod=(indexlen&0xFFFFFF)/10;
	}


	//first try : get a standard value...
	if(gap3tocompute==number_of_sector)
	{
		gap3=searchgap3value(number_of_sector,sectorconfigtab);
		if(gap3!=-1)
		{

			for(i=0;i<number_of_sector;i++)
			{
				sectorconfigtab[i].gap3=(unsigned char)gap3;
			}
			gap3tocompute=0;
		}

	}
	// compute the dispatched the gap3 period
	gap3period=0;
	if(gap3tocompute && (wanted_trackperiod>(track_period+indexperiod)))
	{
		gap3period=wanted_trackperiod-(track_period+indexperiod);
		gap3period=gap3period/gap3tocompute;


		// set the right gap3 lenght according to the sector bitrate
		for(i=0;i<number_of_sector;i++)
		{
			// if gap3 not set...
			if(sectorconfigtab[i].gap3==0xFF)
			{	
				// TODO: make integer this...
				computedgap3=(unsigned long)floor((float)gap3period*(float)((float)sectorconfigtab[i].bitrate/(float)100000));

				switch(sectorconfigtab[i].trackencoding)
				{
					case IBMFORMAT_SD:
					case ISOFORMAT_SD:
						computedgap3=computedgap3/(2*8);
					break;

					case ISOFORMAT_DD11S:
					case IBMFORMAT_DD:
					case ISOFORMAT_DD:
						computedgap3=computedgap3/(1*8);
					break;
				}

				if(computedgap3>200) 
					computedgap3=200;
				sectorconfigtab[i].gap3=(unsigned char)computedgap3;

				//floppycontext->hxc_printf(MSG_DEBUG,"Sector:%d Computed Gap:%d",sectorconfigtab[i].sector, computedgap3);
			}
		}
	}

	// recompute the track size with the new gap settings.
	tracksize=tg_computeMinTrackSize(&tg,trackencoding,bitrate,number_of_sector,sectorconfigtab,&track_period);


	// adjust the track lenght to get the right rpm.
	if(wanted_trackperiod>track_period)
	{
		tracksize=tracksize+((((wanted_trackperiod-track_period) * (bitrate/4) )/(12500)));
	}

	// align the track size
	if(tracksize&0x1F)
	{
		tracksize=(tracksize&(~0x1F))+0x20;
	}


	// alloc the track...
	currentside=tg_initTrack(&tg,tracksize,number_of_sector,trackencoding,bitrate,sectorconfigtab);

	// and write all sectors to it...
	for(i=0;i<number_of_sector;i++)
	{
		tg_addSectorToTrack(&tg,&sectorconfigtab[interleavetab[i]],currentside);
	}

	// "close" the track : extend/add post gap..
	tg_completeTrack(&tg,currentside,trackencoding);

	if(indexlen & REVERTED_INDEX)
	{
		fillindex(indexpos,currentside,indexlen,1,1);
	}
	else
	{
		fillindex(indexpos,currentside,indexlen,1,0);
	}

	if(interleavetab) free(interleavetab);

	return currentside;
}



SIDE * tg_generatetrack(unsigned char * sectors_data,unsigned short sector_size,unsigned short number_of_sector,unsigned char track,unsigned char side,unsigned char sectorid,unsigned char interleave,unsigned char skew,unsigned int bitrate,unsigned short rpm,unsigned char trackencoding,unsigned char gap3, int indexlen,int indexpos)
{
	unsigned short i;
	SIDE * currentside;
	SECTORCONFIG * sectorconfigtab;

	sectorconfigtab=malloc(sizeof(SECTORCONFIG)*number_of_sector);
	memset(sectorconfigtab,0,sizeof(SECTORCONFIG)*number_of_sector);

	for(i=0;i<number_of_sector;i++)
	{
		sectorconfigtab[i].cylinder=track;
		sectorconfigtab[i].head=side;
		sectorconfigtab[i].bitrate=bitrate;//+(10000*i);
		sectorconfigtab[i].gap3=gap3;
		sectorconfigtab[i].input_data=&sectors_data[sector_size*i];
		sectorconfigtab[i].sectorsize=sector_size;
		sectorconfigtab[i].trackencoding=trackencoding;
		sectorconfigtab[i].sector=sectorid+i;
		sectorconfigtab[i].sectorsleft=number_of_sector-i; // Used in Amiga tracks.
	}

	currentside=tg_generatetrackEx(number_of_sector,sectorconfigtab,interleave,skew,bitrate,rpm,trackencoding,indexlen,indexpos);
	free(sectorconfigtab);

	return currentside;
}

SIDE * tg_alloctrack(unsigned int bitrate,unsigned char trackencoding,unsigned short rpm,unsigned int tracksize,int indexlen,int indexpos,unsigned char buffertoalloc)
{
	SIDE * currentside;
	unsigned int tracklen;
	unsigned int i;

	currentside=(SIDE*)malloc(sizeof(SIDE));
	memset(currentside,0,sizeof(SIDE));
				
	currentside->number_of_sector=0;

	tracklen=tracksize/8;
	if(tracksize&7) tracklen++;

	currentside->tracklen=tracksize;

	//////////////////////////////
	// bitrate buffer allocation
	currentside->bitrate=bitrate;

	if(buffertoalloc & TG_ALLOCTRACK_ALLOCTIMIMGBUFFER)
	{
		currentside->bitrate=VARIABLEBITRATE;
		currentside->timingbuffer=malloc(tracklen*sizeof(unsigned long));
		for(i=0;i<tracklen;i++)
		{
			currentside->timingbuffer[i]=bitrate;
		}
	}

		
	///////////////////////////////////////////
	// track encoding code buffer allocation
	currentside->track_encoding=trackencoding;
	if(buffertoalloc & TG_ALLOCTRACK_ALLOCENCODINGBUFFER)
	{
		currentside->track_encoding=VARIABLEENCODING;
		currentside->track_encoding_buffer=malloc(tracklen*sizeof(unsigned char));

		for(i=0;i<tracklen;i++)
		{
			currentside->track_encoding_buffer[i]=trackencoding;
		}
	}

	///////////////////////////////////////////
	// track flakey bits allocation
	if(buffertoalloc & TG_ALLOCTRACK_ALLOCFLAKEYBUFFER)
	{
		currentside->flakybitsbuffer=malloc(tracklen*sizeof(unsigned char));
		
		if(buffertoalloc & TG_ALLOCTRACK_UNFORMATEDBUFFER )
		{
			memset(currentside->flakybitsbuffer,0xFF,tracklen*sizeof(unsigned char));
		}
		else
		{
			memset(currentside->flakybitsbuffer,0x00,tracklen*sizeof(unsigned char));
		}
	}
	
	/////////////////////////////
	// data buffer allocation
	currentside->databuffer=malloc(tracklen);
	memset(currentside->databuffer,0,tracklen);
	if(buffertoalloc & TG_ALLOCTRACK_RANDOMIZEDATABUFFER)
	{
		for(i=0;i<tracklen;i++)
		{
			currentside->databuffer[i]=rand();
		}
	}

	/////////////////////////////
	// index buffer allocation
	currentside->indexbuffer=malloc(tracklen);
	memset(currentside->indexbuffer,0,tracklen);

	if(indexlen & REVERTED_INDEX)
	{
		fillindex(indexpos,currentside,indexlen,1,1);
	}
	else
	{
		fillindex(indexpos,currentside,indexlen,1,0);
	}

	return currentside;
}



unsigned long * tg_allocsubtrack_long(unsigned int tracksize,unsigned long initvalue)
{
	unsigned int tracklen;
	unsigned int i;
	unsigned long * ptr;

	tracklen=tracksize/8;
	if(tracksize&7) tracklen++;

	ptr=malloc(tracklen*sizeof(unsigned long));
	if(ptr)
	{
		for(i=0;i<tracklen;i++)
		{
			ptr[i]=initvalue;
		}
	}
	
	return ptr;
}

unsigned char * tg_allocsubtrack_char(unsigned int tracksize,unsigned char initvalue)
{
	unsigned int tracklen;
	unsigned int i;
	unsigned char * ptr;

	tracklen=tracksize/8;
	if(tracksize&7) tracklen++;

	ptr=malloc(tracklen*sizeof(unsigned char));
	if(ptr)
	{
		for(i=0;i<tracklen;i++)
		{
			ptr[i]=initvalue;
		}
	}
	
	return ptr;
}
