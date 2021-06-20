//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2016-2018 César Nicolás González (@CNGSoft)
//  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------------------------------

// ** reescrito en C (antes Pascal) por CNGSOFT el Domingo 7 de Agosto de 2016 por la noche **
// 
// Information about headers
// AMSDOS - 128-BYTE HEADER
// 	16-BIT CHECKSUM [0..$42] = W[$43]
// 	TYPE = B[$12]
// 	LOAD = W[$15]
// 	SIZE = W[$18], W[$40]
// 	BOOT = W[$1A]
// PLUS3DOS - 128-BYTE HEADER
// 	8-BIT CHECKSUM [0..$7E] = [$7F]
// 	SIZE = W[$10], W[$14]
// 
// AMSTRAD - 28-BYTE HEADER (FLAG $2C)
// 	CHAR[16] : FILENAME PADDED WITH $00
// 	BYTE : BLOCK.ID (1,2,3...)
// 	BYTE : LASTBLOCK?$FF:$00
// 	BYTE : TYPE
//  	WORD : BLOCK SIZE ($0800)
// 	WORD : START+PAST BLOCKS
// 	BYTE : FIRSTBLOCK?$FF:$00
// 	WORD : LENGTH
// 	WORD : BOOT
// SPECTRUM - 17-BYTE HEADER (FLAG $00)
// 	BYTE : TYPE
// 	CHAR[10] : FILENAME PADDED WITH $20
// 	WORD : START
// 	WORD : LENGTH
// 	WORD : LENGTH (?)
// */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinytape.h"

unsigned char body[1<<16],head[1<<8];
FILE *fi,*fo;
char *si=0,*so=0,*sn=0,*mode=0;

int flag_n=0,flag_b=1000,flag_bb,flag_m=0,flag_i=255,flag_o=4096,flag_t=0,flag_h=2560,flag_p=10240,flag_z=4;
int filetype=0,filesize=0,fileload=-1,fileboot=-1,pulselength=-1,bitspersec=-1,bytesperpage=-1,bitsize,detectedHeader=0;

int i,j,k,status=0;

typedef enum filetypes {
		FT_Basic = 0
	,	FT_protected = 1
	, 	FT_binary = 2
	,  FT_ascii = 0x16
	,  FT_none
} TFiletype;

typedef enum tapemodes {
		TM_cpc 		= 0
	,  TM_cpcraw 	= 1
	,	TM_zx			= 2
	,  TM_zxraw    = 3
	,  TM_raw1full = 4
	,  TM_raw1half = 5
	,  TM_raw2full = 6
	,  TM_raw2half = 7
	,  TM_cpctxt   = 8

	,  TM_ntotalModes
} TTapeMode;


void create11(TFiletype filetype_override) {
	memset(head,0,16);
	if (sn)
		strcpy((char*)head,sn);
	if (filetype_override == FT_none)
		head[0x12]=filetype;
	else
		head[0x12]=filetype_override;
	head[0x18]=filesize;
	head[0x19]=filesize>>8;
	head[0x1A]=fileboot;
	head[0x1B]=fileboot>>8;
}

void update11(int n,int is1st,int islast,int l) {
	head[0x10]=n;
	head[0x11]=islast?-1:0;
	head[0x13]=l;
	head[0x14]=l>>8;
	head[0x15]=fileload;
	head[0x16]=fileload>>8;
	head[0x17]=is1st?-1:0;
}

#define fputcc(x,y) { fputc((x),y); fputc((x)>>8,y); }
#define fputccc(x,y) { fputc((x),y); fputc((x)>>8,y); fputc((x)>>16,y); }
void record10(unsigned char *t,int first,int l,int p)
{
	fputc(0x10,fo);
	fputcc(p,fo);
	fputcc(l+2,fo);
	fputc(first,fo);
	fwrite(t,1,l,fo);
	int i=first,j=0;
	while (j<l)
		i^=t[j++];
	fputc(i,fo);
}
void record11(unsigned char *t,int first,int l,int p)
{
	fputc(0x11,fo);
	fputcc(flag_bb,fo);
	fputcc(flag_b,fo);
	fputcc(flag_b,fo);
	fputcc(flag_b,fo);
	fputcc(flag_bb,fo);
	fputcc(flag_o,fo);
	fputc(8,fo);
	fputcc(p,fo);
	p=1+(((l+255)/256)*258)+flag_z;
	fputccc(p,fo);
	fputc(first,fo);
	p=0;
	while (l>0)
	{
		fwrite(t+p,1,256,fo);
		int crc16=0xFFFF;
		first=256; while (first--) // early CRC-16-CCITT as used by Amstrad
		{
			int xor8=(t[p++]<<8)+1;
			while (xor8&0xFF)
			{
				if ((xor8^crc16)&0x8000)
					crc16=((crc16^0x0810)<<1)+1;
				else
					crc16<<=1;
				xor8<<=1;
			}
		}
		crc16=~crc16;
		fputc(crc16>>8,fo); // HI FIRST,
		fputc(crc16,fo); // AND LO NEXT!
		l-=256;
	}
	l=flag_z;
	while (l--)
		fputc(255,fo);
}

void usage() {
	error(1, "Usage: CPC2CDT [option..] infile outfile"
		"\n   -r  FILE name to record on tape, unnamed file if missing"
		"\n   -t       record CPC file as a standard 2k block and a giant block"
		"\n   -m  N    mode: one of these { cpc cpcraw cpctxt zx zxraw raw1full raw1half raw2full raw2half }"
		"\n             / cpc:      Standard CPC File (basic/binary) with/adding AMSDOS header."
		"\n             | cpctxt:   ASCII text file with CRLF line endings, adding AMSDOS header."
		"\n    CPC2CDT  | cpcraw:   CPC File without AMSDOS header."
		"\n      MODES  | zx:       Standard ZX spectrum file with PLUS3DOS header."
		"\n             \\ zxraw:    ZX spectrum file without PLUS3DOS header."
		"\n             / raw1full: RAW data codified as 1 bit per each full pulse (2 pulses) (standard)"
		"\n       TINY  | raw1half: RAW data codified as 1 bit per each half pulse (1 pulse)"
		"\n      MODES  | raw2full: RAW data codified as 2 bits per each full pulse (2 pulses)"
		"\n             \\ raw2half: RAW data codified as 2 bits per each half pulse (1 pulse)"
		"\n   -b  N    baud rate for CPC blocks (1000)"
		"\n   -i  N    ID byte for raw blocks (Default -1 = No ID)"
		"\n   -o  N    number of pilot pulses for CPC blocks (Default 4096)"
		"\n   -z  N    length of CPC block trailing tone in bytes (Default 4)"
		"\n   -h  N    pause between CPC file blocks, in milliseconds (Default 2560)"
		"\n   -p  N    pause after the final block, in milliseconds (Default 10240)"
		"\n   -l  N    load address (Default 16384)"
		"\n   -x  N    run/execute address (Default 16384)"
		"\n"
		"\nSPECIFIC OPTIONS FOR TINY MODES"
		"\n   -rl N    length of a single pulse (half) in Ts (1T=1/3500000s)"
		"\n   -rb N    cadence in bits per second (ignored if -rp is set)"
		"\n   -rp N    bytes per page. It adds an extra byte afer each page (0xFC/0xFE), for counter-enabled loaders"
		"\n"
	);
}

int hexstr2int(const char* value, int max) {
	int v = 0;
	const char* val = value + 2; // Jump over hex prefix
	while (*val) {
		v *= 16;
		if (*val >= '0' && *val <= '9') {
			v += *val - '0';
		} else if (*val >= 'A' && *val <= 'F') {
			v += *val - 'A' + 10;
		} else if (*val >= 'a' && *val <= 'f') {
			v += *val - 'a' + 10;
		} else {
			error(2, "ERROR: Incorrectly formatted hexadecimal value '%s'\n", value);
		}
		++val;
	}
	// Check maximum 
	if (v > max) 
		error(2, "ERROR: Hexadecimal value out of range '%s' (max: '0x%x')\n", value, max);

	return v;
}

void tolowerstr(char* str) {
	while (*str) {
		if (*str >= 'A' && *str <= 'Z')
			*str = *str - 'A' + 'a';
		++str;
	}
}

int str2int(const char* value, int max) {
	int v = atoi(value);
	if (v > max)
		error(2, "ERROR: Value out of range '%s' (max: '%d')\n", value, max);
	return v;
}

int hasHexPrefix(const char* value) {
	return strlen(value) > 2 && value[0]=='0' && value[1]=='x';
}

int isDecimal(const char* value) {
	while(*value) {
		if (*value < '0' || *value > '9')
			return 0;
		++value;
	}
	return 1;
}

int get16bitValue(const char* value) {
	if ( hasHexPrefix(value) )
		return hexstr2int(value, 0xFFFF);
	else if ( isDecimal(value) )
		return str2int(value, 0xFFFF);

	error(3, "ERROR: Expected decimal/hexadecimal number but found '%s'\n", value);
}

TTapeMode str2conversionMode(char *_mode) {
	static const unsigned char nmodes = TM_ntotalModes;
	static const char* modes[] = 
	{ 			"cpc"			, "cpcraw"
			, 	"zx"			, "zxraw"
			, 	"raw1full"	, "raw1half"
			, 	"raw2full"	, "raw2half"
			, 	"cpctxt" 
	};
	mode = _mode;
	tolowerstr(mode);
	for (unsigned char i=0; i < nmodes; ++i) {
		if ( !strcmp(mode, modes[i]) )
			return i;
	}
	error(3, "ERROR: Expected mode but found '%s'. Valid modes are: { %s, %s, %s, %s, %s, %s, %s, %s, %s }\n", mode, modes[0], modes[1], modes[2], modes[3], modes[4], modes[5], modes[6], modes[7], modes[8]);
}

void parseCommandLineArgs(int argc, char *argv[]) {
// Macros for clarity
#define flag(F)              !strcmp( (F) , argv[i] )
#define nextArg(F)           if (i+1<argc) ++i; else error(4, "ERROR: Flag '%s' requires a value\n", (F));
#define get16bitFlag(VAR, F) nextArg((F)); (VAR)=get16bitValue(argv[i]);
#define getStrMode(VAR, F) 	 nextArg((F)); (VAR)=str2conversionMode(argv[i]);
#define getStrFlag(VAR, F) 	 nextArg((F)); (VAR)=argv[i];
	
	for(int i=1; i < argc; ++i) {
		if      ( flag("-b")  ) { get16bitFlag(flag_b, "-b");   }
		else if ( flag("-m")  ) { getStrMode(flag_m, "-m");     }
		else if ( flag("-i")  ) { get16bitFlag(flag_i, "-i");   }
		else if ( flag("-o")  ) { get16bitFlag(flag_o, "-o");   }
		else if ( flag("-t")  ) { flag_t=1;                     }
		else if ( flag("-r")  ) { getStrFlag(sn, "-r");         }
		else if ( flag("-h")  ) { get16bitFlag(flag_h, "-h");   }
		else if ( flag("-p")  ) { get16bitFlag(flag_p, "-p");   }
		else if ( flag("-z")  ) { get16bitFlag(flag_z, "-z");   }
		else if ( flag("-l")  ) { get16bitFlag(fileload, "-l"); }
		else if ( flag("-x")  ) { get16bitFlag(fileboot, "-x"); }
		else if ( flag("-rl") ) { get16bitFlag(pulselength, "-rl"); }
		else if ( flag("-rb") ) { get16bitFlag(bitspersec, "-rb"); }
		else if ( flag("-rp") ) { get16bitFlag(bytesperpage, "-rp"); }
		else if ( !si ) si=argv[i];
		else if ( !so ) so=argv[i];
		else error(5, "ERROR: Unexpected parameter '%s'\n", argv[i]);
	}	

// Undef clarity macros
#undef flag
#undef get16bitFlag
#undef getStrFlag
#undef getStrMode
#undef nextArg
}

void detectHeaderInInputFile() {
	// Open and process input file
	if ( !(fi=fopen(si,"rb")) )
		error(1, "ERROR: cannot open source file '%s'!\n", si);

	// READ HEADER AND DETECT FORMAT
	memset(head,0,1<<8);
	memset(body,0,1<<16);
	filesize=fread(body,1,128,fi);
	if (filesize==128) {
		// Prevent detection of sequence of 128 zeros as an AMSDOS header
		j=0;
		for (i=0; i < 128; ++i) 
			j += body[i];
		// j must be not 0 for this to be recognized as a valid AMSDOS header
		if ( j != 0 ) {
			i=j=0;
			while (i<0x43)
				j+=body[i++];
			if ((body[0x43]+256*body[0x44])==j) { // AMSDOS!
				filetype=body[0x12];
				filesize=body[0x40]+body[0x41]*256;
				fileload=body[0x15]+body[0x16]*256;
				fileboot=body[0x1A]+body[0x1B]*256;
				if (fread(body,1,filesize,fi)<1) error(1, "ERROR: short read while reading AMSDOS header");
				detectedHeader = 1;
			} else {
				while (i<0x7F)
					j+=body[i++];
				if ((j&0xFF)==body[0x7F]) { // PLUS3DOS!
					filetype=body[0x0F];
					filesize=body[0x10]+body[0x11]*256;
					fileload=body[0x12]+body[0x13]*256;
					fileboot=body[0x14]+body[0x15]*256;
					if (fread(body,1,filesize,fi)<1) error(1, "ERROR: short read while reading PLUS3DOS header");
					detectedHeader = 1;
				}
			}
		}
	}
	// If not AMSDOS, nor PLUS3DOS, consider it RAW binary
	if ( !detectedHeader ) {
		filesize=128+fread(body+128,1,(1<<16)-128,fi);
		filetype=FT_binary;
		if (fileboot < 0) fileboot = 0x4000;
		if (fileload < 0) fileload = 0x4000;
	}

	fclose(fi);
}

///
/// Writes a basic/binary or ascii file to the tape
/// using CPC firmware blocks
///
void writeCPCFile(TTapeMode mode) {
	// Set correct filetype
	TFiletype ft = FT_none;
	if (mode == TM_cpctxt) ft = FT_ascii;
	
	// Create header
	create11(ft);
	
	// Create tape blocks
	if (filesize>0x800) {
		update11(j=1,1,0,0x800); // FIRST BLOCK
		record11(head,44,28,16);
		record11(body,22,0x800,flag_h);
		k=filesize-0x800;
		i=0x800;
		if (!flag_t)
			while (k>0x800)
			{
				fileload+=0x800;
				update11(++j,0,0,0x800); // MID BLOCK
				record11(head,44,28,16);
				record11(body+i,22,0x800,flag_h);
				k-=0x800;
				i+=0x800;
			}
		fileload+=0x800;
		update11(++j,0,1,k); // LAST BLOCK
		record11(head,44,28,16);
		record11(body+i,22,k,flag_p);
	}
	else
	{
		update11(1,1,1,filesize); // SINGLE BLOCK
		record11(head,44,28,16);
		record11(body,22,filesize,flag_p);
	}
}

void cpc2cdt_modes() {
	// Set up bauds
	if (flag_b>0)
		flag_b=(3500000/3+flag_b/2)/flag_b;
	else
		flag_b=-flag_b;
	flag_bb=flag_b*2;

	// Open and process output file (CDT)
	if ( (fo=fopen(so,"rb")) ) {
		// File already exists. Open for append mode
		fclose(fo);
		if ( !(fo=fopen(so,"ab")) )
			error(1, "ERROR: cannot open output file '%s' for writing!\n", so);
	} else {
		// File does not exist, create a new one
		if ( !(fo=fopen(so,"wb")) )
			error(1, "ERROR: cannot create new output file '%s'!\n", so);
		fwrite("ZXTape!\032\001\000\040\000\012",1,13,fo);
	}

	// Process file depending on selected mode
	switch (flag_m)
	{
		//----------------------------------------------------------
		case TM_cpc:
		case TM_cpctxt:
			writeCPCFile(flag_m);
			break;
		//----------------------------------------------------------
		case TM_cpcraw:
			record11(body,flag_i,filesize,flag_p);
			break;
		//----------------------------------------------------------
		case TM_zx:
			head[0]=filetype;
			memset(head+1,32,10);
			if (sn)
			{
				i=0;
				while (sn[i])
				{
					head[i+1]=sn[i];
					i++;
				}
			}
			//strcpy(head,sn);
			head[11]=filesize;
			head[12]=filesize>>8;
			head[13]=fileload;
			head[14]=fileload>>8;
			head[15]=fileboot;
			head[16]=fileboot>>8;
			record10(head,0,17,1000);
			record10(body,255,filesize,flag_p);
			break;
		//----------------------------------------------------------
		case TM_zxraw:
			record10(body,flag_i,filesize,flag_p);
			break;
	}
	fclose(fo);
}


int main(int argc,char *argv[])
{
	// Parse arguments
	parseCommandLineArgs(argc, argv);
	if (status||!so)
		usage();
	
	// Detect header in input file
	detectHeaderInInputFile();

	// Process depending on mode
	switch (flag_m) {
		case TM_cpc:
		case TM_cpctxt:
		case TM_cpcraw:
		case TM_zxraw:
		case TM_zx: {
			// CPC2CDT Modes
			cpc2cdt_modes();
			break;
		}
		default: {
			// TINYTAPE Modes
			
			// Convert ID value to negative if it has not been set
			if (flag_i == 255) flag_i=-1;
	
			// Pulselength or bitspersec must be set for tiny tape
			if 		(pulselength > 0) bitsize = -pulselength;
			else if (bitspersec  > 0) bitsize = bitspersec;
			else	error(6, "ERROR: Mode '%s' requires '-rp' or '-rb' to be set.\n", mode);
	
			// Set Bitgaps
			// Bitgaps are the number of bytes per page. At the end of each page
			// a byte (0xFE or 0xFC) is written to act as counter, for counter-enabled loaders.
			// bitgaps = -1 by default, meaning that nothing should be written in-between pages
			tiny_tape_setBitGaps(bytesperpage);
	
			// If there is a header, be it AMSDOS or PLUS3DOS, skip it
			tiny_tape_setSkipHeader(detectedHeader);
			
			// Generate tape with Tiny Modes. Parameters:
			// 	 srcfile:  Source file (RAW/BIN)
			// 	 tzxfile:  Output file (if it exists, srcfile is appended, else tzxfile is created with srcfile)
			// 	 _bittype: 0 (1 bit, full pulse), 1 (1 bit, half pulse), 2 (2 bits, full pulse), 3 (2 bits, half pulse)
			// 	 _bitsize: (<0) Pulse Lenght in Ts, (>0) Bits per second
			// 	 _bitbyte: Data block ID (first byte at the start of the blogk). (-1) for no block ID
			// 	 _bithold: Pause in milliseconds
			//void tiny_tape_gen(	const char* srcfile, const char* tzxfile, int _bittype
			//				, int _bitsize, int _bitbyte, int _bithold) {
			tiny_tape_gen(si, so, flag_m-4, bitsize, flag_i, flag_h);
		}
	}

 	return 0;
}
