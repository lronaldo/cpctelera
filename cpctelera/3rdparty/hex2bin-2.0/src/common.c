#include "common.h"

filetype    Filename;           /* string for opening files */
char        Extension[MAX_EXTENSION_SIZE];       /* filename extension for output files */

FILE        *Filin,             /* input files */
            *Filout;            /* output files */

#ifdef USE_FILE_BUFFERS
char		*FilinBuf,          /* text buffer for file input */
            *FiloutBuf;         /* text buffer for file output */
#endif

int Pad_Byte = 0xFF;
bool Enable_Checksum_Error = false;
bool Status_Checksum_Error = false;
byte 	Checksum;
unsigned int Record_Nb;

/* This will hold binary codes translated from hex file. */
byte *Memory_Block;
unsigned int Lowest_Address, Highest_Address;
unsigned int Starting_Address;
unsigned int Max_Length = 0;
unsigned int Minimum_Block_Size = 0x1000; // 4096 byte
int Module;
bool Minimum_Block_Size_Setted = false;
bool Starting_Address_Setted = false;
bool Max_Length_Setted = false;
bool Swap_Wordwise = false;

int Endian = 0;

t_CRC Cks_Type = CHK8_SUM;
unsigned int Cks_Start = 0, Cks_End = 0, Cks_Addr = 0, Cks_Value = 0;
bool Cks_range_set = false;
bool Cks_Addr_set = false;
bool Force_Value = false;

unsigned int Crc_Poly = 0x07, Crc_Init = 0, Crc_XorOut = 0;
bool Crc_RefIn = false;
bool Crc_RefOut = false;

void usage(void)
{
    fprintf (stderr,
             "\n"
             "usage: %s [OPTIONS] filename\n"
             "Options:\n"
             "  -c            Enable record checksum verification\n"
             "  -C [Poly][Init][RefIn][RefOut][XorOut]\n                CRC parameters"
             "  -e [ext]      Output filename extension (without the dot)\n"
             "  -E [0|1]      Endian for checksum/CRC, 0: little, 1: big\n"
             "  -f [address]  Address of check result to write\n"
             "  -F [address] [value]\n                Address and value to force\n"
             "  -k [0-4]      Select check method (checksum or CRC) and size\n"
             "  -d            display list of check methods/value size\n"
             "  -l [length]   Maximal Length (Starting address + Length -1 is Max Address)\n"
             "                File will be filled with Pattern until Max Address is reached\n"
             "  -m [size]     Minimum Block Size\n"
             "                File Size Dimension will be a multiple of Minimum block size\n"
             "                File will be filled with Pattern\n"
             "                Length must be a power of 2 in hexadecimal [see -l option]\n"
             "                Attention this option is STRONGER than Maximal Length  \n"
             "  -p [value]    Pad-byte value in hex (default: %x)\n"
             "  -r [start] [end]\n"
             "                Range to compute checksum over (default is min and max addresses)\n"
             "  -s [address]  Starting address in hex (default: 0)\n"
             "  -w            Swap wordwise (low <-> high)\n\n",
             Pgm_Name,Pad_Byte);
    exit(1);
} /* procedure USAGE */

void DisplayCheckMethods(void)
{
    fprintf (stderr,
             "Check methods/value size:\n"
             "0:  Checksum  8-bit\n"
             "1:  Checksum 16-bit\n"
             "2:  CRC8\n"
             "3:  CRC16\n"
             "4:  CRC32\n");
    exit(1);
}

#define LAST_CHECK_METHOD 4

void *NoFailMalloc (size_t size)
{
    void *result;

    if ((result = malloc (size)) == NULL)
    {
        fprintf (stderr,"Can't allocate memory.\n");
        exit(1);
    }
    return (result);
}

/* Open the input file, with error checking */
void NoFailOpenInputFile (char *Flnm)
{
    while ((Filin = fopen(Flnm,"r")) == NULL)
    {
        fprintf (stderr,"Input file %s cannot be opened. Enter new filename: ",Flnm);
        if (fgets (Flnm,MAX_FILE_NAME_SIZE,stdin) == NULL) exit(1); /* modified error checking */

        if (Flnm[strlen(Flnm) - 1] == '\n') Flnm[strlen(Flnm) - 1] = '\0';
    }

#ifdef USE_FILE_BUFFERS
    FilinBuf = (char *) NoFailMalloc (BUFFSZ);
    setvbuf(Filin, FilinBuf, _IOFBF, BUFFSZ);
#endif
} /* procedure OPENFILIN */

/* Open the output file, with error checking */
void NoFailOpenOutputFile (char *Flnm)
{
    while ((Filout = fopen(Flnm,"wb")) == NULL)
    {
        /* Failure to open the output file may be
         simply due to an insufficiant permission setting. */
        fprintf(stderr,"Output file %s cannot be opened. Enter new file name: ", Flnm);
        if (fgets(Flnm,MAX_FILE_NAME_SIZE,stdin) == NULL) exit(1);

        if (Flnm[strlen(Flnm) - 1] == '\n') Flnm[strlen(Flnm) - 1] = '\0';
    }

#ifdef USE_FILE_BUFFERS
    FiloutBuf = (char *) NoFailMalloc (BUFFSZ);
    setvbuf(Filout, FiloutBuf, _IOFBF, BUFFSZ);
#endif

} /* procedure OPENFILOUT */

void GetLine(char* str,FILE *in)
{
    char *result;

    result = fgets(str,MAX_LINE_SIZE,in);
    if ((result == NULL) && !feof (in)) fprintf(stderr,"Error occurred while reading from file\n");
}

// 0 or 1
int GetBin(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%u",&value);

    if (result == 1) return value & 1;
    else
    {
        fprintf(stderr,"GetBin: some error occurred when parsing options.\n");
        exit (1);
    }
}

int GetDec(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%u",&value);

    if (result == 1) return value;
    else
    {
        fprintf(stderr,"GetDec: some error occurred when parsing options.\n");
        exit (1);
    }
}

int GetHex(const char *str)
{
    int result;
    unsigned int value;

    result = sscanf(str,"%x",&value);

    if (result == 1) return value;
    else
    {
        fprintf(stderr,"GetHex: some error occurred when parsing options.\n");
        exit (1);
    }
}

// Char t/T: true f/F: false
bool GetBoolean(const char *str)
{
    int result;
    unsigned char value, temp;

    result = sscanf(str,"%c",&value);
    temp = tolower(value);

    if ((result == 1) && ((temp == 't') || (temp == 'f')))
    {
        return (temp == 't');
    }
    else
    {
        fprintf(stderr,"GetBoolean: some error occurred when parsing options.\n");
        exit (1);
    }
}

void GetExtension(const char *str,char *ext)
{
    if (strlen(str) > MAX_EXTENSION_SIZE)
        usage();

    strcpy(ext, str);
}

/* Adds an extension to a file name */
void PutExtension(char *Flnm, char *Extension)
{
    char        *Period;        /* location of period in file name */
    bool     Samename;

    /* This assumes DOS like file names */
    /* Don't use strchr(): consider the following filename:
     ../my.dir/file.hex
    */
    if ((Period = strrchr(Flnm,'.')) != NULL)
        *(Period) = '\0';

    Samename = false;
    if (strcmp(Extension, Period+1) == 0)
        Samename = true;

    strcat(Flnm,".");
    strcat(Flnm, Extension);
    if (Samename)
    {
        fprintf (stderr,"Input and output filenames (%s) are the same.", Flnm);
        exit(1);
    }
}

void VerifyChecksumValue(void)
{
    if ((Checksum != 0) && Enable_Checksum_Error)
	{
		fprintf(stderr,"Checksum error in record %d: should be %02X\n",
			Record_Nb, (256 - Checksum) & 0xFF);
		Status_Checksum_Error = true;
	}
}

void CrcParamsCheck(void)
{
    switch (Cks_Type)
    {
    case CRC8:
        Crc_Poly &= 0xFF;
        Crc_Init &= 0xFF;
        Crc_XorOut &= 0xFF;
        break;
    case CRC16:
        Crc_Poly &= 0xFFFF;
        Crc_Init &= 0xFFFF;
        Crc_XorOut &= 0xFFFF;
        break;
    case CRC32:
        break;
    default:
        fprintf (stderr,"See file CRC list.txt for parameters\n");
        exit(1);
    }
}

void WriteMemBlock16(uint16_t Value)
{
    if (Endian == 1)
    {
        Memory_Block[Cks_Addr - Lowest_Address]    = u16_hi(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u16_lo(Value);
    }
    else
    {
        Memory_Block[Cks_Addr - Lowest_Address +1] = u16_hi(Value);
        Memory_Block[Cks_Addr - Lowest_Address]    = u16_lo(Value);
    }
}

void WriteMemBlock32(uint32_t Value)
{
    if (Endian == 1)
    {
        Memory_Block[Cks_Addr - Lowest_Address]    = u32_b3(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u32_b2(Value);
        Memory_Block[Cks_Addr - Lowest_Address +2] = u32_b1(Value);
        Memory_Block[Cks_Addr - Lowest_Address +3] = u32_b0(Value);
    }
    else
    {
        Memory_Block[Cks_Addr - Lowest_Address +3] = u32_b3(Value);
        Memory_Block[Cks_Addr - Lowest_Address +2] = u32_b2(Value);
        Memory_Block[Cks_Addr - Lowest_Address +1] = u32_b1(Value);
        Memory_Block[Cks_Addr - Lowest_Address]    = u32_b0(Value);
    }
}

void WriteMemory(void)
{
    if ((Cks_Addr >= Lowest_Address) || (Cks_Addr < Highest_Address))
    {
        if(Force_Value)
        {
            switch (Cks_Type)
            {
                case 0:
                    Memory_Block[Cks_Addr - Lowest_Address] = Cks_Value;
                    fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, Cks_Value);
                    break;
                case 1:
                    WriteMemBlock16(Cks_Value);
                    fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, Cks_Value);
                    break;
                case 2:
                    WriteMemBlock32(Cks_Value);
                    fprintf(stdout,"Addr %08X set to %08X\n",Cks_Addr, Cks_Value);
                    break;
                default:;
            }
        }
        else if (Cks_Addr_set)
        {
            /* Add a checksum to the binary file */
            if (!Cks_range_set)
            {
                Cks_Start = Lowest_Address;
                Cks_End = Highest_Address;
            }
            /* checksum range MUST BE in the array bounds */

            if (Cks_Start < Lowest_Address)
            {
                fprintf(stdout,"Modifying range start from %X to %X\n",Cks_Start,Lowest_Address);
                Cks_Start = Lowest_Address;
            }
            if (Cks_End > Highest_Address)
            {
                fprintf(stdout,"Modifying range end from %X to %X\n",Cks_End,Highest_Address);
                Cks_End = Highest_Address;
            }

            switch (Cks_Type)
            {
            case CHK8_SUM:
            {
                uint8_t wCKS = 0;

                for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                {
                    wCKS += Memory_Block[i - Lowest_Address];
                }

                fprintf(stdout,"8-bit Checksum = %02X\n",wCKS & 0xff);
                Memory_Block[Cks_Addr - Lowest_Address] = wCKS;
                fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, wCKS);
            }
            break;

            case CHK16:
            {
                uint16_t wCKS, w;

                wCKS = 0;

                if (Endian == 1)
                {
                    for (unsigned int i=Cks_Start; i<=Cks_End; i+=2)
                    {
                        w =  Memory_Block[i - Lowest_Address +1] | ((word)Memory_Block[i - Lowest_Address] << 8);
                        wCKS += w;
                    }
                }
                else
                {
                    for (unsigned int i=Cks_Start; i<=Cks_End; i+=2)
                    {
                        w =  Memory_Block[i - Lowest_Address] | ((word)Memory_Block[i - Lowest_Address +1] << 8);
                        wCKS += w;
                    }
                }
                fprintf(stdout,"16-bit Checksum = %04X\n",wCKS);
                WriteMemBlock16(wCKS);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, wCKS);
            }
            break;

            case CRC8:
            {
                uint8_t CRC8;
                crc_table = NoFailMalloc(256);

                if (Crc_RefIn)
                {
                    init_crc8_reflected_tab(Reflect8[Crc_Poly]);
                    CRC8 = Reflect8[Crc_Init];
                }
                else
                {
                    init_crc8_normal_tab(Crc_Poly);
                    CRC8 = Crc_Init;
                }

                for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                {
                    CRC8 = update_crc8(CRC8,Memory_Block[i - Lowest_Address]);
                }

                CRC8 = (CRC8 ^ Crc_XorOut) & 0xff;
                Memory_Block[Cks_Addr - Lowest_Address] = CRC8;
                fprintf(stdout,"Addr %08X set to %02X\n",Cks_Addr, CRC8);
            }
            break;

            case CRC16:
            {
                uint16_t CRC16;
                crc_table = NoFailMalloc(256 * 2);

                if (Crc_RefIn)
                {
                    init_crc16_reflected_tab(Reflect16(Crc_Poly));
                    CRC16 = Reflect16(Crc_Init);

                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC16 = update_crc16_reflected(CRC16,Memory_Block[i - Lowest_Address]);
                    }
                }
                else
                {
                    init_crc16_normal_tab(Crc_Poly);
                    CRC16 = Crc_Init;


                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC16 = update_crc16_normal(CRC16,Memory_Block[i - Lowest_Address]);
                    }
                }

                CRC16 = (CRC16 ^ Crc_XorOut) & 0xffff;
                WriteMemBlock16(CRC16);
                fprintf(stdout,"Addr %08X set to %04X\n",Cks_Addr, CRC16);
            }
            break;

            case CRC32:
            {
                uint32_t CRC32;

                crc_table = NoFailMalloc(256 * 4);
                if (Crc_RefIn)
                {
                    init_crc32_reflected_tab(Reflect32(Crc_Poly));
                    CRC32 = Reflect32(Crc_Init);

                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC32 = update_crc32_reflected(CRC32,Memory_Block[i - Lowest_Address]);
                    }
                }
                else
                {
                    init_crc32_normal_tab(Crc_Poly);
                    CRC32 = Crc_Init;

                    for (unsigned int i=Cks_Start; i<=Cks_End; i++)
                    {
                        CRC32 = update_crc32_normal(CRC32,Memory_Block[i - Lowest_Address]);
                    }
                }

                CRC32 ^= Crc_XorOut;
                WriteMemBlock32(CRC32);
                fprintf(stdout,"Addr %08X set to %08X\n",Cks_Addr, CRC32);
            }
            break;

            default:
                ;
            }

            free(crc_table);
        }
    }
    else
    {
        fprintf (stderr,"Force/Check address outside of memory range\n");
    }

    /* write binary file */
    fwrite (Memory_Block,
            Max_Length,
            1,
            Filout);

    free (Memory_Block);

    // Minimum_Block_Size is set; the memory buffer is multiple of this?
    if (Minimum_Block_Size_Setted==true)
    {
        Module = Max_Length % Minimum_Block_Size;
        if (Module)
        {
            Memory_Block = (byte *) NoFailMalloc(Module);
            memset (Memory_Block,Pad_Byte,Module);
            fwrite (Memory_Block,
                    Module,
                    1,
                    Filout);
            free (Memory_Block);
            if (Max_Length_Setted==true)
                fprintf(stdout,"Attention Max Length changed by Minimum Block Size\n");
        }
    }
}
