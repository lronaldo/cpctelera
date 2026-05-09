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

#define JV3_HEADER_MAX  2901

#define JV3_DENSITY     0x80  /* 1=dden, 0=sden */
#define JV3_DAM         0x60  /* data address mark code; see below */
#define JV3_SIDE        0x10  /* 0=side 0, 1=side 1 */
#define JV3_ERROR       0x08  /* 0=ok, 1=CRC error */
#define JV3_NONIBM      0x04  /* 0=normal, 1=short */
#define JV3_SIZE        0x03  /* in used sectors: 0=256,1=128,2=1024,3=512
                                 in free sectors: 0=512,1=1024,2=128,3=256 */
#define JV3_FREE        0xFF  /* in track and sector fields of free sectors */
#define JV3_FREEF       0xFC  /* in flags field, or'd with size code */

#define JV3_DAM_FB_SD   0x00
#define JV3_DAM_FA_SD   0x01
#define JV3_DAM_F9_SD   0x02
#define JV3_DAM_F8_SD   0x03

#define JV3_DAM_FB_DD   0x00
#define JV3_DAM_F8_DD   0x01

#define JV3_SIZE_USED_256   0x00
#define JV3_SIZE_USED_128   0x01
#define JV3_SIZE_USED_1024   0x02
#define JV3_SIZE_USED_512   0x03

#define JV3_SIZE_FREE_256   0x03
#define JV3_SIZE_FREE_128   0x02
#define JV3_SIZE_FREE_1024  0x01
#define JV3_SIZE_FREE_512   0x00


typedef struct {
  unsigned char track;
  unsigned char sector;
  unsigned char flags;
} JV3SectorHeader;

typedef struct {
  unsigned int key;
  unsigned int offset;
  unsigned char DAM;
  unsigned char density;
  unsigned int size;
} JV3SectorsOffsets;

int JV3_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile);
int JV3_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters);
