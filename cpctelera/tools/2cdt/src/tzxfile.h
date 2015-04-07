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

#ifndef __TZX_FILE_HEADER__
#define __TZX_FILE_HEADER__

#include "defs.h"

#define TZX_VERSION_MAJOR	1
#define TZX_VERSION_MINOR	10

#define TZX_STANDARD_SPEED_DATA_BLOCK	0x010
#define TZX_TURBO_LOADING_DATA_BLOCK	0x011
#define TZX_PAUSE_BLOCK					0x020
#define TZX_PURE_DATA_BLOCK				0x014
#define TZX_DIRECT_RECORDING_BLOCK		0x015

#define TZX_T_STATES 3500000


#define TZX_HARDWARE_COMPUTER_CPC464	    0x015 /* Amstrad CPC 464 */
#define TZX_HARDWARE_COMPUTER_CPC664		0x016 /* Amstrad CPC 664 */
#define TZX_HARDWARE_COMPUTER_CPC6128		0x017 /* Amstrad CPC 6128 */
#define TZX_HARDWARE_COMPUTER_CPC464PLUS	0x018 /* Amstrad CPC 464+ */
#define TZX_HARDWARE_COMPUTER_CPC6128PLUS	0x019 /* Amstrad CPC 6128+ */

typedef struct TZX_BLOCK
{
	/* previous TZX block */
	struct TZX_BLOCK *pPrev;
	/* next TZX block */
	struct TZX_BLOCK *pNext;
	/* pointer to header data allocated for this block */
	unsigned char *pBlockHeader;
	/* size of data block */
	unsigned long DataBlockSize;
	/* pointer to data added to this block */
	unsigned char *pBlockData;
} TZX_BLOCK;

typedef struct TZX_FILE
{
	/* version of TZX file */
	unsigned char VersionMajor;
	unsigned char VersionMinor;
	/* pointer to first block */
	TZX_BLOCK *pFirstBlock;
} TZX_FILE;

void	TZX_WriteFile(TZX_FILE *pTZXFile, unsigned char *pFilename);
unsigned char *TZX_GetBlockDataPtr(TZX_BLOCK *);
unsigned char *TZX_GetBlockHeaderPtr(TZX_BLOCK *);

TZX_FILE *TZX_CreateFile(unsigned char VersionMajor, unsigned char VersionMinor);
void	TZX_FreeFile(TZX_FILE *pFile);
void	TZX_AddDataToBlock(TZX_BLOCK *pBlock, int DataSize);
unsigned char *TZX_GetDataBlockPtr(TZX_BLOCK *pBlock);
void	TZX_AddBlockToEndOfFile(TZX_FILE *pFile, TZX_BLOCK *pBlock);
int	TZX_GetBlockHeaderSize(unsigned char ID);
BOOL TZX_BlockHasData(unsigned char ID);
TZX_BLOCK *TZX_CreateBlock(unsigned char ID);
void	TZX_SetupPauseBlock(TZX_BLOCK *pBlock,unsigned long PauseInMilliseconds);
#endif

