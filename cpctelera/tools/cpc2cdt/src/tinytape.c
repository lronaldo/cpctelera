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
//**********************************
//
//Record a raw or AMSDOS binary file
//into a TZX/CDT file using a signal
//that encodes single or double bits
//into either half or full pulses.
//
//**********************************

#include <stdio.h>
#include <stdlib.h>
#include "tinytape.h"

FILE *fi,*fo;
int bittype,bitsize,bithold,bitbyte,bitgaps=-1,bit_tzx;
int databytes,lastbyte,polarity,skipHeader=0;
unsigned char databyte[1<<19];

#define fputcc(i,f); { fputc((i)&255,f); fputc((i)>>8,f); }

void write_sample(int i)
{
	if (lastbyte>=256)
	{
		databyte[databytes++]=lastbyte;
		lastbyte=1;
	}
	lastbyte=lastbyte*2+i;
}
void repeat_write() { write_sample(polarity); }
void toggle_write() { write_sample(polarity=1-polarity); }

void write_byte(int i)
{
	if (bit_tzx)
		databyte[databytes++]=i;
	else
	{
		int j,k,l;
		switch (bittype&3)
		{
			case 0: // 2B: single full bit (100%)
				j=256;
				while (j>>=1)
				{
					toggle_write();
					if (i&j)
					{
						repeat_write();
						toggle_write();
						repeat_write();
					}
					else
						toggle_write();
				}
				break;
			case 1: // 4B: double full bit (120%)
				j=768; k=8;
				while (j>>=2)
				{
					l=(i&j)>>(k-=2);
					toggle_write();
					if (l--) { repeat_write(); if (l--) { repeat_write(); if (l--) repeat_write(); } } // while (l--) repeat_write();
					l=(i&j)>>k;
					toggle_write();
					if (l--) { repeat_write(); if (l--) { repeat_write(); if (l--) repeat_write(); } } // while (l--) repeat_write();
				}
				break;
			case 2: // 2A: single half bit (200%)
				j=256;
				while (j>>=1)
				{
					toggle_write();
					if (i&j)
						repeat_write();
				}
				break;
			case 3: // 4A: double half bit (240%)
				j=768; k=8;
				while (j>>=2)
				{
					l=(i&j)>>(k-=2);
					toggle_write();
					if (l--) { repeat_write(); if (l--) { repeat_write(); if (l--) repeat_write(); } } // while (l--) repeat_write();
				}
				break;
		}
	}
}

int bip8,gaps;
void creatblock(int j)
{
	int i;
	gaps=bitgaps; lastbyte=1; databytes=polarity=bip8=0; // BIP-8 // 20160605: bittype&8=XOR8!
	if (!bit_tzx)
	{
		for (i=1;i<j;i++)
			write_byte(0xFF); // 0xFF is safe, (bittype&1)?0xAA:0xFF isn't
		write_byte((bittype&1)?0xFC:0xFE); // 0xFC is safe, 0xA8 isn't either!
	}
}
void char2block(int i)
{
	bip8^=i;
	write_byte(bittype&8?bip8:i);
	if (bittype&8) bip8=i;
	if (!(--gaps))
	{
		write_byte((bittype&1)?0xFC:0xFE);
		gaps=bitgaps;
	}
}
void closeblock(int h)
{
	write_byte(0xFF^bip8); // ~BIP-8
	int lastbits=8;
	if (bit_tzx)
	{
		fputc(0x11,fo);
		fputcc(bitsize*2,fo);
		fputcc(bitsize,fo);
		fputcc(bitsize,fo);
		fputcc(bitsize,fo);
		fputcc(bitsize*2,fo);
		fputcc(256*16-2-1,fo);
		fputc(lastbits,fo);
		fputcc(h,fo);
	}
	else
	{
		toggle_write(); // EOF!
		while (lastbyte<256)
		{
			lastbits--;
			lastbyte<<=1;
		}
		databyte[databytes++]=lastbyte;
		fputc(0x15,fo);
		fputcc(bitsize,fo);
		fputcc(h,fo);
		fputc(lastbits,fo);
	}
	fputcc(databytes,fo);
	fputc(databytes>>16,fo);
	fwrite(databyte,1,databytes,fo);
	printf("%i bytes.\n",databytes);
}

void tiny_tape_usage() {
	error(1, "TINYTAPE [+]SOURCE.RAW|BIN [+]TARGET.TZX|CDT <0-3> [-]SAMPLE_H <0..255|-1> PAUSE_MS [PAGE_LEN]");
}

// Bitgaps are the number of bytes per page. At the end of each page
// a byte (0xFE or 0xFC) is written to act as counter, for counter-enabled loaders.
// bitgaps = -1 by default, meaning that nothing should be written in-between pages
void tiny_tape_setBitGaps(int bg) { bitgaps = bg; }

// Set for skipping the 128 byte header if the srcfile has
// skipHeader = 0 by default, meaning no header has to be skipped (srcfile has no header)
void tiny_tape_setSkipHeader(int sk) { skipHeader = sk; } 

// srcfile:  Source file (RAW/BIN)
// tzxfile:  Output file (if it exists, srcfile is appended, else tzxfile is created with srcfile)
// _bittype: 0 (1 bit, full pulse), 1 (1 bit, half pulse), 2 (2 bits, full pulse), 3 (2 bits, half pulse)
// _bitsize: (<0) Pulse Lenght in Ts (1T=1/3500000s), (>0) Bits per second
// _bitbyte: Data block ID (first byte at the start of the blogk). (-1) for no block ID
// _bithold: Pause in milliseconds
void tiny_tape_gen(	const char* srcfile, const char* tzxfile, int _bittype
					, int _bitsize, int _bitbyte, int _bithold) {
	int i;
	bittype = _bittype;
	bitsize = _bitsize;
	bitbyte = _bitbyte;
	bithold = _bithold;
		
	// Open Source File
	if ( !(fi=fopen(srcfile,"rb")) )
		error(2, "ERROR: cannot open source file '%s'\n", srcfile);
	
	// Calculate blocksize
	fseek(fi,0L,SEEK_END);
	int blocksize=ftell(fi);

	// Check if we have to skip header or not, and recalculate block size
	if (skipHeader) skipHeader = 128L;
	fseek(fi,skipHeader,SEEK_SET);
	blocksize-=ftell(fi);

	// Open tzxfile (if file already exist, then add, else create new)
	if ( (fo = fopen(tzxfile,"rb")) ) {
		// File already exists
		fclose(fo);
		fo=fopen(tzxfile,"ab");
	} else {
		// File does not exist
		fo=fopen(tzxfile,"wb");
		if (fo)
			fwrite("ZXTape!\032\001\000",1,10,fo);		
	}
	if (!fo)
		error(3, "ERROR: cannot open target file '%s'\n", tzxfile);

	// Output codification

	// Set Bittzx
	bit_tzx=!(bittype&7); // "4" rather than "0" forces sub-optimal encoding

	// If we have a ID, add 1 byte for it to the blocksize
	if ((bitbyte>=0)&&(bitbyte<256)) 
		++blocksize;

	// Fix bitsize and create block
	if (bitsize<0) bitsize=-bitsize;
	else           bitsize=3500000/bitsize;
	creatblock(256);

	// Add the ID, if there is one
	if ((bitbyte>=0)&&(bitbyte<256))
		char2block(bitbyte);

	// Add contents
	while ((i=fgetc(fi))!=EOF)
		char2block(i);
	closeblock(bithold);
	
	// Close files
	fclose(fi);
	fclose(fo);
}
