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
#define byte  unsigned char
#define word  unsigned short
#define dword unsigned int

#pragma pack(1)

typedef struct MFMIMG_
{
	byte headername[7];

	word number_of_track;
	byte number_of_side;
	
	word floppyRPM;
	word floppyBitRate;
	byte floppyiftype;

	dword mfmtracklistoffset;
}MFMIMG;


typedef struct MFMTRACKIMG_
{
	word track_number;
	byte side_number;
	dword mfmtracksize;
	dword mfmtrackoffset;	
}MFMTRACKIMG;

#pragma pack()



int write_MFM_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename);


