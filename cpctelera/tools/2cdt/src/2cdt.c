/*
 *  2CDT Copyright (c) Kevin Thacker
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/* The following program is designed to create a .tzx/.cdt from a tape-file stored
on the PC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "tzxfile.h"
#include "getopt.h"

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int64 ulong64;
typedef signed __int64 long64;
#else
typedef unsigned long long ulong64;
typedef signed long long long64;
#endif

enum
{
    CPC_METHOD_BLOCKS = 0,
    CPC_METHOD_HEADERLESS,
    CPC_METHOD_SPECTRUM,
};

static int ExecutionAddress;
static BOOL ExecutionAddressOverride;
static int LoadAddress;
static BOOL  LoadAddressOverride;
static int Type;
static BOOL TypeOverride;
static int Pause;
static BOOL BuggyEmuExtraPause;

#define MAXFILELEN 16

static int BaudRate;                /* baud rate to write data */
static int TZXWriteMethod;          /* method to write data into TZX file */
static BOOL BlankBeforeUse;         /* blank existing CDT file before use */
static int CPCMethod = CPC_METHOD_BLOCKS;

/* I am using a enum, so that I can poke data into structures without
worrying how the compiler has aligned it */
enum
{
	CPC_TAPE_HEADER_FILENAME_BYTE0 = 0,
	CPC_TAPE_HEADER_FILENAME_BYTE1,
	CPC_TAPE_HEADER_FILENAME_BYTE2,
	CPC_TAPE_HEADER_FILENAME_BYTE3,
	CPC_TAPE_HEADER_FILENAME_BYTE4,
	CPC_TAPE_HEADER_FILENAME_BYTE5,
	CPC_TAPE_HEADER_FILENAME_BYTE6,
	CPC_TAPE_HEADER_FILENAME_BYTE7,
	CPC_TAPE_HEADER_FILENAME_BYTE8,
	CPC_TAPE_HEADER_FILENAME_BYTE9,
	CPC_TAPE_HEADER_FILENAME_BYTE10,
	CPC_TAPE_HEADER_FILENAME_BYTE11,
	CPC_TAPE_HEADER_FILENAME_BYTE12,
	CPC_TAPE_HEADER_FILENAME_BYTE13,
	CPC_TAPE_HEADER_FILENAME_BYTE14,
	CPC_TAPE_HEADER_FILENAME_BYTE15,
	CPC_TAPE_HEADER_BLOCK_NUMBER,
	CPC_TAPE_HEADER_LAST_BLOCK_FLAG,
	CPC_TAPE_HEADER_FILE_TYPE,
	CPC_TAPE_HEADER_DATA_LENGTH_LOW,
	CPC_TAPE_HEADER_DATA_LENGTH_HIGH,
	CPC_TAPE_HEADER_DATA_LOCATION_LOW,
	CPC_TAPE_HEADER_DATA_LOCATION_HIGH,
	CPC_TAPE_HEADER_FIRST_BLOCK_FLAG,
	CPC_TAPE_HEADER_DATA_LOGICAL_LENGTH_LOW,
	CPC_TAPE_HEADER_DATA_LOGICAL_LENGTH_HIGH,
	CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_LOW,
	CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_HIGH,
} CPC_TAPE_HEADER_ENUM;

/* size of header */
#define CPC_TAPE_HEADER_SIZE	64

/* load a file into memory */
BOOL	Host_LoadFile(const char *Filename, unsigned char **pLocation, unsigned long *pLength)
{
	FILE	*fh;
	unsigned char	*pData;

	*pLocation = NULL;
	*pLength = 0;

	if (Filename!=NULL)
	{
		if (strlen(Filename)!=0)
		{
			fh = fopen(Filename,"rb");

			if (fh!=NULL)
			{
				int FileSize;

#ifdef WIN32
				int FNo;

				FNo = _fileno(fh);
				FileSize = _filelength(FNo);
#else
				unsigned long CurrentPosition;
				CurrentPosition = ftell(fh);
				fseek(fh, 0, SEEK_END);
				FileSize = ftell(fh);
				fseek(fh, CurrentPosition, SEEK_SET);
#endif
				if (FileSize!=0)
				{
					pData = (unsigned char *)malloc(FileSize);

					if (pData!=NULL)
					{
						if (fread(pData,1,FileSize,fh)==FileSize)
						{
							*pLocation = pData;
							*pLength = FileSize;
							fclose(fh);
							return TRUE;
						}

						free(pData);
					}
				}

				fclose(fh);
			}
		}
	}

	return FALSE;
}


/* calculate checksum as AMSDOS would for the first 66 bytes of a datablock */
/* this is used to determine if a file has a AMSDOS header */
unsigned int AMSDOS_CalculateChecksum(unsigned char *pHeader)
{
        unsigned int Checksum;
        int i;

        Checksum = 0;

        for (i=0; i<67; i++)
        {
                unsigned int CheckSumByte;

                CheckSumByte = pHeader[i] & 0x0ff;

                Checksum+=CheckSumByte;
        }

        return Checksum;
}

/* CRC code shamelessly taken from Pierre Guerrier's AIFF decoder! */
#define kCRCpoly  4129  /* used for binary long division in CRC */

/* CRC polynomial: X^16+X^12+X^5+1 */
unsigned int CRCupdate(unsigned int CRC, unsigned char new)
{
 unsigned int aux = CRC ^ (new << 8);
 int i;

 for(i=0; i<8; i++)
   if (aux & 0x8000)
       aux = (aux << 1) ^ kCRCpoly;
     else
       aux <<= 1;

 return(aux);
}

/*
ID : 11  -  Turbo loading data block
-------
        This block is very similar  to  the  normal  TAP  block but  with  some
        additional   info  on  the  timings  and  other  important differences.
        The same tape encoding is used as for the standard speed data block.
        If a  block should  use some  non-standard  sync or  pilot  tones  (for
        example all sorts of protection schemes) then use the next three blocks
        to describe it.

00 2  Length of PILOT pulse                                              [2168]
02 2  Length of SYNC First pulse                                          [667]
04 2  Length of SYNC Second pulse                                         [735]
06 2  Length of ZERO bit pulse                                            [855]
08 2  Length of ONE bit pulse                                            [1710]
0A 2  Length of PILOT tone (in PILOT pulses)           [8064 Header, 3220 Data]
0C 1  Used bits in last byte (other bits should be 0)                       [8]
      i.e. if this is 6 then the bits (x) used in last byte are: xxxxxx00
0D 2  Pause After this block in milliseconds (ms)                        [1000]
0F 3  Length of following data
12 x  Data; format is as for TAP (MSb first)

- Length: [0F,10,11]+12
*/

/* 2 pulses per bit, tone is composed of 1 bits */
#define CPC_PILOT_TONE_NUM_WAVES	(2048)
#define CPC_PILOT_TONE_NUM_PULSES (CPC_PILOT_TONE_NUM_WAVES*2)

#define CPC_NOPS_PER_FRAME (19968)
#define CPC_NOPS_PER_SECOND	(CPC_NOPS_PER_FRAME*50)
#define CPC_T_STATES	(CPC_NOPS_PER_SECOND*4)

#define T_STATE_CONVERSION_FACTOR (TZX_T_STATES<<8)/(CPC_T_STATES>>8)
/* pause between each block */
#define CPC_PAUSE_AFTER_BLOCK_IN_MS	2500
/* pause between tape header and data for block */
#define CPC_PAUSE_AFTER_HEADER_IN_MS 14

void InitialiseStandardSpeedDataBlock(TZX_BLOCK *pBlock, int Pause)
{
	unsigned char *pHeader = TZX_GetBlockHeaderPtr(pBlock);

	if (pHeader!=NULL)
	{
		/* check it is a turbo-loading data block */
		if (pHeader[0] == TZX_STANDARD_SPEED_DATA_BLOCK)
		{
			pHeader++;

			pHeader[0x00] = (Pause & 0x0ff);
			pHeader[0x01] = (Pause>>8);


		}
	}
}

void	CPC_InitialiseTurboLoadingDataBlock(TZX_BLOCK *pBlock, int BaudRate,int Pause)
{
	unsigned char *pHeader = TZX_GetBlockHeaderPtr(pBlock);

	if (pHeader!=NULL)
	{
		/* check it is a turbo-loading data block */
		if (pHeader[0] == TZX_TURBO_LOADING_DATA_BLOCK)
		{
			int ZeroPulseLengthInMicroseconds;
			long64 ZeroPulseLengthInCPCTStates;
			int OnePulseLength;
			int ZeroPulseLength;

			pHeader++;
			/* equation from CPC firmware guide:
			Average baud rate: = 1 000 000/(3*half zero length) = 333 333/Half zero length
			*/

			ZeroPulseLengthInMicroseconds = 333333/BaudRate;
			ZeroPulseLengthInCPCTStates = ZeroPulseLengthInMicroseconds<<2;

			ZeroPulseLength = (ZeroPulseLengthInCPCTStates*
							(T_STATE_CONVERSION_FACTOR>>8))>>8;

			/* one pulse is twice the size of a zero pulse */
			OnePulseLength = ZeroPulseLength<<1;

			/* PILOT pulse on CPC is a one bit */
			pHeader[0x00] = (unsigned char)OnePulseLength;
			pHeader[0x01] = (unsigned char)(OnePulseLength>>8);

			/* SYNC on CPC is a zero bit, both sync pulses will be the same */
			pHeader[0x02] = pHeader[0x04] = (unsigned char)ZeroPulseLength;
			pHeader[0x03] = pHeader[0x05] = (unsigned char)(ZeroPulseLength>>8);

			/* write zero pulse length */
			pHeader[0x06] = (unsigned char)ZeroPulseLength;
			pHeader[0x07] = (unsigned char)(ZeroPulseLength>>8);

			/* write one pulse length */
			pHeader[0x08] = (unsigned char)OnePulseLength;
			pHeader[0x09] = (unsigned char)(OnePulseLength>>8);

			/* PILOT pulse is same as 1 Pulse */
			/* pilot tone is 2048 bits long */
			pHeader[0x0a] = CPC_PILOT_TONE_NUM_PULSES & 0x0ff;
			pHeader[0x0b] = (CPC_PILOT_TONE_NUM_PULSES>>8);

			/* the end of the block will be the trailer bytes. Say all bits are
			used, although, because it doesn't contain useful data it doesn't matter */
			pHeader[0x0c] = 8;

			pHeader[0x0d] = (Pause & 0x0ff);
			pHeader[0x0e] = (Pause>>8);


		}
	}
}



void WriteStandardSpeedDataBlock(TZX_FILE *pFile, unsigned char SyncPattern, unsigned char *pData, int DataSize, int Pause)
{
	TZX_BLOCK *pBlock;
	unsigned char *pBlockData;

	pBlock = TZX_CreateBlock(TZX_STANDARD_SPEED_DATA_BLOCK);
	InitialiseStandardSpeedDataBlock(pBlock, Pause);

	if (pBlock!=NULL)
	{
	    /* one byte for sync, one byte for checksum */
        int TZX_DataBlockSize = DataSize+2;

		/* add block to end of file */
		TZX_AddBlockToEndOfFile(pFile,pBlock);

		/* allocate data in block */
		TZX_AddDataToBlock(pBlock, TZX_DataBlockSize);

		pBlockData= TZX_GetBlockDataPtr(pBlock);

		if (pBlockData!=NULL)
		{
		    char CheckSum = SyncPattern;
            int i;

            /* write pattern */
			*pBlockData = SyncPattern;
			++pBlockData;

            for (i=0; i<DataSize; i++)
            {
                char ch = *pData;
                ++pData;
                CheckSum^=ch;
                *pBlockData = ch;
                ++pBlockData;
            }
            *pBlockData = CheckSum&0x0ff;
		}
	}
}


#define CPC_DATA_CHUNK_SIZE 256
#define CPC_DATA_BLOCK_SIZE 2048

/* write a block of data to a file */
void	CPC_WriteTurboLoadingDataBlock(TZX_FILE *pFile, unsigned char SyncPattern, unsigned char *pData, int DataSize, int Pause)
{
	TZX_BLOCK *pBlock;
	unsigned char *pBlockData;

	int NumChunks;
	int TZX_DataBlockSize;

	/* divide into complete 256 byte blocks */
	NumChunks = (DataSize+255)>>8;

	/* each tape block is split into 256 chunks, each chunk has a CRC */

	/* size of all chunks, plus CRC's for each block */
	TZX_DataBlockSize =
		/* size of all chunks */
		(NumChunks<<8) +
		/* size of CRC's for all chunks */
		(NumChunks<<1) +
		/* size of trailer in bytes */
		4 +
		/* size of sync pattern */
		1;



	pBlock = TZX_CreateBlock(TZX_TURBO_LOADING_DATA_BLOCK);
	CPC_InitialiseTurboLoadingDataBlock(pBlock, BaudRate,Pause);


	if (pBlock!=NULL)
	{
		/* add block to end of file */
		TZX_AddBlockToEndOfFile(pFile,pBlock);

		/* allocate data in block */
		TZX_AddDataToBlock(pBlock, TZX_DataBlockSize);

		pBlockData= TZX_GetBlockDataPtr(pBlock);

		if (pBlockData!=NULL)
		{
			int i,j;
			unsigned char *pDataPtr;
			int DataSizeRemaining;
			unsigned char *pBlockPtr;
			unsigned short CRC;

			pDataPtr = pData;
			DataSizeRemaining = DataSize;
			pBlockPtr = pBlockData;

			/* write pattern */
			pBlockPtr[0] = SyncPattern;
			pBlockPtr++;

			/* write each chunk in turn and calculate CRC */
			for (i=0; i<NumChunks; i++)
			{
				/* copy data into block */
				if (DataSizeRemaining<CPC_DATA_CHUNK_SIZE)
				{
					/* less than CPC_DATA_CHUNK_SIZE */
					/* copy data, and fill rest with zeros */

					/* copy less than 256 bytes */
					memcpy(pBlockPtr, pDataPtr, DataSizeRemaining);
					/* fill reset of chunk with zero's */
					memset(pBlockPtr + DataSizeRemaining, 0, CPC_DATA_CHUNK_SIZE-DataSizeRemaining);
					/* update source pointer */
					pDataPtr+=DataSizeRemaining;
					/* update size remaining - nothing */
					DataSizeRemaining = 0;
				}
				else
				{
					/* greater or equal to CPC_DATA_CHUNK_SIZE */
					/* copy CPC_DATA_CHUNK_SIZE max */
					memcpy(pBlockPtr, pDataPtr, CPC_DATA_CHUNK_SIZE);
					/* update source pointer */
					pDataPtr += CPC_DATA_CHUNK_SIZE;
					/* update size remaining */
					DataSizeRemaining-=CPC_DATA_CHUNK_SIZE;
				}

				/* reset CRC */
				CRC = 0x0ffff;

				/* calculate CRC for block */
				for (j=0; j<CPC_DATA_CHUNK_SIZE; j++)
				{
					char ch;

					ch = pBlockPtr[0];
					pBlockPtr++;
					CRC = CRCupdate(CRC, ch);
				}

				/* store CRC inverted */
				pBlockPtr[0] = (CRC>>8)^0x0ff;
				pBlockPtr++;
				pBlockPtr[0] = CRC^0x0ff;
				pBlockPtr++;
			}


			/* write trailer */
			memset(pBlockPtr, 0x0ff, 4);
		}
	}
}

/*
ID : 14  -  Pure data block
-------
        This is the same as in the turbo loading data block, except that it has
        no pilot or sync pulses.

00 2  Length of ZERO bit pulse
02 2  Length of ONE bit pulse
04 1  Used bits in LAST Byte
05 2  Pause after this block in milliseconds (ms)
07 3  Length of following data
0A x  Data
*/

void	CPC_InitialisePureDataBlock(TZX_BLOCK *pBlock, int BaudRate, int Pause)
{
	unsigned char *pHeader = TZX_GetBlockHeaderPtr(pBlock);

	if (pHeader!=NULL)
	{
		/* check it is a turbo-loading data block */
		if (pHeader[0] == TZX_PURE_DATA_BLOCK)
		{
			int ZeroPulseLengthInMicroseconds;
			long64 ZeroPulseLengthInCPCTStates;
			int OnePulseLength;
			int ZeroPulseLength;

			pHeader++;
			/* equation from CPC firmware guide:
			Average baud rate: = 1 000 000/(3*half zero length) = 333 333/Half zero length
			*/

			ZeroPulseLengthInMicroseconds = 333333/BaudRate;
			ZeroPulseLengthInCPCTStates = ZeroPulseLengthInMicroseconds<<2;

			ZeroPulseLength = (ZeroPulseLengthInCPCTStates*
							(T_STATE_CONVERSION_FACTOR>>8))>>8;

			/* one pulse is twice the size of a zero pulse */
			OnePulseLength = ZeroPulseLength<<1;
			/* write zero pulse length */
			pHeader[0x00] = ZeroPulseLength;
			pHeader[0x01] = ZeroPulseLength>>8;

			/* write one pulse length */
			pHeader[0x02] = OnePulseLength;
			pHeader[0x03] = OnePulseLength>>8;

			/* the end of the block will be the trailer bytes. Say all bits are
			used, although, because it doesn't contain useful data it doesn't matter */
			pHeader[0x04] = 8;

			/* write pause */
			pHeader[0x05] = (Pause & 0x0ff);
			pHeader[0x06] = (Pause>>8) & 0x0ff;


		}
	}
}


/* the following is for a bitstream */
unsigned char *pData;
unsigned long ByteCount;
unsigned long BitCount;

/* initialise bit stream with buffer to write data to */
void	BitStream_Initialise(unsigned char *pBuffer)
{
	pData = pBuffer;
	ByteCount = 0;
	BitCount = 0;
}

/* write bit to stream */
void	BitStream_WriteBit(int Bit)
{
	unsigned char Data;

	/* get current data written */
	Data = pData[ByteCount];
	Data &= ~(1<<(7-BitCount));
	Data |= (Bit<<(7-BitCount));
	pData[ByteCount] = Data;

	/* increment bit count */
	BitCount++;
	/* if we overrun 8-bits, then bit 3 will be set, add this on */
	ByteCount += (BitCount>>3);
	/* mask off bit count */
	BitCount &= 0x07;
}

/* write byte to stream */
void	BitStream_WriteByte(unsigned char Byte)
{
	int b;
	int Bit;
	unsigned char LocalByte;

	LocalByte = Byte;

	for (b=0; b<8; b++)
	{
		Bit = LocalByte & 0x080;
		Bit = Bit>>7;
		BitStream_WriteBit(Bit);
		LocalByte = LocalByte<<1;
	}
}



/* write a block of data to a file */
void	CPC_WritePureDataBlock(TZX_FILE *pFile, unsigned char SyncPattern, unsigned char *pData, int DataSize, int Pause)
{
	TZX_BLOCK *pBlock;
	unsigned char *pBlockData;

	int NumChunks;
	int TZX_DataBlockSize;

	/* divide into complete 256 byte blocks */
	NumChunks = (DataSize+255)>>8;

	/* each tape block is split into 256 chunks, each chunk has a CRC */

	/* size of all chunks, plus CRC's for each block */
	TZX_DataBlockSize =
		/* size of all chunks */
		(NumChunks<<8) +
		/* size of CRC's for all chunks */
		(NumChunks<<1) +
		/* size of trailer in bytes */
		4 +
		/* size of sync pattern */
		1;

	TZX_DataBlockSize+=
		/* pilot tone - CPC_PILOT_TONE_NUM_WAVES 1 bit's, a zero bit then data as before ... */
		((CPC_PILOT_TONE_NUM_WAVES + 1)+7)>>3;

	pBlock = TZX_CreateBlock(TZX_PURE_DATA_BLOCK);
	CPC_InitialisePureDataBlock(pBlock, BaudRate,Pause);


	if (pBlock!=NULL)
	{
		/* add block to end of file */
		TZX_AddBlockToEndOfFile(pFile,pBlock);

		/* allocate data in block */
		TZX_AddDataToBlock(pBlock, TZX_DataBlockSize);

		pBlockData= TZX_GetBlockDataPtr(pBlock);

		if (pBlockData!=NULL)
		{
			int i,j;
			unsigned char *pDataPtr;
			int DataSizeRemaining;
			unsigned char *pBlockPtr;
			unsigned short CRC;

			pDataPtr = pData;
			DataSizeRemaining = DataSize;
			pBlockPtr = pBlockData;

			BitStream_Initialise(pBlockPtr);

			/* write leader */
			for (i=0; i<CPC_PILOT_TONE_NUM_WAVES; i++)
			{
				BitStream_WriteBit(1);
			}

			BitStream_WriteBit(0);


			BitStream_WriteByte(SyncPattern);

			/* write each chunk in turn and calculate CRC */
			for (i=0; i<NumChunks; i++)
			{
				int BlockSizeToWrite;

				/* copy data into block */
				if (DataSizeRemaining<CPC_DATA_CHUNK_SIZE)
				{
					BlockSizeToWrite = DataSizeRemaining;
				}
				else
				{
					BlockSizeToWrite = CPC_DATA_CHUNK_SIZE;
				}

				CRC = 0x0ffff;

				for (j=0; j<BlockSizeToWrite; j++)
				{
					char ch;

					/* get byte */
					ch = pDataPtr[0];
					pDataPtr++;
					/* update CRC */
					CRC = CRCupdate(CRC, ch);
					/* write byte to stream */
					BitStream_WriteByte(ch);
				}

				if (BlockSizeToWrite!=CPC_DATA_CHUNK_SIZE)
				{
					/* write padding zero's */
					for (j=0; j<(CPC_DATA_CHUNK_SIZE-BlockSizeToWrite); j++)
					{
						char ch;

						ch = 0;
						/* update CRC */
						CRC = CRCupdate(CRC, ch);
						/* write byte to stream */
						BitStream_WriteByte(ch);
					}
				}

				DataSizeRemaining-=BlockSizeToWrite;

				CRC = CRC^0x0ffff;

				BitStream_WriteByte((CRC>>8));
				BitStream_WriteByte(CRC);
			}

			/* write trailer */
			for (i=0; i<32; i++)
			{
				BitStream_WriteBit(1);
			}
		}
	}
}

/* write a data block in format specified */
void	CPC_WriteDataBlock(TZX_FILE *pFile, unsigned char SyncByte, unsigned char *pData, unsigned long DataSize, int Pause)
{
	switch (TZXWriteMethod)
	{
		case TZX_TURBO_LOADING_DATA_BLOCK:
		{
			CPC_WriteTurboLoadingDataBlock(pFile, SyncByte, pData, DataSize,Pause);
		}
		break;

		case TZX_PURE_DATA_BLOCK:
		{
			/* write header */
			CPC_WritePureDataBlock(pFile, SyncByte, pData, DataSize,Pause);
		}
		break;
	}
}

#define UTILITY_NAME "2CDT"

void	DisplayInfo()
{
		printf("%s will transfer files into a .CDT/.TZX tape image, in Amstrad CPC/CPC+\r\n", UTILITY_NAME);
		printf("KC Compact form.\r\n\r\n");
		printf("Usage: %s [arguments] <input filename> <.cdt image>\r\n\r\n", UTILITY_NAME);
		printf("-n              - Blank CDT file before use\n");
        printf("-b <number>	    - Specify Baud rate (default 2000)\n");
		printf("-s <0 or 1>     - Specify 'Speed Write'.\n");
		printf("                  0 = 1000 baud, 1 = 2000 baud (default)\n");
		printf("-t <method>     - TZX Block Write Method.\n");
		printf("                  0 = Pure Data, 1 = Turbo Loading (default)\n");
		printf("-m <method>     - Data method\n");
        printf("                  0 = blocks (default)\n");
        printf("                  1 = headerless (Firmware function: CAS READ - &BCA1) \n");
        printf("                  2 = spectrum \n");
        printf("                  3 = Two blocks. First block of 2K, second block has remainder\n");
        printf("                  4 = Two blocks. First block of 1 byte, second block has remainder\n");
		printf("-X <number> 	= Define or override execution address (default is &1000 if no header)\r\n");
		printf("-L <number> 	= Define or override load address (default is &1000 if no header)\r\n");
 		printf("-F <number> 	= Define or override file type (0=BASIC, 2=Binary (default if no header)) etc. Applies to Data method 0\r\n");
 		printf("-p <number> 	= Set initial pause in milliseconds (default 3000ms)\r\n");
 		printf("-P 				= Add a 1ms pause for buggy emulators that ignore first block\r\n");
		printf("-r <tape filename>\n");
        printf("                - Add <input filename> as <tape filename> to CDT (rename file)\n");
}

extern char *optarg;

int ReadNumberParameter(char *param)
{
    int Length = strlen(param);
    BOOL bIsHex = FALSE;
    int Offset = 0;
    unsigned long Value = 0;
    char ch;

    if (Length==0)
        return 0;

    /* check for common prefixs for hex numbers */
    if ((Length>1) && ((param[0]=='&') || (param[0]=='$')))
    {
        Offset = 1;
        bIsHex = TRUE;
    }
    else if ((Length>2) && (param[0]=='0') && ((param[1]=='x') || (param[1]=='X')))
    {
        Offset = 2;
        bIsHex = TRUE;
    }

    if (!bIsHex)
    {
        return atoi(param);
    }

    ch = param[Offset];
    while (ch!='\0')
    {
        Value = Value<<4;
        if ((ch>='0') && (ch<='9'))
        {
            Value = Value | (ch-'0');
        }
        else if ((ch>='a') && (ch<='f'))
        {
            Value = Value | ((ch-'a')+10);
        }
        else if ((ch>='A') && (ch<='F'))
        {
            Value = Value | ((ch-'A')+10);
        }
        Offset++;
        ch = param[Offset];
    }

    return Value;
}

int		main(int argc, char *argv[])
{
    unsigned char *pTapeFilename = NULL;

	if (argc==1)
	{
		DisplayInfo();
	}
	else
	{
		TZX_FILE *pTZXFile;
		unsigned char *pSourceFilename;
		unsigned char *pDestFilename;
		unsigned char *pData;
		unsigned long DataLength;
        char c;

		/* initialise defaults */
		BaudRate = 2000;
		Pause = 3000;
		Type = 2;
		TypeOverride = FALSE;
		LoadAddressOverride = FALSE;
		TZXWriteMethod = TZX_TURBO_LOADING_DATA_BLOCK;
        BlankBeforeUse = FALSE;
        ExecutionAddress = LoadAddress = 0x01000;
		ExecutionAddressOverride = FALSE;
		LoadAddressOverride = FALSE;

		printf("-n              - Blank CDT file before use\n");
        printf("-b <number>	    - Specify Baud rate (default 2000)\n");
		printf("-s <0 or 1>     - Specify 'Speed Write'.\n");
		printf("                  0 = 1000 baud, 1 = 2000 baud (default)\n");
		printf("-t <method>     - TZX Block Write Method.\n");
		printf("                  0 = Pure Data, 1 = Turbo Loading (default)\n");
		printf("-m <method>     - Data method\n");
        printf("                  0 = blocks (default)\n");
        printf("                  1 = headerless (Firmware function: CAS READ - &BCA1) \n");
        printf("                  2 = spectrum \n");
        printf("                  3 = Two blocks. First block of 2K, second block has remainder\n");
        printf("                  4 = Two blocks. First block of 1 byte, second block has remainder\n");
		printf("-X <number> 	= Define or override execution address (default is &1000 if no header)\r\n");
		printf("-L <number> 	= Define or override load address (default is &1000 if no header)\r\n");
 		printf("-F <number> 	= Define or override file type (0=BASIC, 2=Binary (default if no header)) etc. Applies to Data method 0\r\n");
 		printf("-p <number> 	= Set initial pause in milliseconds (default 3000ms)\r\n");
 		printf("-P 				= Add a 1ms pause for buggy emulators that ignore first block\r\n");
		printf("-r <tape filename>\n");
        printf("                - Add <input filename> as <tape filename> to CDT (rename file)\n");


        do
        {
            c = getopt(argc, argv,"r:nb:p:m:t:F:L:s:X:p:P");

            switch (c)
            {
                case 'm':
                {
                    int nMethod = atoi(optarg);
                    CPCMethod = nMethod;

                }
                break;

                case 'p':
                {
	                Pause = atoi(optarg);
	                if (Pause<0)
	                {
	                    Pause = 0;
	                }
               }
               break;

                case 'r':
                {
                    pTapeFilename = (unsigned char*)optarg;
                }
                break;

                case 'n':
                {
                    BlankBeforeUse = TRUE;
                }
                break;

                case 'b':
                {
                    int Baud = atoi(optarg);
                    if ((Baud>0) && (Baud<6000))
                        BaudRate = Baud;
                }
                break;

                case 'X':
                {
	                ExecutionAddress = ReadNumberParameter(optarg) & 0x0ffff;
	                ExecutionAddressOverride = TRUE;
                }
                break;

                case 'L':
                {
	                LoadAddress = ReadNumberParameter(optarg) & 0x0ffff;
	                LoadAddressOverride = TRUE;
                }
                break;

                case 's':
                {
                    int SpeedWrite = atoi(optarg);
                    if (SpeedWrite==1)
                    {
                        BaudRate = 2000;
                    }
                    else
                    {
                        BaudRate = 1000;
                    }
                }
                break;

                case 'F':
                {
	                Type = atoi(optarg) & 0x0ff;
	                TypeOverride = TRUE;


                }
                break;


                case 'P':
                {
	                BuggyEmuExtraPause = TRUE;

                }
                break;

                case 't':
                {
                    int nMethod = atoi(optarg);
                    if (nMethod==0)
                    {
                        TZXWriteMethod = TZX_PURE_DATA_BLOCK;
                    }
                    else if (nMethod==1)
                    {
                        TZXWriteMethod = TZX_TURBO_LOADING_DATA_BLOCK;
                    }
                    else if (nMethod==2)
                    {
                        TZXWriteMethod = TZX_STANDARD_SPEED_DATA_BLOCK;
                    }
                }
                break;

                default:
                {
                }
                break;
            }

        }
        while (c!=-1);

        if ((argc-optind)==0)
        {
            printf("No source file or destination file have been specified!\n");
            exit(1);
        }

        if ((argc-optind)==1)
        {
            printf("No destination file has been specified\n");
            exit(1);
        }

        pSourceFilename = (unsigned char*)argv[optind];
        pDestFilename   = (unsigned char*)argv[optind+1];

		/* create TZX file */
		pTZXFile = TZX_CreateFile(TZX_VERSION_MAJOR,TZX_VERSION_MINOR);

		if (pTZXFile!=NULL)
		{
			//int nFile;


			if (BlankBeforeUse)
			{
				TZX_BLOCK *pBlock;

				/* if buggy emu, add an extra small pause */
				if (BuggyEmuExtraPause)
                {
					pBlock = TZX_CreateBlock(TZX_PAUSE_BLOCK);

					if (pBlock!=NULL)
					{
						/* add a 1ms initial pause for buggy emus */
						TZX_SetupPauseBlock(pBlock, 1);
						TZX_AddBlockToEndOfFile(pTZXFile,pBlock);
					}
				}


				/* correct pause */
				pBlock = TZX_CreateBlock(TZX_PAUSE_BLOCK);

				if (pBlock!=NULL)
				{
					TZX_SetupPauseBlock(pBlock, Pause);
					TZX_AddBlockToEndOfFile(pTZXFile,pBlock);
				}
			}


            if (Host_LoadFile( (char*)pSourceFilename, &pData, &DataLength))
            {
                int FileOffset;
                int FileLengthRemaining;
                int TapeBlockSize = 0;
                BOOL FirstBlock,LastBlock = FALSE;
                int BlockIndex;
                unsigned short BlockLocation;

                /* header for tape file */
                unsigned char TapeHeader[CPC_TAPE_HEADER_SIZE];

                /* calculate checksum from loaded file */
                unsigned short CalculatedChecksum = (unsigned short)AMSDOS_CalculateChecksum(pData);

                /* get stored checksum */
                unsigned short StoredChecksum =
                                    (pData[67] & 0x0ff) |
                                     (pData[68] & 0x0ff)<<8;

                FileOffset = 0;
                FileLengthRemaining = DataLength;
                BlockIndex = 1;
                FirstBlock =TRUE;

                /* insert a pause block - 1 second, this is added onto the end of the previous block */
    /*            if (BlankBeforeUse == FALSE)
                {
                    TZX_BLOCK *pBlock;

                    pBlock = TZX_CreateBlock(TZX_PAUSE_BLOCK);

                    if (pBlock!=NULL)
                    {
                        TZX_SetupPauseBlock(pBlock, 2000);
                        TZX_AddBlockToEndOfFile(pTZXFile,pBlock);
                    }
                }
*/


                /* clear tape header */
                memset(TapeHeader, 0, CPC_TAPE_HEADER_SIZE);

                /* checksum's match? */
                if (CalculatedChecksum==StoredChecksum)
                {
                    /* copy file type */
                    TapeHeader[CPC_TAPE_HEADER_FILE_TYPE] = pData[CPC_TAPE_HEADER_FILE_TYPE];
                    /* copy execution address */
                    TapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_LOW] = pData[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_LOW];
                    TapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_HIGH] = pData[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_HIGH];
                    /* copy data location */
                    TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] = pData[CPC_TAPE_HEADER_DATA_LOCATION_LOW];
                    TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH] = pData[CPC_TAPE_HEADER_DATA_LOCATION_HIGH];

                    FileOffset+=128;
                    FileLengthRemaining-=128;

                    /* override execution address? */
                    if (ExecutionAddressOverride)
                    {
						TapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_LOW] = ExecutionAddress&0xFF;
						TapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_HIGH] = (ExecutionAddress>>8)&0xFF;
                    }

                    /* override type? */
                    if (TypeOverride)
                    {
						TapeHeader[CPC_TAPE_HEADER_FILE_TYPE] = Type;
                    }

                    /* override load address? */
                    if (LoadAddressOverride)
                    {
						TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] = LoadAddress&0xFF;
						TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH] = (LoadAddress>>8)&0xFF;
					}

                }
                else
                {
					/* set type */
					TapeHeader[CPC_TAPE_HEADER_FILE_TYPE] = Type;

					/* set execution address */
					TapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_LOW] = ExecutionAddress&0xFF;
					TapeHeader[CPC_TAPE_HEADER_DATA_EXECUTION_ADDRESS_HIGH] = (ExecutionAddress>>8)&0xFF;

					/* set load address */
					TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] = LoadAddress&0xFF;
					TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH] = (LoadAddress>>8)&0xFF;
                }

                if (pTapeFilename!=NULL)
                {
                    int i;
                    int nLength = strlen( (char*)pTapeFilename);
                    if (nLength>16)
                        nLength = 16;
                    for (i=0; i<nLength; i++)
                    {
                        TapeHeader[i] = toupper(pTapeFilename[i]);
                    }
                }
                TapeHeader[CPC_TAPE_HEADER_DATA_LOGICAL_LENGTH_LOW] = (FileLengthRemaining & 0x0ff);
                TapeHeader[CPC_TAPE_HEADER_DATA_LOGICAL_LENGTH_HIGH] = (FileLengthRemaining>>8) & 0x0ff;


                BlockLocation = TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] |
                                (TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH]<<8);

                if (CPCMethod == CPC_METHOD_SPECTRUM)
                {
                    /* write data into block */
                    WriteStandardSpeedDataBlock(pTZXFile, 0x0ff, &pData[FileOffset], FileLengthRemaining, 1000);
                }
                else
                {
                    do
                    {
                        unsigned char Flag;
    /*
                        CPC can't handle this one
                        if (CPCMethod == CPC_METHOD_2BLOCKS)
                        {
                              if (FirstBlock)
                              {
                                    TapeBlockSize=CPC_DATA_BLOCK_SIZE;
                                    LastBlock = FALSE;
                              }
                              else
                              {
                                    TapeBlockSize = FileLengthRemaining;
                                    LastBlock = TRUE;
                              }
                        }
                        else
    */
                        if (CPCMethod == CPC_METHOD_BLOCKS)
                        {
                            /* calc size of tape data block */
                            if (FileLengthRemaining>CPC_DATA_BLOCK_SIZE)
                            {
                                TapeBlockSize = CPC_DATA_BLOCK_SIZE;
                                LastBlock = FALSE;
                            }
                            else
                            {
                                TapeBlockSize = FileLengthRemaining;
                                LastBlock = TRUE;
                            }
                        }
                        else
                        if (CPCMethod == CPC_METHOD_HEADERLESS)
                        {
                            TapeBlockSize = FileLengthRemaining;
                        }



                        /**** HEADER ****/
                        /* SETUP TAPE RELATED DATA */
                        /* block index */
                        TapeHeader[CPC_TAPE_HEADER_BLOCK_NUMBER] = BlockIndex;

                        /* first block? */
                        if (FirstBlock)
                        {
                            FirstBlock = FALSE;

                            Flag = 0x0ff;
                        }
                        else
                        {
                            Flag = 0;
                        }

                        TapeHeader[CPC_TAPE_HEADER_FIRST_BLOCK_FLAG] = Flag;

                        /* last block? */
                        if (LastBlock)
                        {
                            Flag = 0x0ff;
                        }
                        else
                        {
                            Flag = 0;
                        }

                        TapeHeader[CPC_TAPE_HEADER_LAST_BLOCK_FLAG] = Flag;

                        /* size of data following */
                        TapeHeader[CPC_TAPE_HEADER_DATA_LENGTH_LOW] = (unsigned char)TapeBlockSize;
                        TapeHeader[CPC_TAPE_HEADER_DATA_LENGTH_HIGH] = (unsigned char)(TapeBlockSize>>8);

                        /* location of block */
                        TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_LOW] = (unsigned char)BlockLocation;
                        TapeHeader[CPC_TAPE_HEADER_DATA_LOCATION_HIGH] = (unsigned char)(BlockLocation>>8);

                        /* don't write a header if headerless */
                        if (CPCMethod!=CPC_METHOD_HEADERLESS)
                        {
                            /* write header */
                            CPC_WriteDataBlock(pTZXFile, 0x02c, TapeHeader, CPC_TAPE_HEADER_SIZE,10);
                        }

                        /* write data into block */
                        CPC_WriteDataBlock(pTZXFile, 0x016, &pData[FileOffset], TapeBlockSize,CPC_PAUSE_AFTER_BLOCK_IN_MS);

                        BlockLocation+=TapeBlockSize;
                        BlockIndex++;
                        FileOffset+=TapeBlockSize;
                        FileLengthRemaining -= TapeBlockSize;
                    }
                    while (FileLengthRemaining!=0);
                }

                free(pData);
            }

			/* write file */
			if (BlankBeforeUse)
			{
                TZX_WriteFile(pTZXFile, pDestFilename);
			}
			else
			{
			    TZX_AppendFile(pTZXFile, pDestFilename);
			}

			/* free it */
			TZX_FreeFile(pTZXFile);
		}
		else
		{
			printf("Failed to open output file!\r\n");
			exit(1);
		}

	}

	exit(0);

	return 0;
}
