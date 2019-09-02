#include <iostream>
#include <cstring>
#include <cstdio>
using namespace std;
#include "Outils.h"

//
// Initialise une chaine au format hexad�cimal en fonction de la valeur d'entr�e
//
void Hex( char Chaine[], int Valeur, int Digit )
{
    static char TabDigit[ 17 ] = "0123456789ABCDEF";

    while( Digit )
        * Chaine++ = TabDigit[ ( Valeur >> ( 4 * ( --Digit ) ) ) & 0x0F ];
}


//
// Conversion hexa->d�cimal
//
int HexToDec( char * Valeur )
{
    char * p = strchr( Valeur, 'X' );
    if ( p )
        Valeur = ++p;

    p = strchr( Valeur, 'x' );
    if ( p )
        Valeur = ++p;

    p = strchr( Valeur, '#' );
    if ( p )
        Valeur = ++p;

    p = strchr( Valeur, '$' );
    if ( p )
        Valeur = ++p;

    p = strchr( Valeur, '&' );
    if ( p )
        Valeur = ++p;

    int Ret = 0, i = 0;
    while( Valeur[ i ] )
        {
        Ret <<= 4;
        char c = Valeur[ i++ ];
        if ( c >= '0' && c <= '9' )
            Ret += c - '0';
        else
            Ret += ( c & 0x5F ) - 0x37;
        }
    return( Ret );
}


//
// Conversion d'un secteur (512 octets) en affichage Hexa et ASCII
//
void SetBuffViewHexa( unsigned char * src, char * Hex, char * Ascii, unsigned short Offset, int AddOffset)
{
    const char * CodeHexa = "0123456789ABCDEF";
    int q = 0,i;

    //
    // Parcourir les 512 octets de la source et remplir les buffers
    //
    for ( i = 0; i < 512; i++ )
        {
        unsigned char b = * src++;
        if ( b > 32 && b < 127 )
        {
            Ascii[ i ] = b;
           // cout << "b32:" << (int)b <<" Ascii["<<i<<"]:"<< Ascii[ i ] << endl;
        }
        else
        {
            Ascii[ i ] = '.';
        }
        if ( AddOffset && ( ! ( i & 0x0F ) ) )
            {
            Hex[ q++ ] = '#';
            Hex[ q++ ] = CodeHexa[ Offset >> 12 ];
            Hex[ q++ ] = CodeHexa[ ( Offset >> 8 ) & 0x0F ];
            Hex[ q++ ] = CodeHexa[ ( Offset >> 4 ) & 0x0F ];
            Hex[ q++ ] = CodeHexa[ Offset & 0x0F ];
            Hex[ q++ ] = ':';
            }
        Offset++;
        Hex[ q++ ] = CodeHexa[ b >> 4 ];
        Hex[ q++ ] = CodeHexa[ b & 0x0F ];
        Hex[ q++ ] = ' ';
        }
    Hex[ q ] = 0;
    Ascii[ i ] = 0;
}


//
// Retourne le num�ro d'user sous forme de chaine
//
char * GetUser( int u )
{
    static char User[ 8 ];
	sprintf(User, "%d", u);
    return( User);
}



//
// Retourne la taille du fichier sous forme de chaine
//
char * GetTaille( int t )
{
    static char Taille[ 16 ];

    sprintf( Taille, "%d Ko", t );
    return( Taille );
}


//
// Retourne le nom du fichier formatt� amsdos (8+3)
//
char * GetNomAmsdos(const char * AmsName )
{
	// Extract the name (without directory components)
	const char* lastSlash = strrchr(AmsName, '/');
	const char* lastBackslash = strrchr(AmsName, '\\');
	if (lastSlash > lastBackslash)
		AmsName = lastSlash + 1;
	else if (lastSlash < lastBackslash)
		AmsName = lastBackslash + 1;

    static char NomAmsdos[ 16 ];
		int i;

    char * p = NomAmsdos;
    for ( i = 0; i < 8; i++ ) {
        if ( * AmsName != ' ' && *AmsName != '.' )
            * p++ = * AmsName++;
		/*if ( * AmsName == '-' )
			* p++ = * AmsName++;*/
	}

    while( * AmsName != '.' && * AmsName )
        AmsName++;

	AmsName++;
	
    * p = 0;
    strcat( NomAmsdos, "." );

    for ( i = 0; * AmsName && i < 3; i++ )
        *++p = * AmsName++;

    * ++p = 0;
    i = 0;
    while( NomAmsdos[ i ] )
        NomAmsdos[ i++ ] &= 0x7F;

    return( NomAmsdos );
}
