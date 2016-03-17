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

typedef struct pasti_fileheader_
{
 char headertag[4]; // "RSY\0"
 unsigned short codeversion1;
 unsigned short codeversion2;
 unsigned short unused1;
 unsigned char  number_of_track; //valide si <= 0xA8
 unsigned char  unknowvalue;
 unsigned long  unused2;
}pasti_fileheader;


typedef struct pasti_trackheader_
{
 unsigned long  tracksize;
 unsigned long  unused1;
 unsigned short numberofsector;
 unsigned short flags;
 unsigned short Tvalue;
 unsigned char  track_code;
 unsigned char  unused2;

}pasti_trackheader;

typedef struct pasti_sector_
{
 unsigned long   sector_pos;
 unsigned short  sector_pos_timing;
 unsigned short  sector_speed_timing;
 unsigned char   track_num;
 unsigned char   side_num;
 unsigned char	 sector_num;
 unsigned char	 sector_size;
 unsigned short  header_crc;
 unsigned char   FDC_status;
 unsigned char   sector_flags; // (always 00)
}pasti_sector;

#pragma pack()
