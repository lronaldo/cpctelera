/*
D88 (D68/D77/D98) File Structure
================================

Multiple disk image:
====================
Simple concatenation of multiple individual disks.


Disk structure:
===============
Header
Track 0
Track 1
Track 2
Track 3
...


Size: 688 or 672 bytes.   

Offset    Size             Meaning
------------------------------------------------------------------------------------ 

00h       16(CHAR)             Name/Comment.
0Ah       10                   Reserved (00h).
1Ah       1                    Write protect flag. 00h = normal. 10h = write protected.
1Bh       1                    Media flag. 00h = 2D, 10h = 2DD, 20h = 2HD.
1Ch       4(DWORD)             Disk size (including header size).
20h       4(DOWRD) * 164       Track table. Offset of each track from disk start.
                               0 if track is not recorded in image.
                               *Alternative header format only has 160 track table entries.

-> End of disk image if disk is unformatted (header only).


Data:
=====
Offset: 2A0h or 2B0h.
Size: variable.
Concatenation of all tracks in disk (see track table for offsets).


Track:
======
Size: variable.
Concatenation of all sectors in track in their original order (possible 
interleaving).


Sector:
=======
Size: variable.
Sector header + sector data.

Offset    Size          Meaning
------------------------------------------------------------------------------------------ 

00h       1                 C (cylinder/track value of sector's id).
01h       1                 H (head value of sector's id).
02h       1                 R (record/number value of sector's id).
03h       1                 N (sector length in IBM format. Bytes in sector = 128 * (1 << N)).
04h       2(WORD)           Number of sectors in track.
06h       1                 Density. 00h double density. 40h = single density.
07h       1                 Deleted data (DDAM). 0 = normal, 1 = deleted.
08h       1                 Status (error code returned by disk bios). 00h = normal.
09h       5                 Reserved (00h).
0Eh       2(WORD)    Sector length in bytes. (-> end sector header) 
01h       (variable)    Sector data. See sector length field for size.

*/




#pragma pack(1)

typedef struct d88_fileheader_
{
	unsigned char name[16];
	unsigned char reserved[10];
	unsigned char write_protect;
	unsigned char media_flag;
	unsigned long file_size;
}d88_fileheader;

typedef struct d88_sector_
{
	unsigned char  cylinder;
	unsigned char  head;
	unsigned char  sector_id;
	unsigned char  sector_size;
	unsigned short number_of_sectors;
	unsigned char  density;
	unsigned char  deleted_data;
	unsigned char  sector_status;
	unsigned char  reserved[5];
	unsigned short sector_length;
}d88_sector;

#pragma pack()
