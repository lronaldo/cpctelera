#ifndef __OUTILS_H__
#define __OUTILS_H__


void Hex( char Chaine[], int Valeur, int Digit );

int HexToDec( char * Valeur );

void SetBuffViewHexa( unsigned char * src, char * Hex, char * Ascii,unsigned short Offset, int AddOffset);

char * GetNomAmsdos( char * AmsName );

char * GetUser( int u );

char * GetTaille( int t );



#endif
