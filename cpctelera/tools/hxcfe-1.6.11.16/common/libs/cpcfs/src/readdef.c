/* 22disk .def format definition file reader.
(c) May 2000, Kevin Thacker  (amstrad@aiind.upv.es)
This file is part of the CPCfs update.
Derik van Zuetphen has full permission to use this in a full public release
of CPCfs. 
If you would like to use this source in your own programs,
please keep the copyright notice present and please document it in your
program. I would also be interested in hearing from you if you are using it. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif

/*#define DEBUG */

#include "cpcfs.h"

typedef int BOOL;

/* buffer to hold number strings and keywords */
static char ParseBuffer[256];

typedef enum
{
	ELEMENT_NUMBER,	/* number */
	ELEMENT_KEYWORD,	/* operator */
	ELEMENT_STRING,
	ELEMENT_IGNORE,
	ELEMENT_EOL,
	ELEMENT_COMMA
} ELEMENT_ID;
	
typedef struct ELEMENT
{
	ELEMENT_ID Type;
	int Value;
} ELEMENT;

typedef struct PARSE_STATUS
{
	unsigned char *pString;
} PARSE_STATUS;

typedef struct PARSE_LOOKUP
{
	/* id */
	int ID;
	/* string representation */
	char *pString;
} PARSE_LOOKUP;

#undef KEYWORD
#define KEYWORD(x)		PARSE_KEYWORD_##x

typedef enum
{
#include "keywords.h"
} KEYWORD_ID;

#undef KEYWORD
#define KEYWORD(x)	{PARSE_KEYWORD_##x, #x}

PARSE_LOOKUP	Keywords[]=
{
#include "keywords.h"
};

static void Parse_Init(PARSE_STATUS *pParse, char *line)
{
	pParse->pString = line;
}

static int	Parse__LookupID(unsigned char *pString, PARSE_LOOKUP *pTable, int NumEntries)
{
	int i;

	for (i=0; i<NumEntries; i++)
	{
		if (strcmp(pTable[i].pString, pString)==0)
		{
			return pTable[i].ID;
		}
	}

	return -1;
}

static int	Parse__LookupKeyword(unsigned char *pKeyword)
{
	return Parse__LookupID(pKeyword, Keywords, sizeof(Keywords)/sizeof(PARSE_LOOKUP));
}

/*-------------------------------------*/
static BOOL	Parse__IsAKeywordChar(char ch)
{
	return (isalpha(ch) || isdigit(ch) || (ch=='&'));
}

/* is a digit */
static BOOL Parse__IsDigit(char ch)
{
	if ((ch>='0') && (ch<='9'))
		return TRUE;

	return FALSE;
}

/* is a hex digit */
static BOOL Parse__IsHexDigit(char ch)
{
	unsigned char upper_ch;

	if (Parse__IsDigit(ch))
		return TRUE;

	upper_ch = toupper(ch);

	if ((upper_ch>='A') && (upper_ch<='F'))
		return TRUE;

	return FALSE;
}

static int  Parse__ConvertDigitToNumber(char ch)
{
	if (isdigit(ch))
	{
		return ch-'0';
	}
	return 0;
}

static int	Parse__ConvertHexDigitToNumber(char ch)
{
	int Number;


	Number = toupper(ch)-'0';

	if (Number>=10)
	{
		Number -= ('A'-'0'-10);
	}

	return Number;
}


static int	Parse__ConvertBinaryDigitToNumber(char ch)
{
	if ((ch=='0') || (ch=='1'))
		return ch - '0';

	return 0;
}


static unsigned char Parse__GetChar(PARSE_STATUS *pParse)
{
	char ch;

	ch = pParse->pString[0];
	pParse->pString++;

	return ch;
}

static void Parse__BackChar(PARSE_STATUS *pParse)
{
	pParse->pString--;
}


static unsigned long	Parse__ReadNumber(PARSE_STATUS *pParse)
{
	char ch;
	unsigned long Number;
	BOOL IsAHexDigit;
	int Index;

	Index = 0;
	do
	{
		/* get char */
		ch = Parse__GetChar(pParse);
		
		/* is a hex digit? */
		IsAHexDigit = Parse__IsHexDigit(ch);
		
		/* yes */
		if (IsAHexDigit)
		{
			ParseBuffer[Index] = toupper(ch);
			Index++;

		}
	}
	while ((IsAHexDigit) && (ch!='\0'));

	/* hex number has h on end */
	if (toupper(ch)=='H')
	{
		int i;

		Number = 0;

		for (i=0; i<Index; i++)
		{
			Number = Number<<4;
			/* convert to hex and add to current number */
			Number |= Parse__ConvertHexDigitToNumber(ParseBuffer[i]);
		}
	}
	else if (toupper(ch)=='B')
	{
		int i;

		Number = 0;

		for (i=0; i<Index; i++)
		{
			Number = Number<<1;
			/* convert to hex and add to current number */
			Number |= Parse__ConvertBinaryDigitToNumber(ParseBuffer[i]);
		}
	}
	else
	{
		int i;

		Number = 0;
		for (i=0; i<Index; i++)
		{
			Number = Number*10;
			Number = Number + Parse__ConvertDigitToNumber(ParseBuffer[i]);
		}
		 
	 	Parse__BackChar(pParse);
	}

	


	return Number;
}

static unsigned char *Parse__ReadKeyword(PARSE_STATUS *pParse)
{
	char ch;
	BOOL IsAKeywordChar;
	int Index;
	
	Index = 0;

	do
	{
		/* get char */
		ch = Parse__GetChar(pParse);
	
		/* is a keyword char? */
		IsAKeywordChar = Parse__IsAKeywordChar(ch);

		/* yes */
		if (IsAKeywordChar)
		{
			ParseBuffer[Index] = toupper(ch);
			Index++;

		}
	}
	while (IsAKeywordChar);

	ParseBuffer[Index] = '\0';

	Parse__BackChar(pParse);
	

	return ParseBuffer;
}

static void	Parse_Get(PARSE_STATUS *pParse, ELEMENT *pElement)
{
	char ch;

	do
	{
		ch = Parse__GetChar(pParse);
		Parse__BackChar(pParse);
		
		if (ch=='\0')
		{
			pElement->Type = ELEMENT_EOL;
		}
		else
		if (ch==',')
		{
			Parse__GetChar(pParse);
			pElement->Type = ELEMENT_COMMA;
		}
		else
		if (ch==' ')
		{
			/* space */
			do
			{
				ch = Parse__GetChar(pParse);
			}
			while (ch==' ');

			Parse__BackChar(pParse);
		
	   		
			pElement->Type = ELEMENT_IGNORE;
			pElement->Value = 0;
		}
		else
		if ((ch==13) || (ch==10))
		{
			do
			{
				ch = Parse__GetChar(pParse);
			}
			while ((ch==13) || (ch==10));

			Parse__BackChar(pParse);

			pElement->Type = ELEMENT_IGNORE;
			pElement->Value = 0;
		}
		else if (isdigit(ch))
		{
			/* number */
			unsigned long Number;

			Number = Parse__ReadNumber(pParse);
			
			pElement->Type = ELEMENT_NUMBER;
			pElement->Value = Number;
		}
		else
		if (isalpha(ch))
		{
			/* keyword */
			unsigned long OperatorID;
			unsigned char *Operator;

			Operator = Parse__ReadKeyword(pParse);
						 
			OperatorID = Parse__LookupKeyword(Operator);

			if (OperatorID!=-1)
			{
		 		pElement->Type = ELEMENT_KEYWORD;
				pElement->Value = OperatorID;
			}
			else
			{
				pElement->Type = ELEMENT_STRING;
				pElement->Value = (int)Operator;
			}
		}
	}
	while (pElement->Type==ELEMENT_IGNORE);
}

extern DPB_list dpb_list;

static char Description[256];
static char Ident[256];
static DPB_type temp_dpb;

void	dpb_list_add_item(DPB_list *list,char *description, char *ident, DPB_type *dpb);
DPB_list_entry	*dpb_check_if_matching_format_exists(DPB_list *list, DPB_type *type);

void  parse_def_file(char *Filename)
{
	FILE *fh;

	// open file
	fh = fopen(Filename,"rb");

	if (fh!=NULL)
	{
		int BeginEntered;
		char *result;
		char line[256];
		int quit = 0;

		BeginEntered = 0;
		do
		{
			ELEMENT ParseItem;
			PARSE_STATUS ParseStatus;

			// get a line from the file
			result = fgets(line, 255, fh);

			/* initialise parser for this line */
			Parse_Init(&ParseStatus, line);
		
			if (result!=NULL)
			{
				do
				{
					/* get item */
					Parse_Get(&ParseStatus, &ParseItem);
		
					/* did we get a token? */
					if (ParseItem.Type == ELEMENT_KEYWORD)
					{
						switch (ParseItem.Value)
						{
							case PARSE_KEYWORD_END:
							{
								DPB_list_entry *list_entry;

								if (!BeginEntered)
								{


								}

								BeginEntered = 0;

								/* do not allow bad skews! */
								if (
									/* skew will not change index, because the increment is 0 */
									(temp_dpb.skew==0)
									)
#if 0
									|| 
									/* skew will change index, but results in the same index again! */
									((((1+temp_dpb.skew)%dpb->SECS)==1)) && (dpb->SECS!=1))
#endif
								{
									temp_dpb.skew = 1;
								}

								list_entry = dpb_check_if_matching_format_exists(&dpb_list, &temp_dpb);

								if (list_entry==NULL)
								{
									dpb_list_add_item(&dpb_list,Description, Ident,&temp_dpb);


								}
#ifdef DEBUG
								else
								{
									printf("format %s matches %s\r\n", Ident, list_entry->ident);
								}
#endif
							}
							break;

							case PARSE_KEYWORD_BEGIN:
							{
								if (ParseItem.Value == PARSE_KEYWORD_BEGIN)
								{
									int index;
									char *pString;
									char ch;

									if (BeginEntered)
									{
										


									}


									Parse_Get(&ParseStatus, &ParseItem);
								
									ParseItem.Type = ELEMENT_EOL;
								
									BeginEntered = 1;

									/* initialise defaults */
									temp_dpb.order = ORDER_SIDES;
									temp_dpb.side0_hd = 0;
									temp_dpb.side1_hd = 1;
									temp_dpb.skew = 1;

									/* ident */
									strncpy(Ident, (char *)ParseItem.Value, 256);
									Ident[255] = '\0';

									/* get description */
									
									pString = ParseStatus.pString;

									do
									{
									
										ch = pString[0];
										pString++;
									}
									while (ch==' ');

									pString--;

									index = 0;
									do
									{
										
										ch = pString[0];
										pString++;

										if ((ch!=0) && (ch!=10) && (ch!=13))
										{
											Description[index] = ch;
											index++;
										}
										else
										{
											break;
										}
									}
									while (index<255);
									Description[index] = '\0';

									Description[255] = '\0';
								
									memset(&temp_dpb, 0, sizeof(DPB_type));

								}
							}
							break;

							case PARSE_KEYWORD_BSH:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.BSH = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_BLM:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.BLM = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_EXM:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.EXM = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_DSM:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.DSM = ParseItem.Value;
							

								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_DRM:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.DRM = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_AL0:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.AL0 = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_AL1:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.AL1 = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_OFS:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.OFS = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							/* not used by CPCfs */
							case PARSE_KEYWORD_SOFS:
							{
								Parse_Get(&ParseStatus, &ParseItem);
							}
							break;

							/* for 22disk cylinders appears to specify physical drive tracks */
							case PARSE_KEYWORD_CYLINDERS:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.TRKS = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;
							
							case PARSE_KEYWORD_SIDES:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.HDS = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_SKEW:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								/* not used by CPCfs */
								temp_dpb.skew = ParseItem.Value;
							}
							break;

							case PARSE_KEYWORD_SECTORS:
							{
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.SECS = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}

								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type != ELEMENT_COMMA)
								{
									quit = 1;
									break;
								}

								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type == ELEMENT_NUMBER)
								{
									temp_dpb.BPS = ParseItem.Value;
								}
								else
								{
									quit = 1;
									break;
								}
							}
							break;

							case PARSE_KEYWORD_DENSITY:
							{
								/* not used in CPCfs */
								
								/* mfm or fm */
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type != ELEMENT_KEYWORD)
								{
									quit = 1;
									break;
								}

								/* comma */
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type!=ELEMENT_COMMA)
								{
									quit = 1;
									break;
								}
								
								/* low or high */
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type!=ELEMENT_KEYWORD)
								{
									quit = 1;
									break;
								}
							}
							break;

							/* not used by CPCfs */
						 	case PARSE_KEYWORD_COMPLEMENT:
							{
							}
							break;

							case PARSE_KEYWORD_NOTE:
							{
								/* pretend we got end of line and quit out */
								ParseItem.Type = ELEMENT_EOL;
					  		}
							break;

							/* not used by CPCfs */
							case PARSE_KEYWORD_LABEL:
							{
								Parse_Get(&ParseStatus, &ParseItem);
					  		}
							break;

							/* in CPCfs both sides must use the same ID's.
							  Side1 and Side2 are treated the same therefore */
							/* we are looking for the lowest ID index */
							case PARSE_KEYWORD_SIDE1:
							case PARSE_KEYWORD_SIDE2:
							{
								int SideKeyword = ParseItem.Value;								
								int ID;

								/* get head index */
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type!=ELEMENT_NUMBER)
								{
									quit = 1;
									break;
								}
								
								switch (SideKeyword)
								{
									case PARSE_KEYWORD_SIDE1:
									{
										temp_dpb.side0_hd = ParseItem.Value;
									}
									break;

									case PARSE_KEYWORD_SIDE2:
									{
										temp_dpb.side1_hd = ParseItem.Value;
									}
									break;
								
									default:
										break;
								}


								/* this list reflects software interleaving */
								/* get first id in list */
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type!=ELEMENT_NUMBER)
								{
									quit = 1;
									break;
								}

								ID = ParseItem.Value;

								do
								{
									/* get next */
									/* should be end of line or comma */
									Parse_Get(&ParseStatus, &ParseItem);

									/* end of line */
									if (ParseItem.Type==ELEMENT_EOL)
										break;

									/* not a comma! */
									if (ParseItem.Type!=ELEMENT_COMMA)
									{	quit = 1;
										break;
									}
									/* get number */
									Parse_Get(&ParseStatus, &ParseItem);

									if (ParseItem.Type!=ELEMENT_NUMBER)
									{
										quit = 1;
										break;
									}
									if (ParseItem.Value<ID)
									{
										ID = ParseItem.Value;
									}
								}
								while (1==1);

								switch (SideKeyword)
								{
									default:
									case PARSE_KEYWORD_SIDE1:
									{
										temp_dpb.SEC1_side1 = ID;
									}
									break;

									case PARSE_KEYWORD_SIDE2:
									{
										temp_dpb.SEC1_side2 = ID;
									}
									break;
								}

							}
							break;
							
							/* not used in CPCfs */
							case PARSE_KEYWORD_ORDER:
							{
								 /* cylinders, sides, eagle */
								Parse_Get(&ParseStatus, &ParseItem);

								if (ParseItem.Type!=ELEMENT_KEYWORD)
								{
									quit = 1;
									break;
								}
								
								switch (ParseItem.Value)
								{

									default:
									case PARSE_KEYWORD_SIDES:
									{
										temp_dpb.order = ORDER_SIDES;
									}
									break;

									case PARSE_KEYWORD_CYLINDERS:
									{
										temp_dpb.order = ORDER_CYLINDERS;
									}
									break;

									case PARSE_KEYWORD_EAGLE:
									{
										temp_dpb.order = ORDER_EAGLE;
									}
									break;
								}
							}
							break;
						}
					}
					/* continue until end of line */
				}
				while ((ParseItem.Type!=ELEMENT_EOL) && (quit==0));
			}

		}
		while ((result!=0) && (quit==0));

		// close file
		fclose(fh);
	}
}

#if 0
void	main()
{
	parse_def_file("cpmdisks.def");
}
#endif