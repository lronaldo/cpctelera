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
////////////////////////////////////////////////////////////////////////////////////////////////////
// Floppy Descriptor
//
//
#define IBMPC_DD_FLOPPYMODE				0x00
#define IBMPC_HD_FLOPPYMODE				0x01
#define ATARIST_DD_FLOPPYMODE			0x02
#define ATARIST_HD_FLOPPYMODE			0x03
#define AMIGA_DD_FLOPPYMODE				0x04
#define AMIGA_HD_FLOPPYMODE				0x05
#define CPC_DD_FLOPPYMODE				0x06
#define GENERIC_SHUGART_DD_FLOPPYMODE	0x07
#define IBMPC_ED_FLOPPYMODE				0x08
#define MSX2_DD_FLOPPYMODE				0x09
#define C64_DD_FLOPPYMODE				0x0A
#define EMU_SHUGART_FLOPPYMODE			0x0B
#define S950_DD_FLOPPYMODE				0x0C
#define S950_HD_FLOPPYMODE				0x0D

#define CALLINGMETHOD

#define VARIABLEBITRATE					-1
#define VARIABLEENCODING   			    -1

#define ISOIBM_MFM_ENCODING				0x00
#define AMIGA_MFM_ENCODING				0x01
#define ISOIBM_FM_ENCODING				0x02
#define EMU_FM_ENCODING					0x03
#define UNKNOWN_ENCODING				0xFF


typedef struct SIDE_
{
	unsigned int	number_of_sector;	// nombre de secteurs sur la piste (informatif/optionnel) -> -1 si inconnu.
	unsigned char * databuffer;			// buffer data
	long			bitrate;			// si == a VARIABLEBITRATE utiliser timingbuffer
	unsigned long * timingbuffer;		// buffer bitrate de la piste.
	unsigned char * flakybitsbuffer;    // si = 0 pas de flakey/weak bits.
	unsigned char * indexbuffer;		// buffer signal index 1->activé 0->désactivé
	unsigned char * track_encoding_buffer;		// buffer code codage
	
	
	unsigned char   track_encoding;

	unsigned long	tracklen;			// longueur de  databuffer/timingbuffer/flakybitsbuffer/indexbuffer (nombre d'elements)
}SIDE;

typedef struct CYLINDER_
{
	unsigned short	floppyRPM;			// rotation par minute (informatif/optionnel)
	unsigned char	number_of_side;
	SIDE	**		sides;
}CYLINDER;

typedef struct FLOPPY_
{
	unsigned int	floppyBitRate;
	
	unsigned char	floppyNumberOfSide;
	unsigned short	floppyNumberOfTrack;
	unsigned short	floppySectorPerTrack;
	
	unsigned short	floppyiftype;

	CYLINDER	**	tracks;
}FLOPPY;


