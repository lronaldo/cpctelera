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

#include "pcbootsector.h"

/* MS DOS Floppy table

3.5 Inch
                       720 K   1.44 Mo   2.88 Mo
# of Heads (Sides)         2         2         2
# of Cyls (Tracks)        80        80        80
# of Sectors/Track         9        18        36
Total # of Sectors      1440      2880      5760
# of Free Sectors       1426      2847      5726
# Sectors/Cluster          2         1         2
Total # of Clusters      713      2847      2863
# Sectors/FAT              3         9         9
# of FAT Copies            2         2         2
# of Root Dir Sectors      7        14        15
# Reserved Sectors         1         1         1
# of Hidden Sectors        0         0         0
# of Bytes/Sector        512       512       512
# of Bytes/Cluster      1024       512      1024
# Root Dir Entries       112       224       240
Media Descriptor          F9        F0        F0
Recorded Density      Double      High      High
MS-DOS Version Began    3.20      3.30      5.00
DRIVER.SYS Switch       /F:2      /F:7      /F:9
FORMAT.EXE Switch     /F:720   /F:1.44   /F:2.88
Change-Line Support      YES       YES       YES

5.25 Inch
                        160K     180K     320K     360K     1200K
# of Heads (Sides)         1        1        2        2         2
# of Cyls (Tracks)        40       40       40       40        80
# of Sectors/Track         8        9        8        9        15
Total # of Sectors       320      360      640      720      2400
# of Free Sectors        313      351      630      708      2371
# Sectors/Cluster          1        1        2        2         1
Total # of Clusters      313      351      315      354      2371
# of Sectors/FAT           1        2        1        2         7
# of FAT Copies            2        2        2        2         2
# of Root Dir Sectors      4        4        7        7        14
# Reserved Sectors         1        1        1        1         1
# of Hidden Sectors        0        0        0        0         0
# of Bytes/Sector        512      512      512      512       512
# of Bytes/Cluster       512      512     1024     1024       512
# Root Dir Entries        64       64      112      112       224
Media Descriptor          FE       FC       FF       FD        F9
Recorded Density      Double   Double   Double   Double      High
MS-DOS Version Began    1.00     2.00     1.10     2.00      3.00
DRIVER.SYS Switch       /F:0     /F:0     /F:0     /F:0      /F:1
FORMAT.EXE Switch     /F:160   /F:180   /F:320   /F:360    /F:1.2
Change-Line Support       NO       NO       NO       NO       YES

8-Inch
                        250K     500K    1232K
# of Heads (Sides)         1        2        2
# of Cyls (Tracks)        77       77       77
# of Sectors/Track        26       26        8
Total # of Sectors      2002     4004     1232
# of Bytes/Sector        128      128     1024
# of Bytes/Cluster       512      512     1024
# Sectors/Cluster          4        4        1
Total # of Clusters      497      997     1227
# Reserved Sectors         1        4        1
# of Hidden Sectors        3        0        0
# Sectors/FAT              6        6        2
# of FAT Copies            2        2        2
# Root Dir Entries        68       68      192
Media Descriptor          FE       FD       FE
Recorded Density      Single   Single   Double
MS-DOS Version Began    1.00     2.00     2.00
Change-Line Support       NO       NO       NO

*/
typedef struct _fat12config
{
	char * dirext;
	int dir;
	int number_of_track;
	int number_of_side;
	int number_of_sectorpertrack;
	int rpm;
	int bitrate;
	unsigned char *bootsector;
	int gap3;
	int interleave;
	int interface_mode;
	int tracktype;
	unsigned char BPB_media;
	int clustersize;
	int sectorsize;
	int root_dir_entries;
	int reserved_sector;
}fat12config;


fat12config configlist[]=
{
	{".fatst902",0xFF,82,2,11,300,250000,0,3,2,ATARIST_DD_FLOPPYMODE,ISOFORMAT_DD11S,0xF9,2,512,112,1},
	{".fatst160",0xFF,  40 ,1,9,300,250000,0,84,1,ATARIST_DD_FLOPPYMODE,ISOFORMAT_DD,0xF9,2,512,112,1},
	{".fatst360",0xFF,80 ,1,9,300,250000,0,84,1,ATARIST_DD_FLOPPYMODE,ISOFORMAT_DD,0xF9,2,512,112,1},// Atari ST 3.5" SS
	{".fatst",0xFF,  80 ,2,9,300,250000,0,84,1,ATARIST_DD_FLOPPYMODE,ISOFORMAT_DD,0xF9,2,512,112,1},// Atari ST 3.5" 
	{".fat160a",0xFF,40 ,1,8,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFE,1,512,64,1},  //5.25" Single sided, 40 tracks per side, 8 sectors per track (160K). Also used for 8". 300 RPM
	{".fat160b",0xFF,40 ,1,8,360,300000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFE,1,512,64,1},  //5.25" Single sided, 40 tracks per side, 8 sectors per track (160K). Also used for 8". 360 RPM
	{".fat180a",0xFF,40 ,1,9,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFC,1,512,64,1},  //5.25" Single sided, 40 tracks per side, 9 sectors per track (180K) 300 RPM
	{".fat180b",0xFF,40 ,1,9,360,300000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFC,1,512,64,1},  //5.25" Single sided, 40 tracks per side, 9 sectors per track (180K) 360 RPM
	{".fat250" ,0xFF,77 ,1,26,300,250000,win95_bootsector,24,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_SD,0xFE,4,128,68,1},  //8" Single sided, 77 tracks per side, 26 sectors per track (250K) 
	{".fat500" ,0xFF,77 ,2,26,300,250000,win95_bootsector,24,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_SD,0xFD,4,128,68,4},  //8" Double sided, 77 tracks per side, 26 sectors per track (500K) 
	{".fat1232",0xFF,77 ,2,8,300,250000,win95_bootsector,24,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_SD,0xFE,1,1024,192,1},  //8" Double sided, 77 tracks per side, 26 sectors per track (1232K) 
	{".fat320ssa",0xFF, 80 ,1,8,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFA,2,512,112,1}, //5.25" Single sided, 80 tracks per side, 8 sectors per track (320K) 300 RPM
	{".fat320ssb",0xFF, 80 ,1,8,360,300000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFA,2,512,112,1}, //5.25" Single sided, 80 tracks per side, 8 sectors per track (320K) 360 RPM
	{".fat320dsa",0xFF, 40 ,2,8,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFF,2,512,112,1}, //5.25" Double sided, 40 tracks per side, 8 sectors per track (320K) 300 RPM
	{".fat320dsb",0xFF, 40 ,2,8,360,300000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFF,2,512,112,1}, //5.25" Double sided, 40 tracks per side, 8 sectors per track (320K) 360 RPM
	{".fat360a",0xFF,40 ,2,9,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFD,2,512,112,1},  //5.25" Double sided, 40 tracks per side, 9 sectors per track (360K). Also used for 8". 300 RPM
	{".fat360b",0xFF,40 ,2,9,360,300000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFD,2,512,112,1},  //5.25" Double sided, 40 tracks per side, 9 sectors per track (360K). Also used for 8". 360 RPM
	{".fat640",0xFF, 80 ,2,8,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xFB,2,512,112,1},  // 3.5" Double sided, 80 tracks per side, 8 sectors per track (640K)
	{".fat720",0xFF, 80 ,2,9,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xF9,2,512,112,1},  // 3.5" Double sided, 80 tracks per side, 9 sectors per track (720K)
	{".fat720pc",0xFF, 250 ,2,9,300,250000,win95_bootsector,84,1,IBMPC_DD_FLOPPYMODE,IBMFORMAT_DD,0xF9,2,512,112,1},  // 3.5" Double sided, 160 tracks per side, 9 sectors per track (1440K)
	{".fat1200_8i",0xFF,80 ,2,15,300,500000,win95_bootsector,84,1,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xFE,2,512,112,1}, //8" Double Sided, 80 tracks per side, 15 sectors per track (1.2MB)
	{".fat1200",0xFF,80 ,2,15,300,500000,msdos_bootsector,84,1,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF9,1,512,224,1}, //5.25" Double Sided, 80 tracks per side, 15 sectors per track (1.2MB)
	{".fat1440",0xFF,80 ,2,18,300,500000,win95_bootsector,84,1,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,1,512,224,1}, //3.5" Double Sided, 80 tracks per side, 18 sectors per track (1.44MB)
	{".fat4572",0xFF,254 ,2,18,300,500000,win95_bootsector,84,1,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,1,512,224,1}, //3.5" Double Sided, 80 tracks per side, 18 sectors per track (1.44MB)
	{".fat1680",0xFF,80 ,2,21,300,500000,win95_bootsector,14,2,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,1,512,224,1}, //3.5" Double Sided, 80 tracks per side, 21 sectors per track (1.68MB)
	{".fat2880",0xFF,80 ,2,36,300,1000000,win95_bootsector,84,1,IBMPC_ED_FLOPPYMODE,IBMFORMAT_DD,0xF0,2,512,240,1},//3.5" Double Sided, 80 tracks per side, 36 sectors per track (2.88MB)
	{".fat3381",0xFF,127,2,26,240,500000,win95_bootsector,14,2,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,2,512,224,1},
	{".fat6789",0xFF,255,2,27,240,500000,win95_bootsector,14,2,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,4,512,112,1},
	{".fatbigst",0xFF,127,2,27,75,250000,0,84,2,ATARIST_DD_FLOPPYMODE,ISOFORMAT_DD,0xF9,2,512,112,1},
	{".fatmonsterst",0xFF,254,2,17,200,250000,0,35,1,ATARIST_DD_FLOPPYMODE,ISOFORMAT_DD,0xF9,2,512,112,1},
	{".fatbig",0xFF,127,2,24,100,500000,msdos_bootsector,84,2,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,2,512,112,1},
	{".krz",0x00,80 ,2,18,300,500000,win95_bootsector,84,1,IBMPC_HD_FLOPPYMODE,IBMFORMAT_DD,0xF0,1,512,224,1}, //3.5" Double Sided, 80 tracks per side, 18 sectors per track (1.44MB)
	{"",0xFF,80,2,9,300,250000,0,0xF9,2,512,112,1}
};
