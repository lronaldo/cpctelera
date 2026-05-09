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
#define DEFAULT_HD_BITRATE 500000
#define DEFAULT_DD_BITRATE 250000
#define DEFAULT_AMIGA_BITRATE 253360

#define DEFAULT_DD_RPM 300
#define DEFAULT_AMIGA_RPM 300

typedef struct track_generator_
{
	unsigned long last_bit_offset;
	
	unsigned short mfm_last_bit;
}track_generator;



typedef struct SECTORCONFIG_
{
	unsigned char   head;
	unsigned char   sector;
	unsigned char   sectorsleft;
	unsigned char   cylinder;

	unsigned int   sectorsize;

	unsigned char  use_alternate_sector_size_id;
	unsigned char  alternate_sector_size_id;

	unsigned char  missingdataaddressmark;

	unsigned char  use_alternate_header_crc;	//0x1 -> Bad crc  , 0x2 alternate crc
	unsigned short data_crc;

	unsigned char  use_alternate_data_crc;		//0x1 -> Bad crc  , 0x2 alternate crc
	unsigned short header_crc;

	unsigned char  use_alternate_datamark;
	unsigned char  alternate_datamark;

	unsigned char  use_alternate_addressmark;
	unsigned char  alternate_addressmark;

	unsigned long startsectorindex;
	unsigned long startdataindex;

	unsigned char  trackencoding;

	unsigned char  gap3;

	unsigned int   bitrate;

	unsigned char * input_data;
	unsigned char fill_byte;
}SECTORCONFIG;

typedef struct isoibm_config_
{
	// format index
	int indexformat;

	// post index gap4 config
	unsigned char	data_gap4a;
	unsigned int	len_gap4a;

	// index sync config 
	unsigned char	data_isync;
	unsigned int	len_isync;

	// index mark coding
	unsigned char	data_indexmarkp1;
	unsigned char	clock_indexmarkp1;
	unsigned char	len_indexmarkp1;

	unsigned char	data_indexmarkp2;
	unsigned char	clock_indexmarkp2;
	unsigned char	len_indexmarkp2;

	// gap1 config

	unsigned char	data_gap1;
	unsigned int	len_gap1;

	// h sync config 
	unsigned char	data_ssync;
	unsigned int	len_ssync;

	// d sync config 
	unsigned char	data_dsync;
	unsigned int	len_dsync;

	// address mark coding
	unsigned char	data_addrmarkp1;
	unsigned char	clock_addrmarkp1;
	unsigned char	len_addrmarkp1;

	unsigned char	data_addrmarkp2;
	unsigned char	clock_addrmarkp2;
	unsigned char	len_addrmarkp2;

	// gap2 config
	unsigned char	data_gap2;
	unsigned int	len_gap2;

	// data mark coding
	unsigned char	data_datamarkp1;
	unsigned char	clock_datamarkp1;
	unsigned char	len_datamarkp1;

	unsigned char	data_datamarkp2;
	unsigned char	clock_datamarkp2;
	unsigned char	len_datamarkp2;

	// gap3 config
	unsigned char	data_gap3;
	unsigned int	len_gap3;

	unsigned char	data_gap4b;
	unsigned int	len_gap4b;
	
}isoibm_config;







#define IBMFORMAT_SD 0x1
#define IBMFORMAT_DD 0x2
#define ISOFORMAT_SD 0x3
#define ISOFORMAT_DD 0x4
#define ISOFORMAT_DD11S 0x5
#define AMIGAFORMAT_DD  0x6

static isoibm_config formatstab[]=
{    //     I         --gap4a --i sync --     index mark      --  gap1 --h sync -- --d sync --    add mark            -- gap2 --
	{	
		IBMFORMAT_SD,	 
			
		0xFF,40, // post index gap4 config
		
		0x00, 6, // index sync config 
		
		0x00,0x00,0,// index mark coding
		0xFC,0xD7,1, 
		
		0xFF,26, // gap1 config
		
		0x00, 6, // h sync config
		
		0x00, 6, // d sync config
		
		0x00,0x00,0,// address mark coding
		0xFE,0xC7,1, 
		
		0xFF,11, // gap2 config
		
		0x00,0x00,0,// data mark coding
		0xFB,0xC7,1, 
		
		0xFF,84, // gap3 config
		
		0xFF,255 // gap4 config
	},
	
	
	{	
		IBMFORMAT_DD,	 
		
		0x4E,80, // post index gap4 config
		
		0x00,12, // index sync config 
		
		0xC2,0x14,3,// index mark coding
		0xFC,0xFF,1, 
		
		0x4E,50, // gap1 config
		
		0x00,12, // h sync config
		
		0x00,12, // d sync config
		
		0xA1,0x0A,3,// address mark coding
		0xFE,0xFF,1, 
		
		0x4E,22, // gap2 config
		
		0xA1,0x0A,3,// data mark coding
		0xFB,0xFF,1, 
		
		0x4E,84, // gap3 config
		
		0x4E,255 // gap4 config
		
	},
	
	{	
		ISOFORMAT_SD,
		
		0xFF,00, // post index gap4 config
		
		0x00,00, // index sync config 
		
		0x00,0x00,0,// index mark coding
		0xFC,0xD7,1,
		
		0xFF,16, // gap1 config
		
		0x00,06, // h sync config
		
		0x00,06, // d sync config
		
		0x00,0x00,0,// address mark coding
		0xFE,0xC7,1,
		
		0xFF,11, // gap2 config
		
		0x00,0x00,0,// data mark coding
		0xFB,0xC7,1,
		
		0xFF,84, // gap3 config
		
		0xFF,255 // gap4 config
		
	},
	
	{	
		ISOFORMAT_DD,
		
		0x4E,00, // post index gap4 config
		
		0x00,00, // index sync config 
		
		0x00,0x00,0,// index mark coding
		0x00,0x00,0, 
		
		0x4E,32, // gap1 config
		
		0x00,12, // h sync config
		
		0x00,12, // d sync config
		
		0xA1,0x0A,3,// address mark coding
		0xFE,0xFF,1,
		
		0x4E,22, // gap2 config
		
		0xA1,0x0A,3,// data mark coding
		0xFB,0xFF,1,
		
		0x4E,84, // gap3 config
		0x4E,255 // gap4 config
		
	},	
	{	
		ISOFORMAT_DD11S,
		
		0x4E,00, // post index gap4 config
		
		0x00,00, // index sync config 
		
		0x00,0x00,0,// index mark coding
		0x00,0x00,0, 
		
		0x4E,00, // gap1 config
		
		0x00,03, // h sync config
		
		0x00,12, // d sync config
		
		0xA1,0x0A,3,// address mark coding
		0xFE,0xFF,1,
		
		0x4E,22, // gap2 config
		
		0xA1,0x0A,3,// data mark coding
		0xFB,0xFF,1,
		
		0x4E,5,  // gap3 config
		
		0x4E,0xFF // gap4 config
		
	},	
	{	
		AMIGAFORMAT_DD,
		
		0x4E,00, // post index gap4 config
		
		0x00,00, // index sync config 
		
		0x00,0x00,0,// index mark coding
		0x00,0x00,0, 
		
		0x4E,00, // gap1 config
		
		0x00,02, // h sync config
		
		0x00,12, // d sync config
		
		0xA1,0x0A,2,// address mark coding (0x4489 0x4489)
		0xFF,0xFF,0,
		
		0x00,16, // gap2 config
		
		0xA1,0x0A,0,// data mark coding
		0xFB,0xFF,0,
		
		0x00,0,  // gap3 config
		
		0xFF,0xFF // gap4 config
		
	},	

{	
		0,
		
		0x4E,00, // post index gap4 config
		
		0x00,00, // index sync config 
		
		0x00,0x00,0,// index mark coding
		0x00,0x00,0,
		
		0x4E,32, // gap1 config
		
		0x00,12,
		
		0x00,12,
		
		0xA1,0xFF,3,
		0xFE,0xFF,1,
		
		0x4E,22,
		
		0xA1,0xFF,3,
		0xFB,0xFF,1,
		
		0x4E,84,
		
		0x4E,255
		}
};

int BuildCylinder(unsigned char * mfm_buffer,int mfm_size,unsigned char * track_clk,unsigned char * track_data,int track_size);
void BuildFMCylinder(char * buffer,int fmtracksize,char * bufferclk,char * track,int size);

void getMFMcode(track_generator *tg,unsigned char data,unsigned char clock,unsigned char * dstbuf);
void getFMcode (track_generator *tg,unsigned char data,unsigned char clock,unsigned char * dstbuf);
int  pushTrackCode(track_generator *tg,unsigned char data,unsigned char clock,SIDE * side,unsigned char trackencoding);

void          tg_initTrackEncoder(track_generator *tg);
unsigned long tg_computeMinTrackSize(track_generator *tg,unsigned char trackencoding,unsigned int bitrate,unsigned int numberofsector,SECTORCONFIG * sectorconfigtab,unsigned long * track_period);
void          tg_addSectorToTrack(track_generator *tg,SECTORCONFIG * sectorconfig,SIDE * currentside);
void          tg_completeTrack(track_generator *tg, SIDE * currentside,unsigned char trackencoding);
SIDE *		  tg_initTrack(track_generator *tg,unsigned long tracksize,unsigned short numberofsector,unsigned char trackencoding,unsigned int bitrate,SECTORCONFIG * sectorconfigtab);
SIDE *        tg_generatetrack(unsigned char * sectors_data,unsigned short sector_size,unsigned short number_of_sector,unsigned char track,unsigned char side,unsigned char sectorid,unsigned char interleave,unsigned char skew,unsigned int bitrate,unsigned short rpm,unsigned char trackencoding,unsigned char gap3,int indexlen,int indexpos);
SIDE *        tg_generatetrackEx(unsigned short number_of_sector,SECTORCONFIG * sectorconfigtab,unsigned char interleave,unsigned char skew,unsigned int bitrate,unsigned short rpm,unsigned char trackencoding,int indexlen,int indexpos);
SIDE *        tg_alloctrack(unsigned int bitrate,unsigned char trackencoding,unsigned short rpm,unsigned int tracksize,int indexlen,int indexpos,unsigned char buffertoalloc);

unsigned long * tg_allocsubtrack_long(unsigned int tracksize,unsigned long initvalue);
unsigned char * tg_allocsubtrack_char(unsigned int tracksize,unsigned char initvalue);

#define       TG_ALLOCTRACK_ALLOCTIMIMGBUFFER 0x01
#define       TG_ALLOCTRACK_ALLOCFLAKEYBUFFER 0x02
#define       TG_ALLOCTRACK_ALLOCENCODINGBUFFER 0x04
#define       TG_ALLOCTRACK_RANDOMIZEDATABUFFER 0x08
#define       TG_ALLOCTRACK_UNFORMATEDBUFFER 0x10
