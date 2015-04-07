#ifndef BITMAPCPC_H
#define BITMAPCPC_H


#define     TAILLE_CPC_X    640

#define     TAILLE_CPC_Y    200

#define     MARKER_OCP      1       // Marker pour compression RLE
int Mode, NbCol, NbLignes;
unsigned char BitmapCPC[ 0x4000 ];
unsigned char Palette[ 16 ];
typedef struct
    {
    unsigned char b, v, r, a;
    } StRVB;



void CBitmapCPC( void ) { NbCol = 80; NbLignes = 200; }
bool LireImage( char * Nom, StRVB * Bitmap );
void Render( StRVB * Bitmap, bool Flat );
void SetBitmapCPC( unsigned char * BitmapSource );
unsigned char * GetBitmapCPC( void ) { return( BitmapCPC ); }
void SetMode( int m ) { Mode = m; }
void InitPalette( unsigned char Pal[ 16 ], bool SetPal );
unsigned char * GetPalette( void ) { return( Palette ); }
int GetMode( void ) { return( Mode ); }
void SetNbCol( int n );
void SetNbLignes( int n );


void DepactOCP( void );
void LisseBitmap( StRVB * Bitmap );






StRVB GetPalCPC( int Coul );

int GetRgbCPC( int Coul );


#endif
