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

typedef struct cpcdsk_fileheader_
{
 char headertag[34]; // "EXTENDED CPC DSK File\r\nDisk-Info\r\n" ou  "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
 char creatorname[14];
 unsigned char number_of_tracks;
 unsigned char number_of_sides;
 unsigned short size_of_a_track; // not used in extended cpc dsk file
}cpcdsk_fileheader;


typedef struct cpcdsk_trackheader_
{
 char headertag[13];  	// "Track-Info\r\n"
 unsigned short unused1;
 unsigned char unused1b;
 unsigned char track_number;
 unsigned char side_number;
 unsigned char datarate;
 unsigned char rec_mode;
 unsigned char sector_size_code;
 unsigned char number_of_sector;
 unsigned char gap3_length;
 unsigned char filler_byte;
}cpcdsk_trackheader;

typedef struct cpcdsk_sector_
{
 unsigned char   track;
 unsigned char   side;
 unsigned char   sector_id;
 unsigned char   sector_size_code;
 unsigned char	 fdc_status_reg1;
 unsigned char	 fdc_status_reg2;
 unsigned short  data_lenght;
}cpcdsk_sector;

#pragma pack()
