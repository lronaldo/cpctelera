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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
/* TZX file support */
#include "tzxfile.h"
/* header for TZX file */
const unsigned char *TZX_FileHeader = (const unsigned char *)"ZXTape!\x1a";

/* the internal format for the TZX is a link-list system.
Blocks can be edited. A new file can be written with the new data */



/**************************************/
/* insert block after block specified */
void	TZX_InsertBlockAfter(TZX_FILE *pFile,TZX_BLOCK *pBlock, TZX_BLOCK *pPrev)
{
	/* setup block pointers */
	pBlock->pPrev = pPrev;
	pBlock->pNext = pPrev->pNext;

	/* setup pointers for block before */
	pPrev->pNext = pBlock;
}

/***************************************/
/* insert block before block specified */
void	TZX_InsertBlockBefore(TZX_FILE *pFile, TZX_BLOCK *pBlock, TZX_BLOCK *pNext)
{
	/* setup block pointers */
	pBlock->pNext = pNext;
	pBlock->pPrev = pNext->pPrev;

	/* setup pointers for block before */
	pNext->pPrev = pBlock;

	/* is block we are inserting after the first in the list? */
	if (pFile->pFirstBlock == pNext)
	{
		/* yes. New block is now the start of the list */
		pFile->pFirstBlock = pBlock;
	}
}


/*****************************************/
/* insert a block at the end of the file */
void	TZX_AddBlockToEndOfFile(TZX_FILE *pFile, TZX_BLOCK *pBlock)
{
	/* blocks in file? */
	if (pFile->pFirstBlock==NULL)
	{
		/* no */
		pFile->pFirstBlock = pBlock;
	}
	else
	{
		/* yes */
		TZX_BLOCK *pCurrentBlock;

		/* search for last block in list */
		pCurrentBlock = pFile->pFirstBlock;

		while (pCurrentBlock->pNext!=NULL)
			pCurrentBlock = pCurrentBlock->pNext;

		TZX_InsertBlockAfter(pFile,pBlock, pCurrentBlock);
	}
}

/********************************/
/* detach block from block list */
void	TZX_DetachBlock(TZX_FILE *pFile,TZX_BLOCK *pBlock)
{
	/* block before this block? */
	if (pBlock->pPrev!=NULL)
	{
		/* yes */
		pBlock->pPrev->pNext = pBlock->pNext;
	}
	else
	{
		/* no, this block is first in list */
		pFile->pFirstBlock = pBlock->pNext;
	}

	/* block after this block? */
	if (pBlock->pNext!=NULL)
	{
		/* yes */
		pBlock->pNext->pPrev = pBlock->pPrev;
	}
}

/************************/
/* free data in a block */
void	TZX_FreeBlock(TZX_BLOCK *pBlock)
{
	/* free it */
	free(pBlock);
}



/******************************/
/* create a internal TZX file */

TZX_FILE *TZX_CreateFile(unsigned char VersionMajor, unsigned char VersionMinor)
{
	TZX_FILE *pTZXFile;

	/* alloc the header */
	pTZXFile = malloc(sizeof(TZX_FILE));

	if (pTZXFile!=NULL)
	{
		/* set version */
		pTZXFile->VersionMajor = VersionMajor;
		pTZXFile->VersionMinor = VersionMinor;

		/* initialise block linked list */
		pTZXFile->pFirstBlock = NULL;
	}

	return pTZXFile;
}

/*******************/
/* free a TZX file */

void	TZX_FreeFile(TZX_FILE *pFile)
{
	TZX_BLOCK *pBlock;

	pBlock = pFile->pFirstBlock;

	while (pBlock!=NULL)
	{
		TZX_BLOCK *pNextBlock = pBlock->pNext;

		/* remove block from list */
		TZX_DetachBlock(pFile, pBlock);

		/* free TZX block */
		TZX_FreeBlock(pBlock);

		pBlock = pNextBlock;
	}

	/* free TZX file header */
	free(pFile);
}

/*****************************************************************************************************/
/* write a TZX file */

void TZX_WriteBlocks(TZX_FILE *pTZXFile, FILE *fh)
{
    TZX_BLOCK *pBlock;

    /* get pointer to first block */
    pBlock = pTZXFile->pFirstBlock;

    /* write each block in turn to file */
    while (pBlock!=NULL)
    {
        if (pBlock->pBlockHeader!=NULL)
        {
            BOOL BlockHasData;
            int BlockHeaderSize;
            unsigned char BlockID;

            /* get block ID */
            BlockID = pBlock->pBlockHeader[0];
            /* get size of header */
            BlockHeaderSize = TZX_GetBlockHeaderSize(BlockID);
            /* does block have additional data ? */
            BlockHasData = TZX_BlockHasData(BlockID);

            if ((!BlockHasData) || ((BlockHasData) && (pBlock->pBlockData!=NULL)))
            {
                /* block requires data and has data, or block doesn't require data */

                /* write the header */
                fwrite(pBlock->pBlockHeader, BlockHeaderSize, sizeof(unsigned char), fh);

                if (pBlock->pBlockData!=NULL)
                {
                    /* write the data */

                    fwrite(pBlock->pBlockData, pBlock->DataBlockSize, sizeof(unsigned char), fh);
                }
            }
        }

        /* next block */
        pBlock = pBlock->pNext;
    }
}


void	TZX_AppendFile(TZX_FILE *pTZXFile, unsigned char *pFilename)
{
	FILE *fh;

	/* open TZX file */
	fh = fopen((const char *)pFilename,"r+b");

	if (fh!=NULL)
	{
		TZX_BLOCK *pBlock;

        fseek(fh, 0, SEEK_END);

        TZX_WriteBlocks(pTZXFile, fh);

		/* close TZX file */
		fclose(fh);
	}
}


void	TZX_WriteFile(TZX_FILE *pTZXFile, unsigned char *pFilename)
{
	FILE *fh;

	/* open TZX file */
	fh = fopen((const char *)pFilename,"wb");

	if (fh!=NULL)
	{
		TZX_BLOCK *pBlock;

		/* write header */
		fwrite(TZX_FileHeader, 8, sizeof(unsigned char), fh);
		/* write version numbers */
		fwrite(&pTZXFile->VersionMajor, 1, sizeof(unsigned char), fh);
		fwrite(&pTZXFile->VersionMinor, 1, sizeof(unsigned char), fh);

        TZX_WriteBlocks(pTZXFile, fh);

		/* close TZX file */
		fclose(fh);
	}
}

void	TZX_SetupPauseBlock(TZX_BLOCK *pBlock,unsigned long PauseInMilliseconds)
{
	unsigned char *pHeader = TZX_GetBlockHeaderPtr(pBlock);

	if (pHeader!=NULL)
	{
		if (pHeader[0] == TZX_PAUSE_BLOCK)
		{
			pHeader[1] = (unsigned char)PauseInMilliseconds;
			pHeader[2] = (unsigned char)(PauseInMilliseconds>>8);
		}
	}
}

/*****************************************************************************************************/

/* given a TZX block ID, this returns the size of the header */
int	TZX_GetBlockHeaderSize(unsigned char ID)
{
	switch (ID)
	{
		case TZX_STANDARD_SPEED_DATA_BLOCK:
			return 4+1;
		case TZX_TURBO_LOADING_DATA_BLOCK:
			return 18+1;

		case TZX_PAUSE_BLOCK:
			return 2+1;
		case TZX_PURE_DATA_BLOCK:
			return 0x0a + 1;
		case TZX_DIRECT_RECORDING_BLOCK:
			return 0x08 + 1;

		default:
			break;
	}

	return 0;
}

/*****************************************************************************************************/
/* return TRUE if the block has additional data, false if not */
BOOL TZX_BlockHasData(unsigned char ID)
{
	switch (ID)
	{
		case TZX_STANDARD_SPEED_DATA_BLOCK:
		case TZX_TURBO_LOADING_DATA_BLOCK:
		case TZX_PURE_DATA_BLOCK:
		case TZX_DIRECT_RECORDING_BLOCK:
			return TRUE;

		case TZX_PAUSE_BLOCK:
			return FALSE;

		default:
			break;
	}

	return FALSE;

}

/*****************************************************************************************************/
/* set block size */
void	TZX_SetBlockSizeInHeader(TZX_BLOCK *pBlock, unsigned long Size)
{
	unsigned char ID = pBlock->pBlockHeader[0];
	unsigned char *pBlockData;

	pBlockData = &pBlock->pBlockHeader[1];

	switch (ID)
	{
		case TZX_STANDARD_SPEED_DATA_BLOCK:
		{
			pBlockData[2] = (unsigned char)Size;
			pBlockData[3] = (unsigned char)(Size>>8);

		}
		break;

		case TZX_TURBO_LOADING_DATA_BLOCK:
		{
			pBlockData[0x0f] = (unsigned char)Size;
			pBlockData[0x010] = (unsigned char)(Size>>8);
			pBlockData[0x011] = (unsigned char)(Size>>16);

		}
		break;

		case TZX_PURE_DATA_BLOCK:
		{
			pBlockData[0x07] = (unsigned char)Size;
			pBlockData[0x08] = (unsigned char)(Size>>8);
			pBlockData[0x09] = (unsigned char)(Size>>16);
		}
		break;

		case TZX_DIRECT_RECORDING_BLOCK:
		{
			pBlockData[0x05] = (unsigned char)Size;
			pBlockData[0x06] = (unsigned char)(Size>>8);
			pBlockData[0x07] = (unsigned char)(Size>>16);
		}
		break;

		default:
			break;
	}
}


/*****************************************************************************************************/
/* create a block of the specified ID in the TZX image file */
TZX_BLOCK *TZX_CreateBlock(unsigned char ID)
{
	TZX_BLOCK *pBlock;

	pBlock = malloc(sizeof(TZX_BLOCK));

	if (pBlock!=NULL)
	{
		int BlockHeaderSize;

		/* reset block data */
		memset(pBlock, 0, sizeof(TZX_BLOCK));

		BlockHeaderSize = TZX_GetBlockHeaderSize(ID);

		pBlock->pBlockHeader = malloc(BlockHeaderSize);

		if (pBlock->pBlockHeader!=NULL)
		{
			pBlock->pBlockHeader[0] = ID;
		}
	}

	return pBlock;
}

/*****************************************************************************************************/
/* create a block of the specified ID in the TZX image file */
void	TZX_AddDataToBlock(TZX_BLOCK *pBlock, int DataSize)
{
	/* only add data if a header exists */
	if (pBlock->pBlockHeader!=NULL)
	{
		/* get block id */
		unsigned char BlockID = pBlock->pBlockHeader[0];

		/* does this block ID have additional data? */
		if (TZX_BlockHasData(BlockID))
		{
			/* allocate memory for the additional data */
			pBlock->pBlockData = malloc(DataSize);

			if (pBlock->pBlockData!=NULL)
			{
				pBlock->DataBlockSize = DataSize;

				/* set size in TZX header */
				TZX_SetBlockSizeInHeader(pBlock, DataSize);
			}
		}
	}
}

/*****************************************************************************************************/
/* get pointer to TZX data block */
unsigned char *TZX_GetBlockDataPtr(TZX_BLOCK *pBlock)
{
	return pBlock->pBlockData;
}

/*****************************************************************************************************/
/* get pointer to TZX data block */
unsigned char *TZX_GetBlockHeaderPtr(TZX_BLOCK *pBlock)
{
	return pBlock->pBlockHeader;
}
