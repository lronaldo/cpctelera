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

typedef struct stt_header_
{
	unsigned long	stt_signature;//STEM
	unsigned short	stt_version;
	unsigned short	stt_flags;
	unsigned short	tracks_flags;
	unsigned short	number_of_tracks;
	unsigned short	number_of_sides;	
}stt_header;

typedef struct stt_track_offset_
{
	unsigned long	track_offset;
	unsigned short	track_size;
}stt_track_offset;


typedef struct stt_track_header_
{
	unsigned long	stt_track_signature; //TRCK
	unsigned short	tracks_flags;
	unsigned short	section_size;
	unsigned short	sectors_flags;
	unsigned short	number_of_sectors;
}stt_track_header;


typedef struct stt_sector_
{
	unsigned char	track_nb_id;
	unsigned char	side_nb_id;
	unsigned char	sector_nb_id;
	unsigned char	sector_len_code;
	unsigned char	crc_byte_1;
	unsigned char	crc_byte_2;
	unsigned short	data_offset;
	unsigned short	data_len;
}stt_sector;

#pragma pack()
