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
#pragma pack(1)

typedef struct vtrucco_picfileformatheader_
{
	unsigned char HEADERSIGNATURE[7];
	unsigned char write_protected;
	unsigned short floppyRPM;

	unsigned char number_of_track;
	unsigned char floppyinterfacemode;
	unsigned char number_of_side;

	unsigned char track_encoding;
	unsigned short bitRate;
	unsigned short track_list_offset;

	unsigned char formatrevision;

	unsigned char RESERVED[100];

	unsigned char input_filename[128];

	unsigned char RESERVED2[50];

	unsigned char CREDITS[73];

}vtrucco_picfileformatheader;


typedef struct vtrucco_pictrack_
{
	unsigned short offset;
	unsigned short track_len;
}vtrucco_pictrack;

#pragma pack()


int write_vtrucco_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename,int forceifmode);



