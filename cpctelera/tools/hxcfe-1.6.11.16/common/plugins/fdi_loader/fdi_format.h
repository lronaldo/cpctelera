/*
FDI Format
Supported by various emulators, including SPIN. Note that this entry was translated from UKV Spectrum Debugger's documentation by Mac Buster.

   Offset   Field size    Description  

    0x00       0x03       Text "FDI"
    0x03       0x01       Write protection (0: write enabled; 1: disabled)
    0x04       0x02       Number of cylinders
    0x06       0x02       Number of heads
    0x08       0x02       Offset of disk description
    0x0A       0x02       Data offset
    0x0C       0x02       Length of additional information in header. (UKV
                          uses 00)
    0x0E       (x)        Additional information; length is specified in the
                          previous field.
    0x0E+(x)              Track headers area. This section contains all
                          information on the disk format. This area must
                          contain at least "Number of cylinders"*"Number of
                          heads" headers. The headers are stored in the
                          sequence Cyl 0 Head 0; Cyl 0 Head 1; Cyl 1 Head 0
                          etc.
   (Description offset)   A description of the disk terminated with 0x00 can
                          be placed here; the MAKEFDI utility (supplied by
                          UKV) allows for up to 64 characters, including
                          the terminating null.
   (Data offset)          The actual disk data. The size and sequence depends
                          on the disk format.

Track Header Format:

   Offset   Field size    Description

    0x00       0x04       Track offset: the offset of the track data, relative
                          to the data offset defined above.
    0x04       0x02       (Always 0x0000)
    0x06       0x01       Number of sectors on this track.
    0x07   (Sectors*7)    Sector infomation:
                          Bytes 00-03 give the cylinder number, head number,
			  sector size (00: 128 bytes; 01: 256; 02: 512; 03:
                          1024; 04: 4096) and sector number respectively.
                          Byte 04 contains the flags:
   		            Bits 0-5 are CRC marks: if the CRC was correct for
   			    a sector size 128,256,512,1024 or 4096, then the
			    respective bit will be set. If all bits are 0 then
                            there was a CRC error when this sector was
                            written.
		            Bit 6 is always 0.
		            Bit 7 is 0 for normal data, or 1 for deleted data.
		          Bytes 05-06 give the sector offset, relative to
		          (data offset+track offset)			 
   7*(Sectors+1)          Track header length

Note that UKV 1.2 does not use the flag byte.
*/

#pragma pack(1)

typedef struct fdi_header_
{
	unsigned char  signature[3];
	unsigned char  write_protect;
	unsigned short number_of_cylinders;
	unsigned short number_of_heads;
	unsigned short diskdescription_offset;
	unsigned short data_offset;
	unsigned short additionnal_infos_len;
}fdi_header;


typedef struct fdi_track_header_
{
	unsigned long  track_offset;
	unsigned short unused;
	unsigned char  number_of_sectors;
}fdi_track_header;

typedef struct fdi_sector_header_
{
	unsigned char  cylinder_number;
	unsigned char  head_number;
	unsigned char  sector_number;
	unsigned char  sector_size;
	unsigned char  flags;
	unsigned short sector_offset;
}fdi_sector_header;

#pragma pack()
