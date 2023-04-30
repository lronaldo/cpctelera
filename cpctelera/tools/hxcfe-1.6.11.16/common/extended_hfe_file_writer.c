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

#define FASTWRITE 1

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"

#include "extended_hfe_file_writer.h"

unsigned char * ramfile;
int ramfile_size;

unsigned char ext_bit_inverter[]=
{
        0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,
        0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
        0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,
        0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
        0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,
        0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
        0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,
        0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
        0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,
        0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
        0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,
        0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
        0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,
        0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
        0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,
        0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
        0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,
        0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
        0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,
        0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
        0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,
        0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
        0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,
        0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
        0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,
        0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
        0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,
        0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
        0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,
        0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
        0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,
        0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};


unsigned short ext_bit_expander[]=
{
	0x0000,0x0002,0x0008,0x000a,0x0020,0x0022,0x0028,0x002a,
	0x0080,0x0082,0x0088,0x008a,0x00a0,0x00a2,0x00a8,0x00aa,
	0x0200,0x0202,0x0208,0x020a,0x0220,0x0222,0x0228,0x022a,
	0x0280,0x0282,0x0288,0x028a,0x02a0,0x02a2,0x02a8,0x02aa,
	0x0800,0x0802,0x0808,0x080a,0x0820,0x0822,0x0828,0x082a,
	0x0880,0x0882,0x0888,0x088a,0x08a0,0x08a2,0x08a8,0x08aa,
	0x0a00,0x0a02,0x0a08,0x0a0a,0x0a20,0x0a22,0x0a28,0x0a2a,
	0x0a80,0x0a82,0x0a88,0x0a8a,0x0aa0,0x0aa2,0x0aa8,0x0aaa,
	0x2000,0x2002,0x2008,0x200a,0x2020,0x2022,0x2028,0x202a,
	0x2080,0x2082,0x2088,0x208a,0x20a0,0x20a2,0x20a8,0x20aa,
	0x2200,0x2202,0x2208,0x220a,0x2220,0x2222,0x2228,0x222a,
	0x2280,0x2282,0x2288,0x228a,0x22a0,0x22a2,0x22a8,0x22aa,
	0x2800,0x2802,0x2808,0x280a,0x2820,0x2822,0x2828,0x282a,
	0x2880,0x2882,0x2888,0x288a,0x28a0,0x28a2,0x28a8,0x28aa,
	0x2a00,0x2a02,0x2a08,0x2a0a,0x2a20,0x2a22,0x2a28,0x2a2a,
	0x2a80,0x2a82,0x2a88,0x2a8a,0x2aa0,0x2aa2,0x2aa8,0x2aaa,
	0x8000,0x8002,0x8008,0x800a,0x8020,0x8022,0x8028,0x802a,
	0x8080,0x8082,0x8088,0x808a,0x80a0,0x80a2,0x80a8,0x80aa,
	0x8200,0x8202,0x8208,0x820a,0x8220,0x8222,0x8228,0x822a,
	0x8280,0x8282,0x8288,0x828a,0x82a0,0x82a2,0x82a8,0x82aa,
	0x8800,0x8802,0x8808,0x880a,0x8820,0x8822,0x8828,0x882a,
	0x8880,0x8882,0x8888,0x888a,0x88a0,0x88a2,0x88a8,0x88aa,
	0x8a00,0x8a02,0x8a08,0x8a0a,0x8a20,0x8a22,0x8a28,0x8a2a,
	0x8a80,0x8a82,0x8a88,0x8a8a,0x8aa0,0x8aa2,0x8aa8,0x8aaa,
	0xa000,0xa002,0xa008,0xa00a,0xa020,0xa022,0xa028,0xa02a,
	0xa080,0xa082,0xa088,0xa08a,0xa0a0,0xa0a2,0xa0a8,0xa0aa,
	0xa200,0xa202,0xa208,0xa20a,0xa220,0xa222,0xa228,0xa22a,
	0xa280,0xa282,0xa288,0xa28a,0xa2a0,0xa2a2,0xa2a8,0xa2aa,
	0xa800,0xa802,0xa808,0xa80a,0xa820,0xa822,0xa828,0xa82a,
	0xa880,0xa882,0xa888,0xa88a,0xa8a0,0xa8a2,0xa8a8,0xa8aa,
	0xaa00,0xaa02,0xaa08,0xaa0a,0xaa20,0xaa22,0xaa28,0xaa2a,
	0xaa80,0xaa82,0xaa88,0xaa8a,0xaaa0,0xaaa2,0xaaa8,0xaaaa
};



void extaddpad(unsigned char * track,int mfmsize,int tracksize)
{
	int i,j;

	i=tracksize-1;
	do{
		
		i--;
	}while(i && !track[i]);

	if(i)
	{
		j=i+1;
/*		do
		{
			track[j++]=track[i-1];
			if(j<tracksize)
				track[j++]=track[i];
		}while(j<tracksize);*/
		
		do
		{
			if(j<tracksize)track[j++]=ext_bit_inverter[0x00];
			if(j<tracksize)track[j++]=ext_bit_inverter[0x06];
			if(j<tracksize)track[j++]=ext_bit_inverter[0xAA];
			if(j<tracksize)track[j++]=ext_bit_inverter[0xAA];
		}
		while(j<tracksize);

	}
}

#ifdef FASTWRITE
	FILE * extrfopen(char* fn,char * mode)
	{
		ramfile=0;
		ramfile_size=0;
		return (FILE *)1;
	};

	int extrfwrite(void * buffer,int size,int mul,FILE * file)
	{
		ramfile=realloc(ramfile,ramfile_size+size);
		memcpy(&ramfile[ramfile_size],buffer,size);
		ramfile_size=ramfile_size+size;
		return size;
	}

	int extrfclose(FILE *f)
	{

		if(ramfile)
			free(ramfile);
		return 0;
	};
#else
	#define extrfopen  fopen
	#define extrfwrite fwrite
	#define extrfclose fclose
#endif

int write_EXTHFE_file(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppy,char * filename,int forceifmode,int double_step)
{	

	picexttrack * track;

	FILE * hxcpicfile;

	picfileformatextheader * FILEHEADER;
	unsigned char * mfmtracks0,*mfmtracks1,*mfmtrackfinal,*mfmtemp;
	unsigned char * offsettrack;
	int mfmsize,mfmsize2;
	unsigned int i,j,k,l;
	unsigned int trackpos;
	unsigned int tracklistlen;
	unsigned int tracksize;
	unsigned char factor;

	factor=1;// factor=1-> 50% duty cycle  // factor=2-> 25% duty cycle 
	floppycontext->hxc_printf(MSG_INFO_1,"Write HFE file %s for the standalone emulator.",filename);

	ramfile=0;
	ramfile_size=0;
	
	hxcpicfile=extrfopen(filename,"wb");

	if(hxcpicfile)
	{
		FILEHEADER=(picfileformatextheader *) malloc(512);
		memset(FILEHEADER,0xFF,512);
		memcpy(&FILEHEADER->HEADERSIGNATURE,"HXCPICFE",8);

		FILEHEADER->number_of_track=(unsigned char)floppy->floppyNumberOfTrack;
		FILEHEADER->number_of_side=floppy->floppyNumberOfSide;
		if(floppy->floppyBitRate!=VARIABLEBITRATE)
		{
			FILEHEADER->bitRate=(floppy->floppyBitRate*factor)/1000;
		}
		else
		{
			FILEHEADER->bitRate=(floppy->tracks[0]->sides[0]->bitrate*factor)/1000;
		}
		FILEHEADER->floppyRPM=0;//floppy->floppyRPM;

		if(forceifmode==-1)
		{
			FILEHEADER->floppyinterfacemode=(unsigned char)floppy->floppyiftype;
		}
		else
		{
			FILEHEADER->floppyinterfacemode=forceifmode;
		}

		FILEHEADER->track_encoding=0;
		FILEHEADER->formatrevision=1;
		FILEHEADER->track_list_offset=1;
		FILEHEADER->write_protected=1;

		if(floppy->tracks[floppy->floppyNumberOfTrack/2]->sides[0]->track_encoding)
		{
			FILEHEADER->track_encoding=floppy->tracks[floppy->floppyNumberOfTrack/2]->sides[0]->track_encoding;
		}

        if(double_step)
			FILEHEADER->single_step=0x00;
		else
			FILEHEADER->single_step=0xFF;

        extrfwrite(FILEHEADER,512,1,hxcpicfile);

		tracklistlen=((((((FILEHEADER->number_of_track)+1)*sizeof(picexttrack))/512)+1));
		offsettrack=(unsigned char*) malloc(tracklistlen*512);
		memset(offsettrack,0xFF,tracklistlen*512);
		
		i=0;
		trackpos=FILEHEADER->track_list_offset+tracklistlen;

		while(i<(FILEHEADER->number_of_track))
		{
				mfmsize=0;
				mfmsize2=0;


				mfmsize=floppy->tracks[i]->sides[0]->tracklen * factor;
				if(mfmsize&7)
					mfmsize=(mfmsize/8)+1;
				else
					mfmsize=mfmsize/8;


				if(floppy->tracks[i]->number_of_side==2)
				{
					mfmsize2=floppy->tracks[i]->sides[1]->tracklen * factor;
					if(mfmsize2&7)
						mfmsize2=(mfmsize2/8)+1;
					else
						mfmsize2=mfmsize2/8;
				}



				if(mfmsize2>mfmsize) mfmsize=mfmsize2;
			
				if(mfmsize*2>0xFFFF)
				{
					floppycontext->hxc_printf(MSG_ERROR,"Argg!! track %d too long (%x) and shorten to 0xFFFF !",i,mfmsize*2);
					mfmsize=0x7FFF;
				}

				track=(picexttrack *)(offsettrack+(i*sizeof(picexttrack)));
				track->track_len=mfmsize*2;
				track->offset=trackpos;

				if((mfmsize*2)%512)
					trackpos=trackpos+(((mfmsize*2)/512)+1);
				else
					trackpos=trackpos+((mfmsize*2)/512);

				//trackpos=trackpos+(((mfmsize*2)/512)+1);
			i++;
		};

        extrfwrite(offsettrack,512*tracklistlen,1,hxcpicfile);

		i=0;
		while(i<(FILEHEADER->number_of_track))
		{

				mfmsize=floppy->tracks[i]->sides[0]->tracklen * factor;
				if(mfmsize&7)
					mfmsize=(mfmsize/8)+1;
				else
					mfmsize=mfmsize/8;

				mfmsize2=0;
				if(floppy->tracks[i]->number_of_side==2)
				{
					mfmsize2=floppy->tracks[i]->sides[1]->tracklen * factor;
					if(mfmsize2&7)
						mfmsize2=(mfmsize2/8)+1;
					else
						mfmsize2=mfmsize2/8;
				}

				if(mfmsize>0x7FFF)
				{
					mfmsize=0x7FFF;
				}
				if(mfmsize2>0x7FFF)
				{
					mfmsize2=0x7FFF;
				}
				track=(picexttrack *)(offsettrack+(i*sizeof(picexttrack)));

				if(track->track_len%512)
					tracksize=((track->track_len&(~0x1FF))+0x200)/2;//(((track->track_len/512)+1)*512)/2;
				else
					tracksize=track->track_len/2;

				if(factor>1)
				{
					mfmtemp=(unsigned char*) malloc(tracksize);
					memset(mfmtemp,0x00,tracksize);
				}
				mfmtracks0=(unsigned char*) malloc(tracksize);
				mfmtracks1=(unsigned char*) malloc(tracksize);
				mfmtrackfinal=(unsigned char*) malloc(tracksize*2);

				memset(mfmtracks0,0x00,tracksize);
				memset(mfmtracks1,0x00,tracksize);
				memset(mfmtrackfinal,0x55,tracksize*2);

				if(factor==1)
				{
					memcpy(mfmtracks0,floppy->tracks[i]->sides[0]->databuffer,mfmsize);
					extaddpad(mfmtracks0,mfmsize,tracksize);
					//memset(&mfmtracks0[mfmsize],floppy->tracks[i]->sides[0]->databuffer[mfmsize-1],tracksize-mfmsize);
				}
				else
				{
					memcpy(mfmtemp,floppy->tracks[i]->sides[0]->databuffer,mfmsize/2);
					memset(&mfmtemp[mfmsize/2],floppy->tracks[i]->sides[0]->databuffer[(mfmsize/2)-1],(tracksize/2)-(mfmsize/2));

					for(l=0;l<(tracksize/2);l=l+1)
					{	
						mfmtracks0[l*2]=  ext_bit_expander[mfmtemp[l]]>>8;
						mfmtracks0[(l*2)+1]=ext_bit_expander[mfmtemp[l]]&0xFF;
					}
				}
				if(floppy->tracks[i]->number_of_side==2)
				{
				
					if(factor==1)
					{
						memcpy(mfmtracks1,floppy->tracks[i]->sides[1]->databuffer,mfmsize2);
						//memset(&mfmtracks1[mfmsize2],floppy->tracks[i]->sides[1]->databuffer[mfmsize2-1],tracksize-mfmsize2);
						extaddpad(mfmtracks1,mfmsize2,tracksize);
					}
					else
					{
						memcpy(mfmtemp,floppy->tracks[i]->sides[1]->databuffer,mfmsize2/2);
						memset(&mfmtemp[mfmsize2/2],floppy->tracks[i]->sides[1]->databuffer[(mfmsize2/2)-1],(tracksize/2)-(mfmsize2/2));

						for(l=0;l<(tracksize/2);l=l+1)
						{
							mfmtracks1[l*2]=  (ext_bit_expander[mfmtemp[l]]>>8);
							mfmtracks1[(l*2)+1]=ext_bit_expander[mfmtemp[l]]&0xFF;
						}
					}

				}
				if(factor>1)
					free(mfmtemp);
				
				for(k=0;k<tracksize/256;k++)
				{

					for(j=0;j<256;j++)
					{
						// inversion des bits pour le EUSART du PIC.

						// head 0
						mfmtrackfinal[(k*512)+j]=     ext_bit_inverter[mfmtracks0[(k*256)+j]];
						// head 1
						mfmtrackfinal[(k*512)+j+256]= ext_bit_inverter[mfmtracks1[(k*256)+j]];

					}
				}
				
				
				extrfwrite(mfmtrackfinal,tracksize*2,1,hxcpicfile);
					
				free(mfmtracks0);
				free(mfmtracks1);
				free(mfmtrackfinal);

		
			i++;
		};

		free(offsettrack);

 
#ifdef FASTWRITE
		hxcpicfile=fopen(filename,"wb");
		if(hxcpicfile)
		{
			fwrite(ramfile,ramfile_size,1,hxcpicfile);
			fclose(hxcpicfile);
		}
		else
		{
			extrfclose(hxcpicfile);
			floppycontext->hxc_printf(MSG_ERROR,"Cannot create %s!",filename);
			return -1;
		}

#endif

		extrfclose(hxcpicfile);	

		floppycontext->hxc_printf(MSG_INFO_1,"%d tracks written to the file",FILEHEADER->number_of_track);

		free(FILEHEADER);

		return 0;
	}
	else
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot create %s!",filename);

		return -1;
	}

}
