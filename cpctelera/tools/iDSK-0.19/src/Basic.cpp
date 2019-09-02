#include <iostream>
using namespace std;
#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "MyType.h"
#include "Basic.h"


//static char ConvCpcFr[ 128 ] = " !\"#$%&'()*+,-./0123456789:;<=>?àABCDEFGHIJKLMNOPQRSTUVWXYZ[ç]^_`abcdefghijklmnopqrstuvwxyzéùè~";


//
// Tableau de décryptage d'un programme en basic protégé
//
static BYTE DproBasic[ 128 ] =
    {
    0xAB, 0x2C, 0xED, 0xEA, 0x6C, 0x37, 0x3F, 0xEC,
    0x9B, 0xDF, 0x7A, 0x0C, 0x3B, 0xD4, 0x6D, 0xF5,
    0x04, 0x44, 0x03, 0x11, 0xDF, 0x59, 0x8F, 0x21,
    0x73, 0x7A, 0xCC, 0x83, 0xDD, 0x30, 0x6A, 0x30,
    0xD3, 0x8F, 0x02, 0xF0, 0x60, 0x6B, 0x94, 0xE4,
    0xB7, 0xF3, 0x03, 0xA8, 0x60, 0x88, 0xF0, 0x43,
    0xE8, 0x8E, 0x43, 0xA0, 0xCA, 0x84, 0x31, 0x53,
    0xF3, 0x1F, 0xC9, 0xE8, 0xAD, 0xC0, 0xBA, 0x6D,
    0x93, 0x08, 0xD4, 0x6A, 0x2C, 0xB2, 0x07, 0x27,
    0xC0, 0x99, 0xEE, 0x89, 0xAF, 0xC3, 0x53, 0xAB,
    0x2B, 0x34, 0x5C, 0x2F, 0x13, 0xEE, 0xAA, 0x2C,
    0xD9, 0xF4, 0xBC, 0x12, 0xB3, 0xC5, 0x1C, 0x68,
    0x01, 0x20, 0x2C, 0xFA, 0x77, 0xA6, 0xB5, 0xA4,
    0xFC, 0x9B, 0xF1, 0x32, 0x5B, 0xC3, 0x70, 0x77,
    0x85, 0x36, 0xBE, 0x5B, 0x8C, 0xC8, 0xB5, 0xC2,
    0xF0, 0x0B, 0x98, 0x0F, 0x36, 0x9D, 0xD8, 0x96
    };


BYTE GetByte( BYTE * BufFile, int Pos, int Deprotect )
{
	//BYTE b = ( BYTE )( BufFile[ Pos ] ^ ( DproBasic[ Pos & 0x7F ] * Deprotect ) );
	//cout << "GetByte:"<<hex<<b<<endl;
    return( BYTE )( BufFile[ Pos ] ^ ( DproBasic[ Pos & 0x7F ] * Deprotect ) );
}


int GetWord( BYTE * BufFile, int Pos, int Deprotect )
{
    int Ret = BufFile[ Pos ] ^ ( DproBasic[ Pos & 0x7F ] * Deprotect );
    Pos++;
    Ret += ( ( BufFile[ Pos ] ^ ( DproBasic[ Pos & 0x7F ] * Deprotect ) ) << 8 );
    return( Ret );
}


//
// Ajoute un "mot" (nom d'une variable, RSX...) dans la chaine "Listing"
//
int AddWord( BYTE * BufFile, int Pos, char * Listing, int Deprotect )
{
    int LenVar = 0, l = strlen( Listing );
    BYTE b;

    do
        {
        b = GetByte( BufFile, Pos++, Deprotect );
        Listing[ l++ ] = ( char )( b & 0x7F );
        }
    while( ! ( b & 0x80 ) && LenVar++ < 0xFF );
    Listing[ l ] = 0;
    return( Pos );
}


//
// Convertir le buffer en listing basic
//
void Basic( BYTE * BufFile, char * Listing, bool IsBasic, bool CrLf )
{
    static char Tmp[ 32 ];
    int Pos = 0, Token = 0;
    int StartLigne = 0, EndLigne;
    char * p;
    double f;
    int exp;
	int Deprotect=0;
	//cout << BufFile <<endl;
    static const char * Nbre[ 11 ] =
        {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"
        };
    static const char * MotsClefs[ 0x80 ] =
        {
        "AFTER", "AUTO", "BORDER", "CALL", "CAT", "CHAIN", "CLEAR", "CLG",
        "CLOSEIN", "CLOSEOUT", "CLS", "CONT", "DATA", "DEF", "DEFINT",
        "DEFREAL", "DEFSTR", "DEG", "DELETE", "DIM", "DRAW", "DRAWR", "EDIT",
        "ELSE", "END", "ENT", "ENV", "ERASE", "ERROR", "EVERY", "FOR",
        "GOSUB", "GOTO", "IF", "INK", "INPUT", "KEY", "LET", "LINE", "LIST",
        "LOAD", "LOCATE", "MEMORY", "MERGE", "MID$", "MODE", "MOVE", "MOVER",
        "NEXT", "NEW", "ON", "ON BREAK", "ON ERROR GOTO", "SQ", "OPENIN",
        "OPENOUT", "ORIGIN", "OUT", "PAPER", "PEN", "PLOT", "PLOTR", "POKE",
        "PRINT", "'", "RAD", "RANDOMIZE", "READ", "RELEASE", "REM", "RENUM",
        "RESTORE", "RESUME", "RETURN", "RUN", "SAVE", "SOUND", "SPEED", "STOP",
        "SYMBOL", "TAG", "TAGOFF", "TROFF", "TRON", "WAIT", "WEND", "WHILE",
        "WIDTH", "WINDOW", "WRITE", "ZONE", "DI", "EI", "FILL", "GRAPHICS",
        "MASK", "FRAME", "CURSOR", "#E2", "ERL", "FN", "SPC", "STEP", "SWAP",
        "#E8", "#E9", "TAB", "THEN", "TO", "USING", ">", "=", ">=", "<", "<>",
        "<=", "+", "-", "*", "/", "^", "\\ ", "AND", "MOD", "OR", "XOR", "NOT",
        "#FF"
        };

    static const char * Fcts[ 0x80 ] =
        {
        "ABS", "ASC", "ATN", "CHR$", "CINT", "COS", "CREAL", "EXP", "FIX",
        "FRE", "INKEY", "INP", "INT", "JOY", "LEN", "LOG", "LOG10", "LOWER$",
        "PEEK", "REMAIN", "SGN", "SIN", "SPACE$", "SQ", "SQR", "STR$", "TAN",
        "UNT", "UPPER$", "VAL", "", "", "", "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "EOF", "ERR", "HIMEM", "INKEY$", "PI", "RND",
        "TIME", "XPOS", "YPOS", "DERR", "", "", "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
        "", "", "", "", "", "", "", "", "", "", "", "BIN$", "DEC$", "HEX$",
        "INSTR", "LEFT$", "MAX", "MIN", "POS", "RIGHT$", "ROUND", "STRING$",
        "TEST", "TESTR", "COPYCHR$", "VPOS"
        };


    * Listing = 0;
    Token = GetByte( BufFile, 0, Deprotect );
    for ( ;; )
        {
        	//cout << "Listing : " <<Listing << endl;
        if ( IsBasic )
            {
            int lg = GetWord( BufFile, Pos, Deprotect );
            Pos += 2;
            if ( ! lg )
                break;

            int NumLigne = GetWord( BufFile, Pos, Deprotect );
            Pos += 2;
            sprintf( Tmp, "%d ", NumLigne );
            strcat( Listing, Tmp );
            }
        else
            if ( ! Token || Token == 0x1A )
                break;

        int DansChaine = 0; // #### Hum, plus compliqué que ça je pense...
        do
            {
            //cout << "Tmp:"<<Tmp<<endl;
            Token = GetByte( BufFile, Pos++, Deprotect );
            if ( ! IsBasic && Token == 0x1A )
                break;

            if ( DansChaine || ! IsBasic )
                {
                Tmp[ 0 ] = ( char )Token;
                Tmp[ 1 ] = 0;
                strcat( Listing, Tmp );
                if ( Token == '"' )
                    DansChaine ^= 1;
                   // cout << " DansChaine Tmp:"<<Tmp<<endl;
                }
            else
                if ( Token > 0x7F && Token < 0xFF )
                    {
                    // #### Traitement particulier du ':' avant le ELSE
                    if (  Listing[ strlen( Listing ) - 1 ] == ':' 
                       && Token == 0x97
                       )
                        Listing[ strlen( Listing ) - 1 ] = 0;

                    strcat( Listing
                          , MotsClefs[ Token & 0x7F ]
                          );
                    }
                else
                    if ( Token >= 0x0E && Token <= 0x18 )
                        strcat( Listing
                              , Nbre[ Token - 0x0E ]
                              );
                    else
                        if ( Token >= 0x20 && Token < 0x7C )
                            {
                            Tmp[ 0 ] = ( char )Token;
                            Tmp[ 1 ] = 0;
                            strcat( Listing, Tmp );
                            if ( Token == '"' )
                                DansChaine ^= 1;
                            }
                        else
                            {
                            //cout << "Token:" << Token <<endl;
                            switch( Token )
                                {
                                case 0x01 :
                                    Tmp[ 0 ] = ':';
                                    Tmp[ 1 ] = 0;
                                    strcat( Listing, Tmp );
                                    break;

                                case 0x02 : // Variable entière (type %)
                                    Pos = AddWord( BufFile
                                                 , 2 + Pos
                                                 , Listing
                                                 , Deprotect
                                                 );
                                    strcat( Listing, "%" );
                                    break;


                                case 0x03 : // Variable chaine (type $)
                                    Pos = AddWord( BufFile
                                                 , 2 + Pos
                                                 , Listing
                                                 , Deprotect
                                                 );
                                    strcat( Listing, "$" );
                                    break;

                                case 0x04 : // Variable float (type !)
                                    Pos = AddWord( BufFile
                                                 , 2 + Pos
                                                 , Listing
                                                 , Deprotect
                                                 );
                                    strcat( Listing, "!" );
                                    break;

                                case 0x0B :
                                case 0x0C :
                                case 0x0D : // Variable "standard"
                                    Pos = AddWord( BufFile
                                                 , 2 + Pos
                                                 , Listing
                                                 , Deprotect
                                                 );
                                    break;

                                case 0x19 : // Constante entière 8 bits
									sprintf(Listing+strlen(Listing),"%d",(BYTE)GetByte( BufFile, Pos, Deprotect)); 
                                    Pos++;
                                    break;

                                case 0x1A :
                                case 0x1E : // Constante entière 16 bits
									sprintf(Listing+strlen(Listing),"%d",GetWord( BufFile, Pos, Deprotect));
                                    Pos += 2;
                                    break;

                                case 0x1B :
                                    sprintf( Tmp
                                           , "&X%X"
                                           , GetWord( BufFile, Pos, Deprotect )
                                           );
                                    strcat( Listing, Tmp );
                                    Pos += 2;
                                    break;

                                case 0x1C :
                                    sprintf( Tmp
                                           , "&%X"
                                           , GetWord( BufFile, Pos, Deprotect )
                                           );
                                    strcat( Listing, Tmp );
                                    Pos += 2;
                                    break;

                                case 0x1F : // Constante flottante
                                    f = ( GetByte( BufFile, Pos + 2, Deprotect ) << 16 )
                                      + ( GetByte( BufFile, Pos + 1, Deprotect ) << 8 )
                                      + GetByte( BufFile, Pos, Deprotect )
                                      + ( ( GetByte( BufFile, Pos + 3, Deprotect ) & 0x7F ) << 24 );
                                    f = 1 + ( f / 0x80000000 );

                                    if ( GetByte( BufFile, Pos + 3, Deprotect ) & 0x80 )
                                        f = -f;

                                    exp = GetByte( BufFile, Pos + 4, Deprotect ) - 129;
                                    Pos += 5;
                                    sprintf( Tmp, "%f", f * pow( (double) 2, exp ) );
                                    // Suppression des '0' inutiles
                                    p = &Tmp[ strlen( Tmp ) - 1 ];
                                    while( * p == '0' )
                                        * p-- = 0;

                                    if ( * p == '.' )
                                        * p = 0;

                                    strcat( Listing, Tmp );
                                    break;

                                case 0x7C :
                                    strcat( Listing, "|" );
                                    Pos = AddWord( BufFile
                                                 , 1 + Pos
                                                 , Listing
                                                 , Deprotect
                                                 );
                                    break;

                                case 0xFF :
                                    if ( GetByte( BufFile, Pos, Deprotect ) < 0x80 )
                                        strcat( Listing
                                              , Fcts[ GetByte( BufFile
                                                             , Pos++
                                                             , Deprotect
                                                             )
                                                    ]
                                              );
                                    else
                                        {
                                        Tmp[ 1 ] = 0;
                                        Tmp[ 0 ] = ( char )( GetByte( BufFile
                                                                    , Pos++
                                                                    , Deprotect
                                                                    ) & 0x7F
                                                           );
                                        strcat( Listing, Tmp );
                                        }
                                    break;

                                default :
                                    Token = Token;
                                }
                            }
            }
        while( Token );
        if ( CrLf )
            {
            //
            // Retour à la ligne si > 80 caractères
            //
            EndLigne = strlen( &Listing[ StartLigne ] );
            while( EndLigne > 80 )
                {
                memmove( &Listing[ StartLigne + 82 ]
                       , &Listing[ StartLigne + 80 ]
                       , EndLigne
                       );
                memcpy( &Listing[ StartLigne + 80 ], "\r\n", 2 );
                StartLigne += 82;
                EndLigne -= 80;
                }
            }
        strcat( Listing, "\r\n" );
        StartLigne = strlen( Listing );
        }
    // Conversion des caractères accentués si nécessaire

      for ( int i = strlen( Listing); i--; )
        {
        	//cout << i << " ";
        
            if ( ! isprint(Listing[ i ]) &&  Listing[ i ] != '\n' && Listing[ i ] != '\r'  ) Listing[ i ] = '?';  
        }
}
