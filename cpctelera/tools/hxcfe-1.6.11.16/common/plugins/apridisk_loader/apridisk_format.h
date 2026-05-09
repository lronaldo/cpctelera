/*
ApriDisk file format
Version History

    * 7 December 2005: Version 1.1. Added the creator block, which has shown up in some Apridisk files.
    * 18 January 2005: Version 1.0. Updated with corrections from Jonathan Marsters (the author of Apridisk).
    * 13 February 2003: Version 0.1. Original reverse-engineered format definition.

Header

The header is 128 bytes long. It contains the C string

	"ACT Apricot disk image\032\004"

padded out to 128 bytes with zeros. LibDsk 1.1.3+ requires the presence of this header.

Data records

The records for each sector now follow. In existing examples they are sorted by sector, head, cylinder, but this is not necessary. 
LibDsk does not sort ApriDisk files it generates, though ApriDisk files created by dsktrans will be in the expected order.

A data record header is at least 16 bytes long. It is formed:

	DD	item_type	;4 bytes, little-endian.
				;0xE31D0000 => deleted item
				;0xE31D0001 => sector
				;0xE31D0002 => comment
				;0xE31D0003 => creator
	DW	compression	;2 bytes, little-endian.
				;0x9E90 => not compressed
				;0x3E5A => compressed
	DW	header_size	;2 bytes, little-endian.
	DD	data_size	;4 bytes, little-endian. 
	DB	head		;Head ID, 0 or 1
	DB	sector		;Sector ID, 1 based
	DW	cylinder	;2 bytes, little-endian.
				;Cylinder ID, 0 based.

If the header_size is greater than 0x10, the remainder of the header then follows.

The data_size then gives the number of bytes to read. If the sector is not compressed, the bytes read will be the sector.

The Deleted Item tag allows in-place rewriting of Apridisk files; 
if a compressed sector has to be replaced with an uncompressed one, then the compressed sector is marked as deleted and the uncompressed one appended to the file. 
Currently LibDsk does not rewrite in place; it loads the file into memory and writes a new file on close. 
This means the DOS version can't handle 720k Apridisk images.

  
Compression
Compressed sectors use an RLE scheme:

	DW	count		;2 bytes, little-endian.
	DB	byte		;The byte to repeat count times. 

Compression only appears to be used in sectors all bytes of which are identical. 
LibDsk only generates a compressed sector if all bytes are the same.

  
	
Comment

After all the sectors, there may be a comment record. This is stored in the same way as a sector:

	DD	item_type	;4 bytes, little-endian.
				;0xE31D0000 => deleted item
				;0xE31D0001 => sector
				;0xE31D0002 => comment
				;0xE31D0003 => creator
	DW	compression	;2 bytes, little-endian.
				;0x9E90 => not compressed
				;0x3E5A => compressed
	DW	header_size	;2 bytes, little-endian.
	DD	data_size	;4 bytes, little-endian. 
	DB	0,0,0,0		;Comment ID, always 0.

The comment itself is stored as an ASCII string, complete with terminating 0 byte. 
Newlines in it are stored as '\r' characters.

  
Creator

Some APRIDISK images contain a record of a type I have called 'creator', which has the same format as a comment record except for the initial item type. 
This record, if present, tends to appear at the start of the file before any sectors.
*/


char APRIDISK_HeaderString[]="ACT Apricot disk image\032\004";

#pragma pack(1)

#define DATA_RECORD_DELETED      0xE31D0000 
#define DATA_RECORD_SECTOR       0xE31D0001 
#define DATA_RECORD_COMMENT      0xE31D0002 
#define DATA_RECORD_CREATOR      0xE31D0003 


#define DATA_NOT_COMPRESSED      0x9E90 
#define DATA_COMPRESSED      0x3E5A 


typedef struct apridisk_data_record_
{
	unsigned long  item_type;
	unsigned short compression;
	unsigned short header_size;
	unsigned long data_size;
	unsigned char head;
	unsigned char sector;
	unsigned short cylinder;
}apridisk_data_record;


typedef struct apridisk_compressed_data_
{
	unsigned short count;
	unsigned char  byte;
}apridisk_compressed_data;


#pragma pack()
