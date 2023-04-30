/*
//
// Copyright (C) 2006, 2007, 2008 Jean-François DEL NERO
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

#define byte  unsigned char
#define word  unsigned short
#define dword unsigned int

#define STRINGTAGSIZE 16
#define AFI_CRCSIZE 2

/////////////////////////////////////////////////////
//Block tag
#define AFI_IMG_TAG			"AFI_FLOPPY_IMG"
#define AFI_DATA_TAG		"TRACKDATA"
#define AFI_INFO_TAG		"AFI_INFO"
#define AFI_STRING_TAG		"STRING"
#define AFI_TRACKLIST_TAG	"TRACKLIST"
#define AFI_TRACK_TAG		"TRACK"

/////////////////////////////////////////////////////
//  file header
typedef struct AFIIMG_
{	
	byte  afi_img_tag[STRINGTAGSIZE];
        
	byte  version_code_major;
    byte  version_code_minor;
        
	dword header_size;
	
	dword floppyinfo_offset;
	dword track_list_offset;

	word header_crc;
}AFIIMG;

/////////////////////////////////////////////////////
//  file info struct
typedef struct AFIIMGINFO_
{	
	byte  afi_img_infos_tag[STRINGTAGSIZE];
        
	dword  floppy_info_size;

    dword  mediatype_code;
    dword  platformtype_code;
        
	dword  total_track;
	dword  start_track;
	dword    end_track;
	dword  start_side;
	dword    end_side;

	dword number_of_string;
}AFIIMGINFO;
//dword string_info_list; //*number_of_string
//word info_crc;

//
// media type enum
enum {
	AFI_MEDIA_UNKNOW,
	AFI_MEDIA_3P50_DD,
	AFI_MEDIA_3P50_HD,
	AFI_MEDIA_3P50_ED,
	AFI_MEDIA_5P25,
	AFI_MEDIA_3P00
};

// platform type enum
enum {
	AFI_PLATFORM_UNKNOW,
	AFI_PLATFORM_ATARI_ST,
	AFI_PLATFORM_AMIGA,
	AFI_PLATFORM_PC,
	AFI_PLATFORM_CPC,
	AFI_PLATFORM_MSX2
};

/////////////////////////////////////////////////////
//  string info struct
typedef struct AFI_STRING_
{	
	byte  afi_string_tag[STRINGTAGSIZE];
	byte  afi_string_type_tag[STRINGTAGSIZE];
	dword  string_size;
}AFI_STRING;
//byte  afi_string;//*string_size
//word string_crc;

//string type tag
#define AFI_FLOPPY_TITLE  "DISC_TITLE"
#define AFI_FLOPPY_AUTHOR "DISC_AUTHOR"
#define AFI_FLOPPY_RELEASE_DATE "RELEASE_DATE"
#define AFI_FLOPPY_FILECREATIONDATE "FILECREATION_DATE"
#define AFI_FLOPPY_MEDIATYPE "MEDIA_TYPE"
#define AFI_FLOPPY_PLATFORMTYPE "PLATFORM_TYPE"

/////////////////////////////////////////////////////
// track list 
typedef struct AFITRACKLIST_
{	
	byte  afi_img_track_list_tag[STRINGTAGSIZE];
	dword number_of_track;
}AFITRACKLIST;
//	dword track_offset_list; //*number_of_track
//	word  tracklist_crc;

//
/////////////////////////////////////////////////////
// track
typedef struct AFITRACK_
{	
	byte  afi_track_tag[STRINGTAGSIZE];

	dword track_number;
	dword side_number;

	dword encoding_mode;
	dword nb_of_element;

	dword number_of_data_chunk;
}AFITRACK;
//	dword data_offset_list; //*number_of_data_chunk
//	word  afi_track_crc;

// encoding mode
enum {
	AFI_TRACKENCODING_NONE,
	AFI_TRACKENCODING_MFM,
	AFI_TRACKENCODING_PDC
};
//AFI_TRACKENCODING_MFM -> AFI_DATA_INDEX
//                         AFI_DATA_MFM
//                         AFI_DATA_BITRATE
//                         AFI_DATA_WEAKBITS (optional)

//AFI_TRACKENCODING_PDC -> AFI_DATA_INDEX
//                         AFI_DATA_PDC
//                         AFI_DATA_WEAKBITS (optional)

/////////////////////////////////////////////////////
// data
typedef struct AFIDATA_
{	
	byte  afi_data_tag[STRINGTAGSIZE];

	dword TYPEIDCODE;
	byte  type_tag[STRINGTAGSIZE];

	dword nb_bits_per_element;

	dword packed_size;
	dword packer_id;
	dword unpacked_size;

}AFIDATA;
//	byte data;//*packed_size
//	word  afi_data_crc;


// data type 
enum {
	AFI_DATA_NONE,
	AFI_DATA_MFM,
	AFI_DATA_INDEX,
	AFI_DATA_BITRATE,
	AFI_DATA_PDC,
	AFI_DATA_WEAKBITS
};

#define AFI_DATA_TYPE_MFM      "MFM_DATA"
#define AFI_DATA_TYPE_INDEX    "INDEX_DATA"
#define AFI_DATA_TYPE_BITRATE  "BITRATE_DATA"
#define AFI_DATA_TYPE_PDC      "PDC_DATA"
#define AFI_DATA_TYPE_WEAKBITS "WEAKBITS_DATA"

typedef struct AFI_DATACODE_
{	
	byte  idcode;        
	char * idcodetag;
}AFI_DATACODE;


// data packer type
enum {
	AFI_COMPRESS_NONE,
	AFI_COMPRESS_GZIP,
	AFI_COMPRESS_RLE,	// RFU
	AFI_COMPRESS_LZW    // RFU
};



int write_AFI_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename);

#pragma pack()

