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
#define FLOPPYEMUFREQ 16000000

typedef struct trackzone_
{
	int start;
	int end;
	unsigned long bitrate;

	unsigned char code1;
	unsigned int code1lenint;

	unsigned char code2;
	unsigned int code2lenint;

}trackzone;

typedef struct trackpart_
{
	unsigned char code;
	int len;
}trackpart;

#define GRANULA 64

int GetNewTrackRevolution(HXCFLOPPYEMULATOR* floppycontext,unsigned char * index_h0,unsigned char * datah0,unsigned int lendatah0,unsigned char * datah1,unsigned int lendatah1,unsigned char * randomh0,unsigned char * randomh1,long fixedbitrateh0,unsigned long * timeh0,long fixedbitrateh1,unsigned long * timeh1,unsigned char ** finalbuffer,unsigned char ** randomfinalbuffer,unsigned char readysignal,unsigned char diskchange,unsigned char writeprotect,unsigned char amigaready,unsigned char selectconfig);




