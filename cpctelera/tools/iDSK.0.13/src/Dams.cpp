#include <iostream>
#include <cstdio>
#include <string.h>
using namespace std;


//
// Convertir le buffer en listing au format Dams
// Adaptation des sources de Thierry JOUIN ( Ramlaid )
//
void Dams( unsigned char * BufFile, int TailleFic, char * Listing )
{
    const char * MotCleDams[ 0x80 ] = 
        {
        "LD","INC","DEC","ADD","ADC","SUB","SBC","AND","XOR","OR","CP",
        "PUSH","POP","BIT","RES","SET","RLC","RRC","RL","RR","SLA","SRA",
        "SRL","IN","OUT","RST","DJNZ","EX","IM","JR","CALL","RET","JP",
        "NOP","RLCA","RRCA","RLA","RRA","DAA","CPL","SCF","CCF","HALT",
        "EXX","DI","EI","NEG","RETN","RETI","RRD","RLD","LDI","CPI","INI",
        "OUTI","LDD","CPD","IND","OUTD","LDIR","CPIR","INIR","OTIR","LDDR",
        "CPDR","INDR","OTDR","DB","DW","DM","DS","EQU","ORG","ENT",
        "IF","ELSE","END"
        };
    char Tmp[ 32 ];
    int PosFile = 0;
    int PosDest = 0;
    unsigned char c;

    * Listing = 0;
    c = BufFile[ PosFile++ ];
    while( c )
        {
        if ( c == 0xFF )
            {
            // Commentaire ligne
            Listing[ PosDest++ ] = ';';
            c = BufFile[ PosFile++ ];
            while( c != 0x0D && PosFile < TailleFic )
                {
                Listing[ PosDest++ ] = c;
                c = BufFile[ PosFile++ ];
                }
            Listing[ PosDest++ ] = '\r';
            Listing[ PosDest++ ] = '\n';
            }
        else
            {
            if ( c >= 0x80 && c != 0x0D )
                {
                // Mnemonique sans label
                // ENT
                if ( c == 0xC9 )
                    Listing[ PosDest++ ] = ';';

                sprintf( Tmp, "\t%s\t", MotCleDams[ c & 0x7F ] );
                int l = strlen( Tmp );
                memcpy( &Listing[ PosDest ], Tmp, l );
                PosDest += l;
                // DS ?,?
                if ( c == 0xC6 )
                    {
                    c = BufFile[ PosFile++ ];
                    // Fin de ligne
                    while( c != 0x0D && PosFile < TailleFic )
                        {
                        if ( c == ',' )
                            {
                            while( c != 0x0D && c != 0xFF && PosFile < TailleFic )
                                c = BufFile[ PosFile++ ];
                            }
                        if ( c != 0x0D )
                            {
                            if ( c == 0xFF )
                                Listing[ PosDest++ ] = '\t';
                            else          
                                Listing[ PosDest++ ] = c;

                            c = BufFile[ PosFile++ ];
                            }
                        }
                    }
                else
                    {
                    c = BufFile[ PosFile++ ];
                    // Fin de ligne
                    while( c != 0x0D && PosFile < TailleFic )
                        {
                        if ( c == 0xFF )
                            Listing[ PosDest++ ] = '\t';
                        else          
                                Listing[ PosDest++ ] = c;

                        c = BufFile[ PosFile++ ];
                        }
                    }
                Listing[ PosDest++ ] = '\r';
                Listing[ PosDest++ ] = '\n';
                }
            else
                {
                // Label
                while( c < 0x80 && c != 0x0D && PosFile < TailleFic )
                    {
                    Listing[ PosDest++ ] = c;
                    c = BufFile[ PosFile++ ];
                    }
                if ( c != 0x0D )
                    {
                    // Mnemonique apres label
                    // ENT
                    if ( c == 0xC9 )
                        Listing[ PosDest++ ] = ';';

                    if ( c != 0xFF )
                        {
                        sprintf( Tmp, "\t%s\t", MotCleDams[ c & 0x7F ] );
                        int l = strlen( Tmp );
                        memcpy( &Listing[ PosDest ], Tmp, l );
                        PosDest += l;
                        }
                    else
                        {
                        Listing[ PosDest++ ] = '\t';
                        Listing[ PosDest++ ] = '\t';
                        Listing[ PosDest++ ] = '\t';
                        }
                    // DS ?,?
                    if ( c == 0xC6 )
                        {
                        c = BufFile[ PosFile++ ];
                        // Fin de ligne
                        while( c != 0x0D && PosFile < TailleFic )
                            {
                            if ( c == ',' )
                                {
                                while( c != 0x0D && c != 0xFF && PosFile < TailleFic )
                                    c = BufFile[ PosFile++ ];
                                }
                            if ( c != 0x0D )
                                {
                                if ( c == 0xFF )
                                    {
                                    Listing[ PosDest++ ] = '\t';
                                    Listing[ PosDest++ ] = ';';
                                    }
                                else          
                                    Listing[ PosDest++ ] = c;

                                c = BufFile[ PosFile++ ];
                                }
                            }
                        }
                    else
                        {
                        c = BufFile[ PosFile++ ];
                        // Fin de ligne
                        while( c != 0x0D && PosFile < TailleFic )
                            {
                            if ( c == 0xFF )
                                {
                                Listing[ PosDest++ ] = '\t';
                                Listing[ PosDest++ ] = ';';
                                }
                            else          
                                Listing[ PosDest++ ] = c;

                            c = BufFile[ PosFile++ ];
                            }
                        }
                    Listing[ PosDest++ ] = '\r';
                    Listing[ PosDest++ ] = '\n';
                    }
                else
                    {
                    Listing[ PosDest++ ] = '\r';
                    Listing[ PosDest++ ] = '\n';
                    }
                }
            }
        c = BufFile[ PosFile++ ];
        if ( PosFile > TailleFic )
            break;
        }
}
