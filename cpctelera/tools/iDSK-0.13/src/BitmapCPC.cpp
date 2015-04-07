#include <iostream>
#include <cstdio>
#include "MyType.h"
#include "BitmapCPC.h"
#include "GestDsk.h"
#include <string>
#include <string.h>
using namespace std;


//
// Couleurs du CPC converties en composantes r, v, b
//
static StRVB RgbCPC[ 27 ] =
    {
        { 0x00, 0x00, 0x00, 0x00 },
        { 0x7F, 0x00, 0x00, 0x00 },
        { 0xFF, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x7F, 0x00 },
        { 0x7F, 0x00, 0x7F, 0x00 },
        { 0xFF, 0x00, 0x7F, 0x00 },
        { 0x00, 0x00, 0xFF, 0x00 },
        { 0x7F, 0x00, 0xFF, 0x00 },
        { 0xFF, 0x00, 0xFF, 0x00 },
        { 0x00, 0x7F, 0x00, 0x00 },
        { 0x7F, 0x7F, 0x00, 0x00 },
        { 0xFF, 0x7F, 0x00, 0x00 },
        { 0x00, 0x7F, 0x7F, 0x00 },
        { 0x7F, 0x7F, 0x7F, 0x00 },
        { 0xFF, 0x7F, 0x7F, 0x00 },
        { 0x00, 0x7F, 0xFF, 0x00 },
        { 0x7F, 0x7F, 0xFF, 0x00 },
        { 0xFF, 0x7F, 0xFF, 0x00 },
        { 0x00, 0xFF, 0x00, 0x00 },
        { 0x7F, 0xFF, 0x00, 0x00 },
        { 0xFF, 0xFF, 0x00, 0x00 },
        { 0x00, 0xFF, 0x7F, 0x00 },
        { 0x7F, 0xFF, 0x7F, 0x00 },
        { 0xFF, 0xFF, 0x7F, 0x00 },
        { 0x00, 0xFF, 0xFF, 0x00 },
        { 0x7F, 0xFF, 0xFF, 0x00 },
        { 0xFF, 0xFF, 0xFF, 0x00 }
    };


int GetRgbCPC( int Coul )
{
    if ( Coul >= 0 && Coul < 27 )
        {
        StRVB i = RgbCPC[ Coul ];
        return( ( i.b << 16 ) + ( i.v << 8 ) + i.r );
        }
    return( -1 );
}


StRVB GetPalCPC( int Coul )
{
    if ( Coul >= 0 && Coul < 27 )
        return( RgbCPC[ Coul ] );
    
    return( RgbCPC[ 0 ] );
}


void InitPalette( unsigned char NewPal[ 16 ], bool SetNewPal )
{
    /*
    Si sauvegard� avec ConvImgCpc, alors la palette se trouve
    dans l'image...
    */
  int i;
  if (  BitmapCPC[ 0x7D0 ] == 0x3A
	&& BitmapCPC[ 0x7D1 ] == 0xD0
	&& BitmapCPC[ 0x7D2 ] == 0xD7
	&& BitmapCPC[ 0x7D3 ] == 0xCD
	)
    {
      Mode = BitmapCPC[ 0x17D0 ];
      for ( i = 0; i < 16; i++ )
	Palette[ i ] = BitmapCPC[ 0x17D1 + i ];
    }
  if ( SetNewPal )
    for ( i = 0; i < 16; i++ )
      Palette[ i ] = NewPal[ i ];
}


//
// D�compacter une image au format OCP
//
void DepactOCP( void )
{
  static unsigned char BufTmp[ 0x4000 ];
  int PosIn = 0, PosOut = 0;
  int LgOut, CntBlock = 0;
  int c,i;
  unsigned char a;
  memcpy( BufTmp, BitmapCPC, sizeof( BufTmp ) );
  memset( BitmapCPC, 0, 0x4000 );
  while( PosOut < 0x4000 )
    {
      if ( ! strncmp( ( char * )&BufTmp[ PosIn ], "MJH", 3 ) )
	{
	  PosIn += 3;
	  LgOut = BufTmp[ PosIn++ ];
	  LgOut += ( BufTmp[ PosIn++ ] << 8 );
	  CntBlock = 0;
	  while( CntBlock < LgOut )
	    {
	      if ( ! strncmp( ( char * )&BufTmp[ PosIn ], "MJH", 3 ) )
		break;
	      
	      a = BufTmp[ PosIn++ ];
	      if ( a == MARKER_OCP )
                    {
		      c = BufTmp[ PosIn++ ];
		      a = BufTmp[ PosIn++ ];
		      if ( ! c )
                        c = 0x100;
		      
		      for ( i = 0; i < c && CntBlock < LgOut; i++ )
                        {
			  BitmapCPC[ PosOut++ ] = a;
			  CntBlock++;
                        }
                    }
	      else
		{
		  BitmapCPC[ PosOut++ ] = a;
		  CntBlock++;
		}
	    }
	}
      else
	PosOut = 0x4000;
    }
}


bool LireImage( char * Nom, StRVB * Bitmap )
{
    static unsigned char Entete[ 0x80 ];
    bool Ret = FALSE;
    //DWORD Nb;
    FILE* hFile;
    
    
    if ( (hFile=fopen(Nom,"r"))!=NULL )
        {
	  fread(Entete,sizeof(Entete),1,hFile);
	  //        ReadFile( hFile, Entete, sizeof( Entete ), &Nb, NULL );
        if ( CheckAmsdos( Entete ) )
            {
	      fread(BitmapCPC,sizeof( BitmapCPC ),1,hFile);
	      //            ReadFile( hFile, BitmapCPC, sizeof( BitmapCPC ), &Nb, NULL );
            if ( ! strncmp( ( char * )BitmapCPC, "MJH", 3 ) )
                DepactOCP();

            InitPalette( NULL, FALSE );
            Ret = TRUE;
            }
	// CloseHandle( hFile );
	fclose(hFile);
        if ( Ret )
            Render( Bitmap, 1 );
        }
    return( Ret );

}


//
// Affiche l'image � l'�cran
//
void Render( StRVB * Bitmap, bool Flat )
{
  int AdrCPC = 0, i, p0, p1, p2, p3;
  int y,x,AdrBitmap;
  unsigned char Octet;
    for ( y = 0; y < NbLignes; y++ )
        {
        AdrBitmap = TAILLE_CPC_X * ( y + ( ( 200 - NbLignes ) >> 1 ) )
                      + ( ( ( 80 - NbCol ) >> 1 ) << 3 );
        for ( x = 0; x < NbCol; x++ )
            {
            Octet = BitmapCPC[ AdrCPC + x ];
            switch( Mode )
                {
                case 0 :
                case 3 : // Mode 3 = Mode 0
                    p0 = ( Octet >> 7 )
                       + ( ( Octet & 0x20 ) >> 3 )
                       + ( ( Octet & 0x08 ) >> 2 )
                       + ( ( Octet & 0x02 ) << 2 );
                    p1 = ( ( Octet & 0x40 ) >> 6 )
                       + ( ( Octet & 0x10 ) >> 2 )
                       + ( ( Octet & 0x04 ) >> 1 )
                       + ( ( Octet & 0x01 ) << 3 );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p0 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p0 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p0 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p0 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p1 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p1 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p1 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p1 ] );
                    break;

                case 1 :
                    p0 = ( ( Octet >> 7 ) & 1 ) + ( ( Octet >> 2 ) & 2 );
                    p1 = ( ( Octet >> 6 ) & 1 ) + ( ( Octet >> 1 ) & 2 );
                    p2 = ( ( Octet >> 5 ) & 1 ) + ( ( Octet >> 0 ) & 2 );
                    p3 = ( ( Octet >> 4 ) & 1 ) + ( ( Octet << 1 ) & 2 );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p0 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p0 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p1 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p1 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p2 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p2 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p3 ] );
                    Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ p3 ] );
                    break;

                case 2 :
                    for ( i = 8; i--; )
                        Bitmap[ AdrBitmap++ ] = GetPalCPC( Palette[ ( Octet >> i ) & 1 ] );
                    break;
                }
            }
        if ( Flat )
            AdrCPC += NbCol;
        else
            {
            AdrCPC += 0x800;
            if ( AdrCPC > 0x3FFF )
                AdrCPC -= 0x3FB0;
            }
        }
}


void SetBitmapCPC( unsigned char * BitmapSource )
{
    memcpy( BitmapCPC, BitmapSource, 0x4000 );
    if ( ! strncmp( ( char * )BitmapCPC, "MJH", 3 ) )
        DepactOCP();

    InitPalette( NULL, FALSE );
}


void SetNbCol( int n )
{
    if ( n > 0 && n <= 80 )
        NbCol = n;
}


void SetNbLignes( int n )
{
    if ( n > 0 && n <= 200 )
        NbLignes = n;
}
