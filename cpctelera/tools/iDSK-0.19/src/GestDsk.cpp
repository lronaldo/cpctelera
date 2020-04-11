#include <iostream>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <sstream>

#include "MyType.h"
#include "GestDsk.h"
#include "endianPPC.h"
#include "Outils.h"
#include <cerrno>

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif

using namespace std;

char Listing[ 0x280000 ];
unsigned char BufFile[ 0x10000 ];
int TailleFic, CurLigne;


//
// Verifie si en-tete AMSDOS est valide
//
bool CheckAmsdos( unsigned char * Buf ) {
    int i, Checksum = 0;
    bool ModeAmsdos = false;
	unsigned short CheckSumFile;
	CheckSumFile = Buf[ 0x43 ] + Buf[ 0x43 +1 ] *256;
    for ( i = 0; i < 67; i++ )
		Checksum += Buf[ i ];
	
    if ( ( CheckSumFile == ( unsigned short )Checksum ) && Checksum )
        ModeAmsdos = true;
	
    return( ModeAmsdos );
}



//
// Cr�e une en-t�te AMSDOS par d�faut
//
StAmsdos * CreeEnteteAmsdos( char * NomFic, unsigned short Longueur ) {
    static char NomReel[ 256 ];
    static StAmsdos Entete;
    static char Nom[ 12 ];
    int i;
	
    strcpy( NomReel, NomFic );
    memset( &Entete, 0, sizeof( Entete ) );
    memset( Nom, ' ', sizeof( Nom ) );
    char * p = NULL;
    do {
	p = strchr( NomReel, '/' ); //Sous linux c'est le / qu'il faut enlever ...
        if ( p )
            strcpy( NomReel, ++p );
    } while( p );
    p = strchr( NomReel, '.' );
    if ( p )
        * p++ = 0;
	
    int l = strlen( NomReel );
    if ( l > 8 )
        l = 8;
	
    for ( int i = 0; i < l; i++ )
        Nom[ i ] = ( char )toupper( NomReel[ i ] );
	
    if ( p )
        for ( i = 0; i < 3; i++ )
            Nom[ i + 8 ] = ( char )toupper( p[ i ] );
	
    memcpy( Entete.FileName, Nom, 11 );
    Entete.Length = 0; 	//Non renseign� par AMSDos !!
    Entete.RealLength = Entete.LogicalLength = Longueur;
    Entete.FileType = 2; //Fichier binaire

    SetChecksum(&Entete);

    return( &Entete );
}


//
// Calcule et positionne le checksum AMSDOS
//
void SetChecksum( StAmsdos * pEntete ) {
    int i, Checksum = 0;
    unsigned char * p = ( unsigned char * )pEntete;
    for ( i = 0; i < 67; i++ )
        Checksum += * (p+i);
	
    pEntete->CheckSum = ( unsigned short )Checksum;
}


//
// Effectue un "nettoyage" de l'en-tete Amsdos :
// remet a zero les octets inutilises
//
void ClearAmsdos( unsigned char * Buf ) {
    if ( CheckAmsdos( Buf ) ) {
        int i, Checksum = 0;
        StAmsdos * pEntete = ( StAmsdos * )Buf;
        memset( pEntete->Unused, 0, sizeof( pEntete->Unused ) );
        memset( pEntete->Unused2, 0, sizeof( pEntete->Unused2 ) );
        for ( i = 0; i < 67; i++ )
            Checksum += Buf[ i ];
		
		Buf[ 0x43 ] = ( unsigned short )Checksum;
	}
}

//
// Recherche le plus petit secteur d'une piste
//
int DSK::GetMinSect( void ) {
    int Sect = 0x100;
    CPCEMUTrack * tr = ( CPCEMUTrack * )&ImgDsk[ sizeof( CPCEMUEnt ) ];
    for ( int s = 0; s < tr->NbSect; s++ )
        if ( Sect > tr->Sect[ s ].R )
            Sect = tr->Sect[ s ].R;
	
    return( Sect );
}


//
// Retourne la position d'un secteur dans le fichier DSK
//
int DSK::GetPosData( int track, int sect, bool SectPhysique ) {
    // Recherche position secteur
    int Pos = sizeof( CPCEMUEnt );
    CPCEMUTrack * tr = ( CPCEMUTrack * )&ImgDsk[ Pos ];
    short SizeByte;
    for ( int t = 0; t <= track; t++ ) {
        Pos += sizeof( CPCEMUTrack );
        for ( int s = 0; s < tr->NbSect; s++ ) {
            if ( t == track ) {
                if (  ( ( tr->Sect[ s ].R == sect ) && SectPhysique )
					  || ( ( s == sect ) && ! SectPhysique )
					  )
                    break;
			}
			SizeByte = tr->Sect[ s ].SizeByte ;
			if (SizeByte)
				Pos += SizeByte;
			else
				Pos += ( 128 << tr->Sect[ s ].N );
        }
	}
    return( Pos );
}


//
// Recherche un bloc libre et le remplit
//
int DSK::RechercheBlocLibre( int MaxBloc ) {
    for ( int i = 2; i < MaxBloc; i++ )
        if ( ! Bitmap[ i ] ) {
            Bitmap[ i ] = 1;
            return( i );
	}
    return( 0 );
}


//
// Recherche une entr�e de r�pertoire libre
//
int DSK::RechercheDirLibre( void ) {
    for ( int i = 0; i < 64; i++ ) {
        StDirEntry * Dir = GetInfoDirEntry( i );
        if ( Dir->User == USER_DELETED )
            return( i );
	}
    return( -1 );
}


//
// Retourne les donn�es "brutes" de l'image disquette
//
unsigned char * DSK::GetRawData( int Pos ) {
    return( &ImgDsk[ Pos ] );
}


//
// Ecriture de donn�es "brutes" dans l'image disquette
//
void DSK::WriteRawData( int Pos, unsigned char * Data, int Longueur ) {
    memcpy( &ImgDsk[ Pos ], Data, Longueur );
}


//
// Retourne la taille du fichier image
//
int DSK::GetTailleDsk( void ) {
    CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
    int NbTracks = Infos->NbTracks;
    int Pos = sizeof( CPCEMUEnt );
    CPCEMUTrack * tr = ( CPCEMUTrack * )&ImgDsk[ Pos ];
    for ( int t = 0; t < NbTracks; t++ ) {
        Pos += sizeof( CPCEMUTrack );
        for ( int s = 0; s < tr->NbSect; s++ ) {
            if ( tr->Sect[ s ].SizeByte )
                Pos += tr->Sect[ s ].SizeByte;
            else
                Pos += ( 128 << tr->Sect[ s ].N );
		}
	}
    return( Pos );
}


//
// Retourne le nombre de pistes de la disquette
//
int DSK::GetNbTracks( void ) {
    CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
    return( Infos->NbTracks );
}


//
// Lecture d'un bloc AMSDOS (1 block = 2 secteurs)
//
unsigned char * DSK::ReadBloc( int bloc ) {
    static unsigned char BufBloc[ SECTSIZE * 2 ];
    int track = ( bloc << 1 ) / 9;
    int sect = ( bloc << 1 ) % 9;
    int MinSect = GetMinSect();
    if ( MinSect == 0x41 )
        track += 2;
    else
        if ( MinSect == 0x01 )
            track++;
	
    int Pos = GetPosData( track, sect + MinSect, true );
    memcpy( BufBloc, &ImgDsk[ Pos ], SECTSIZE );
    if ( ++sect > 8 ) {
        track++;
        sect = 0;
	}
	
    Pos = GetPosData( track, sect + MinSect, true );
    memcpy( &BufBloc[ SECTSIZE ], &ImgDsk[ Pos ], SECTSIZE );
    return( BufBloc );
}


//
// Formatter une piste
//
void DSK::FormatTrack( CPCEMUEnt * Infos, int t, int MinSect, int NbSect ) {
    CPCEMUTrack * tr = ( CPCEMUTrack * )&ImgDsk[ sizeof( CPCEMUEnt ) + t * Infos->DataSize ];
    memset( &ImgDsk[ sizeof( CPCEMUEnt )
		+ sizeof( CPCEMUTrack )
		+ ( t * Infos->DataSize )
		]
			, 0xE5
			, 0x200 * NbSect
			);
    strcpy( tr->ID, "Track-Info\r\n" );
    tr->Track = ( unsigned char )t;
    tr->Head = 0;
    tr->SectSize = 2;
    tr->NbSect = ( unsigned char )NbSect;
    tr->Gap3 = 0x4E;
    tr->OctRemp = 0xE5;
    int ss = 0;
    //
    // Gestion "entrelacement" des secteurs
    //
    for ( int s = 0; s < NbSect; ) {
        tr->Sect[ s ].C = ( unsigned char )t;
        tr->Sect[ s ].H = 0;
        tr->Sect[ s ].R = ( unsigned char )( ss + MinSect );
        tr->Sect[ s ].N = 2;
        tr->Sect[ s ].SizeByte = 0x200;
        ss++;
        if ( ++s < NbSect ) {
            tr->Sect[ s ].C = ( unsigned char )t;
            tr->Sect[ s ].H = 0;
            tr->Sect[ s ].R = ( unsigned char )( ss + MinSect + 4 );
            tr->Sect[ s ].N = 2;
            tr->Sect[ s ].SizeByte = 0x200;
            s++;
		}
	}
}


//
// Ecriture d'un bloc AMSDOS (1 block = 2 secteurs)
//
void DSK::WriteBloc( int bloc, unsigned char BufBloc[ SECTSIZE * 2 ] ) {
    int track = ( bloc << 1 ) / 9;
    int sect = ( bloc << 1 ) % 9;
    int MinSect = GetMinSect();
    if ( MinSect == 0x41 )
        track += 2;
    else
        if ( MinSect == 0x01 )
            track++;
	
    //
    // Ajuste le nombre de pistes si d�passement capacit�
    //
    CPCEMUEnt * Entete = ( CPCEMUEnt * )ImgDsk;
    if ( track > Entete->NbTracks - 1 ) {
        Entete->NbTracks = ( unsigned char )( track + 1 );
        FormatTrack( Entete, track, MinSect, 9 );
	}
	
    int Pos = GetPosData( track, sect + MinSect, true );
    memcpy( &ImgDsk[ Pos ], BufBloc, SECTSIZE );
    if ( ++sect > 8 ) {
        track++;
        sect = 0;
	}
    Pos = GetPosData( track, sect + MinSect, true );
    memcpy( &ImgDsk[ Pos ], &BufBloc[ SECTSIZE ], SECTSIZE );
}


//
// Ecriture d'un secteur
//
void DSK::WriteSect( int Track, int Sect, unsigned char * Buff, int AmsdosMode ) {
    int MinSect = AmsdosMode ? GetMinSect() : 0;
    if ( ( MinSect == 0x41 ) && AmsdosMode )
        Track += 2;
    else
        if ( ( MinSect == 0x01 ) && AmsdosMode )
            Track++;
	
    int Pos = GetPosData( Track, Sect + MinSect, AmsdosMode );
    memcpy( &ImgDsk[ Pos ], Buff, SECTSIZE );
}


//
// Lecture d'un secteur
//
unsigned char * DSK::ReadSect( int Track, int Sect, int AmsdosMode ) {
    int MinSect = AmsdosMode ? GetMinSect() : 0;
    if ( ( MinSect == 0x41 ) && AmsdosMode )
        Track += 2;
    else
        if ( ( MinSect == 0x01 ) && AmsdosMode )
            Track++;
	
    int Pos = GetPosData( Track, Sect + MinSect, AmsdosMode );
    return( &ImgDsk[ Pos ] );
}


//
// Retourne les informations d'une piste
//
CPCEMUTrack * DSK::GetInfoTrack( int Track ) {
    int Pos = sizeof( CPCEMUEnt );
    CPCEMUTrack * tr = ( CPCEMUTrack * )&ImgDsk[ Pos ];
    for ( int t = 0; t < Track; t++ ) {
		Pos += sizeof( CPCEMUTrack );
		
	    for ( int s = 0; s < tr->NbSect; s++ ) {
            if ( tr->Sect[ s ].SizeByte )
                Pos += tr->Sect[ s ].SizeByte;
            else
                Pos += ( 128 << tr->Sect[ s ].N );
		}
	}
    return( ( CPCEMUTrack * )&ImgDsk[ Pos ] );
}

//
// Remplit un "bitmap" pour savoir o� il y a des fichiers sur la disquette
// Retourne �galement le nombre de Ko utilis�s sur la disquette
//
int DSK::FillBitmap( void ) {
    int NbKo = 0;
	
    memset( Bitmap, 0, sizeof( Bitmap ) );
    Bitmap[ 0 ] = Bitmap[ 1 ] = 1;
    for ( int i = 0; i < 64; i++ ) {
        StDirEntry * Dir = GetInfoDirEntry( i );
        if ( Dir->User != USER_DELETED ) {
            for ( int j = 0; j < 16; j++ ) {
                int b = Dir->Blocks[ j ];
                if ( b > 1 && ( ! Bitmap[ b ] ) ) {
                    Bitmap[ b ] = 1;
                    NbKo++;
				}
			}
		}
	}
    return( NbKo );
}


//
// Positionne une entr�e dans le r�pertoire
//
void DSK::SetInfoDirEntry( int NumDir, StDirEntry * Dir ) {
    int MinSect = GetMinSect();
    int s = ( NumDir >> 4 ) + MinSect;
    int t = ( MinSect == 0x41 ? 2 : 0 );
    if ( MinSect == 1 )
        t = 1;
	
    for (int i =0; i<16; i++) 
		memcpy( &ImgDsk[ ( ( NumDir & 15 ) << 5 ) + GetPosData( t, s, true ) ]
				, Dir
				, sizeof( StDirEntry )
				);
}


//
// V�rifie l'existente d'un fichier, retourne l'indice du fichier si existe,
// -1 sinon
//
int DSK::FileExist( char * Nom ) {
	int i;
	for ( i = 0; i < 64; i++ ) {
		StDirEntry * Dir = GetInfoDirEntry( i ); 
		for(int q=0;q<12;q++)
		    Dir->Nom[q]=Dir->Nom[q]&127; // Avoid missing hidden files
		if (  Dir->User != USER_DELETED 
			  && ! strncmp( Nom, ( char * )Dir->Nom, 11 ) // 11 = 8+3 car le point est enlev�
			  )
			return( i );
	}
	return( -1 );
}


StDirEntry * DSK::GetNomDir( string NomFic ) {
    static StDirEntry DirLoc;
    int i;
	
    memset( &DirLoc, 0, sizeof( DirLoc ) );
    memset( DirLoc.Nom, ' ', 8 );
    memset( DirLoc.Ext, ' ', 3 );
    size_t p = NomFic.find('.');
    if ( p!=std::string::npos )
    {
        NomFic.copy( DirLoc.Nom, std::min((int)p,8), 0);
	p++;
        NomFic.copy( DirLoc.Ext, std::min( (int)(NomFic.size()-p), 3 ), p );
    }
    else
        NomFic.copy( DirLoc.Nom, std::min((int)NomFic.size(), 8 ),0);
	
    for ( i = 0; i < 11; i++ )
        DirLoc.Nom[ i ] = ( unsigned char )toupper( DirLoc.Nom[ i ] );

    return( &DirLoc );
}


int DSK::FileIsIn( string FileName ) {
	StDirEntry * DirLoc = GetNomDir( FileName );
	return FileExist( ( char*) DirLoc->Nom );
}

//
// Copie un fichier sur le DSK
//
// la taille est determine par le nombre de NbPages
// regarder pourquoi different d'une autre DSK
int DSK::CopieFichier( unsigned char * BufFile, char * NomFic, int TailleFic, int MaxBloc, int UserNumber, bool System_file, bool Read_only ) {
    int j, l, Bloc, PosFile, NbPages = 0, PosDir, TaillePage;
    FillBitmap();
    StDirEntry * DirLoc = GetNomDir( NomFic ); 	//Construit l'entr�e pour mettre dans le catalogue
    for ( PosFile = 0; PosFile < TailleFic; ) { //Pour chaque bloc du fichier
        PosDir = RechercheDirLibre(); 		//Trouve une entr�e libre dans le CAT
        if ( PosDir != -1 ) {
            DirLoc->User = UserNumber;			//Remplit l'entr�e : User 0
            // http://www.cpm8680.com/cpmtools/cpm.htm
            if(System_file) DirLoc->Ext[1]|=0x80;
            if(Read_only) DirLoc->Ext[0]|=0x80;
            DirLoc->NumPage = ( unsigned char )NbPages++;	// Num�ro de l'entr�e dans le fichier
            TaillePage = (TailleFic - PosFile + 127) >> 7 ;	// Taille de la page (on arrondit par le haut)
            if ( TaillePage > 128 )				// Si y'a plus de 16k il faut plusieurs pages
                TaillePage = 128;
			
            DirLoc->NbPages = ( unsigned char )TaillePage;
            l = ( DirLoc->NbPages + 7 ) >> 3; //Nombre de blocs=TaillePage/8 arrondi par le haut
            memset( DirLoc->Blocks, 0, 16 );
            for ( j = 0; j < l; j++ ) { //Pour chaque bloc de la page
                Bloc = RechercheBlocLibre( MaxBloc );	//Met le fichier sur la disquette
                if ( Bloc ) {
                    DirLoc->Blocks[ j ] = ( unsigned char )Bloc;
                    WriteBloc( Bloc, &BufFile[ PosFile ] );
                    PosFile += 1024;	// Passe au bloc suivant
		}
                else
                    return( ERR_NO_BLOCK );
				
	    }
            SetInfoDirEntry( PosDir, DirLoc );
	}
        else
            return( ERR_NO_DIRENTRY );
	}
    return( ERR_NO_ERR );
}


//
// Retourne une entr�e du r�pertoire
//
StDirEntry * DSK::GetInfoDirEntry( int NumDir ) {
    static StDirEntry Dir;
    int MinSect = GetMinSect();
    int s = ( NumDir >> 4 ) + MinSect;
    int t = ( MinSect == 0x41 ? 2 : 0 );
    if ( MinSect == 1 )
        t = 1;
	
    memcpy( &Dir
			, &ImgDsk[ ( ( NumDir & 15 ) << 5 ) + GetPosData( t, s, true ) ]
			, sizeof( StDirEntry )
			);
    return( &Dir );
}


//
// V�rifier si DSK est "standard" (DATA ou VENDOR)
//
bool DSK::CheckDsk( void ) {
    CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
    if ( Infos->NbHeads == 1 ) {
        int MinSectFirst = GetMinSect();
        if ( MinSectFirst != 0x41 && MinSectFirst != 0xC1 && MinSectFirst != 0x01 )
		{
			cout << "DSK has wrong sector number!" << endl;
            return( false );
		}
		
       
        if ( Infos->NbTracks > 42 )
            Infos->NbTracks = 42;
		
        for ( int track = 0; track < Infos->NbTracks; track++ ) {
            // Recherche position secteur
            int Pos = sizeof( CPCEMUEnt ) + ( 0x1200 + sizeof( CPCEMUTrack ) ) * track;
            CPCEMUTrack * tr = ( CPCEMUTrack * )&ImgDsk[ Pos ];
			
            int MinSect = 0xFF, MaxSect = 0;
            if ( tr->NbSect != 9 )
			{
				cout << "Warning : track " << track <<" has "<<(int)tr->NbSect<<" sectors ! (wanted 9)" << endl;
                // return( false );
			}
            for ( int s = 0; s < (int)tr->NbSect; s++ ) {
                if ( MinSect > tr->Sect[ s ].R )
                    MinSect = tr->Sect[ s ].R;
				
                if ( MaxSect < tr->Sect[ s ].R )
                    MaxSect = tr->Sect[ s ].R;
			}
            if ( MaxSect - MinSect != 8 )
			{
				cout << "Warning : strange sector numbering in track "<<track<<"!" << endl;
                // return( false );
			}
            if ( MinSect != MinSectFirst )
			{
				cout << "Warning : track "<<track<<" start at sector"<<MinSect<<" while track 0 starts at "<<MinSectFirst << endl;
                //return( false );
			}
		}
        return( true );
	}
	cout << "Multi-side dsk ! Expected 1 head, got " << (int)Infos->NbHeads << endl;
    return( false );
}


//
// Lire un fichier DSK
//
bool DSK::ReadDsk( std::string NomFic ) {
    bool Ret = false;
    CPCEMUEnt * Infos;
	if(sizeof(CPCEMUEnt) != 0x100) cout << "INVALID DSK BUILD" << endl;
    FILE* fp ;
	
    if ( (fp=fopen(NomFic.c_str(),"rb"))!=NULL ) {
		fread(ImgDsk,sizeof(ImgDsk),1,fp);
		Infos = ( CPCEMUEnt * )ImgDsk;
        if ( isBigEndian( ) ) FixEndianDsk( false ); // fix endian for Big endianness machines (PPC)
		if (  ! strncmp( Infos->debut, "MV -", 4 )
			  || ! strncmp( Infos->debut, "EXTENDED CPC DSK", 16 )
			  )
            Ret = true;
		fclose(fp);
	}
    return( Ret );
}


//
// Formatter une disquette
//
void DSK::FormatDsk( int NbSect, int NbTrack ) {
    CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
	
    strcpy( Infos->debut, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n" );
    Infos->DataSize = ( short )( sizeof( CPCEMUTrack ) + (0x200 * NbSect) );
    Infos->NbTracks = ( unsigned char ) NbTrack;
    Infos->NbHeads = 1;
    for ( int t = 0; t < NbTrack; t++ )
        FormatTrack( Infos, t, 0xC1, NbSect );
    
	
    FillBitmap();
}



//
// Modifie le endianness de la disquette
//
void DSK::FixEndianDsk( bool littleToBig) {
    CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
	//std::cerr<< "FixEndianDsk() Infos->DataSize : " << Infos->DataSize <<std::endl;
	
	if ( ! littleToBig ) 
		Infos->DataSize = FIX_SHORT( Infos->DataSize );	
    for ( int t = 0; t < Infos->NbTracks; t++ )
        FixEndianTrack( Infos, t, 9 );
	if ( littleToBig )	
		Infos->DataSize = FIX_SHORT( Infos->DataSize );		
	FillBitmap();
}

//
// Modifie le endianness de la piste
//
void DSK::FixEndianTrack( CPCEMUEnt * Infos, int t, int NbSect ) {
    CPCEMUTrack *tr;
	if ( Infos->DataSize != 0 )
		tr = ( CPCEMUTrack * )&ImgDsk[ sizeof( CPCEMUEnt ) + t * Infos->DataSize ];
	else {
		int ExtendedDataSize = ImgDsk[ 0x34 + t ] *256; //case of a extended dsk image
		tr = ( CPCEMUTrack * )&ImgDsk[ sizeof( CPCEMUEnt ) + t * ExtendedDataSize ];
	}
	int ss = 0;
	
    //
    // Gestion "entrelacement" des secteurs
    //
    for ( int s = 0; s < NbSect; ) {
        tr->Sect[ s ].SizeByte = FIX_SHORT( tr->Sect[ s ].SizeByte );
		tr->Sect[ s ].Un1 = FIX_SHORT( tr->Sect[ s ].Un1 );
		ss++;
        if ( ++s < NbSect ) {
            tr->Sect[ s ].SizeByte = FIX_SHORT( tr->Sect[ s ].SizeByte );
			tr->Sect[ s ].Un1 = FIX_SHORT( tr->Sect[ s ].Un1 );
            s++;
		}
	}
	tr->Unused = FIX_SHORT( tr->Unused );
}


//
// Ecriture du fichier DSK
//
bool DSK::WriteDsk( string NomDsk ) {
    CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
    FILE* fp;
    int Taille,Copie;
    
    
    if ( (fp=fopen(NomDsk.c_str(),"wb+")) != NULL) {
		if ( ! Infos->DataSize ) Infos->DataSize = 0x100 + SECTSIZE * 9;
        Taille = Infos->NbTracks * Infos->DataSize + sizeof( * Infos );
        if ( isBigEndian() ) FixEndianDsk( true ) ; // Fix endianness for Big endian machines (PPC)
		
		if ( (Copie=(fwrite(ImgDsk,1,Taille,fp))) !=Taille )
			cerr << Copie << "!=" << Taille;
		fclose(fp);
		// in case of the same DSK image stay in memory
        if ( isBigEndian() ) FixEndianDsk( false ) ; // unFix endianness for Big endian machines (PPC)
		
        return( true );
	}
    return( false );
}


void DSK::DskEndian() {
	CPCEMUEnt * Infos = ( CPCEMUEnt * )ImgDsk;
	for ( int  i=1 ; i<(int)Infos->NbTracks ; i++) {
		CPCEMUTrack * TrackData = GetInfoTrack( i );
		TrackData = CPCEMUTrackEndian ( TrackData ) ;
	}
	Infos = CPCEMUEntEndian ( Infos ) ;
}


StAmsdos* DSK::StAmsdosEndian ( StAmsdos * pEntete ){
	pEntete->Length = FIX_SHORT( pEntete->Length );
	pEntete->Adress = FIX_SHORT( pEntete->Adress );
	pEntete->LogicalLength = FIX_SHORT( pEntete->LogicalLength);
	pEntete->EntryAdress = FIX_SHORT( pEntete->EntryAdress );
	pEntete->RealLength = FIX_SHORT( pEntete->RealLength );
	pEntete->CheckSum = FIX_SHORT( pEntete->CheckSum ) ;
	return ( pEntete );
}


CPCEMUEnt* DSK::CPCEMUEntEndian ( CPCEMUEnt* Infos ) {
	Infos->DataSize = FIX_SHORT( Infos->DataSize );
	return (Infos);
}


CPCEMUTrack* DSK::CPCEMUTrackEndian ( CPCEMUTrack* tr ) {
	for ( int i=0;i < (int)tr->NbSect ; i++) {
		tr->Sect[i] = CPCEMUSectEndian( tr->Sect[i] );
	}
	
	return ( tr);
	
}


CPCEMUSect DSK::CPCEMUSectEndian ( CPCEMUSect Sect) {
	Sect.Un1 = FIX_SHORT( Sect.Un1 );
	Sect.SizeByte = FIX_SHORT( Sect.SizeByte );
	return (Sect);
}

// Retourne le type de fichier sous forme de chaine
//
const char * DSK::GetType( int Langue, StAmsdos * Ams ) {
    if ( CheckAmsdos( ( unsigned char * )Ams ) ) {
        switch( Ams->FileType ) {
            case 0 :    // BASIC
				return( "BASIC"); //GetTexteLoc( 22, Langue ) );
				
            case 1 :    // BASIC (P)
				return( "BASIC(P)"); // GetTexteLoc( 23, Langue ) );
				
            case 2 :    // BINAIRE
				return("BINAIRE"); // GetTexteLoc( 24, Langue ) );
				
            case 3 :    // BINAIRE (P)
                return( "BINAIRE(P)"); //GetTexteLoc( 25, Langue ) );
				
            default :
				return( "INCONNU"); // GetTexteLoc( 26, Langue ) );
            }
        }
    return("ASCII"); // GetTexteLoc( 27, Langue ) );
}

char * DSK::GetEntryNameInCatalogue ( int num , char* Nom ) {
	int PosItem[ 64 ];
	StDirEntry TabDir[ 64 ];
	
	memset( PosItem, 0, sizeof( PosItem ) );
	
	for ( int i = 0; i < 64; i++ )
		memcpy( &TabDir[ i ], GetInfoDirEntry( i ), sizeof( StDirEntry ));
	
	for ( int i = 0; i < 64; i++ ) {
		SetInfoDirEntry( i, &TabDir[ i ] );
		
		if ( TabDir[ i ].User != USER_DELETED && ! TabDir[ i ].NumPage && num == i) {
			memcpy( Nom, TabDir[ i ].Nom, 8 );
			memcpy( &Nom[ 9 ], TabDir[ i ].Ext, 3 );
			Nom[ 8 ] = '.';
			Nom[ 12 ] = 0;
			for ( int j = 0; j < 12; j++ )
				Nom[ j ] &= 0x7F;
			for ( int j = 0; j < 12; j++ )
				if ( ! isprint( Nom[ j ] ) ) 
					Nom[ j ] = '?' ;
			return Nom;
		}
	}
	return Nom;
}

char * DSK::GetEntrySizeInCatalogue ( int num , char* Size ) {
	int PosItem[ 64 ];
	StDirEntry TabDir[ 64 ];
	
	
	memset( PosItem, 0, sizeof( PosItem ) );
	
	for ( int i = 0; i < 64; i++ )
		memcpy( &TabDir[ i ], GetInfoDirEntry( i ), sizeof( StDirEntry ));
	
	for ( int i = 0; i < 64; i++ ) {
		SetInfoDirEntry( i, &TabDir[ i ] );
		
		if ( TabDir[ i ].User != USER_DELETED && ! TabDir[ i ].NumPage && num == i) {
			int p = 0, t = 0;
            		do {
				if ( TabDir[ p + i ].User == TabDir[ i ].User ) {
					t += TabDir[ p + i ].NbPages; 
				}
				p++;
			}
			while( TabDir[ p + i ].NumPage && ( p + i ) < 64  );
			sprintf( Size, "%d Ko", ( t + 7 ) >>3 );
			return Size;
		}
	}
	return Size;
}


bool DSK::GetFileInDsk( char* path, int Indice ){
	int i = Indice;
	char current[ 16 ];
	char  NomIndice[ 16 ];
	int lMax = 0x1000000;
	int cumul=0;
	FILE* f;
	StDirEntry TabDir[ 64 ];
	
	if ( (f=fopen(path,"wb"))==NULL )
		return false;
	
	for ( int i = 0; i < 64; i++ )
		memcpy( &TabDir[ i ], GetInfoDirEntry( i ), sizeof( StDirEntry ));
		
		
	memset( NomIndice, 0 , sizeof( NomIndice ) );
	strncpy( NomIndice, GetNomAmsdos( TabDir[ i ].Nom ), 16);
	strncat( NomIndice, GetNomAmsdos( TabDir[ i ].Ext), 3);

	do
    {
		// Longueur du fichier
		int l = ( TabDir[ i ].NbPages + 7 ) >> 3;
		for ( int j = 0; j < l; j++ ) {
			int TailleBloc = 1024;
			unsigned char * p = ReadBloc( TabDir[ i ].Blocks[ j ] );
			int NbOctets = min( lMax, TailleBloc );
			if ( NbOctets > 0 ) {
				fwrite(p,1,NbOctets,f);
				cumul+=NbOctets;
			}	
			lMax -= 1024;
        }
		memset( current , 0, sizeof( current ) );
		i++;
		strncpy(current, GetNomAmsdos( TabDir[ i ].Nom ), 16 );
		strncat(current, GetNomAmsdos( TabDir[ i ].Ext ), 3);
		
		if ( i > 64 ) return false;
    }while (! strncmp( NomIndice, current , max( strlen( NomIndice ), strlen( current ) )));
	
	fclose (f);
	return true;
}


bool DSK::PutFileInDsk( string Masque ,int TypeModeImport ,int loadAdress, int exeAdress, int UserNumber, bool System_file, bool Read_only ) {
	static unsigned char Buff[ 0x20000 ];
	static char *cFileName;
	unsigned long Lg;
	bool ret;
	FILE* Hfile;
	if ( NULL==(cFileName = (char*)malloc(16*sizeof(char))) )
		return false;
	
	

	cFileName = GetNomAmsdos((char *)Masque.c_str());
	if ((  Hfile = fopen(Masque.c_str(),"rb")) == NULL ) return false;
        Lg=fread(Buff,1, 0x20000 ,Hfile);
	fclose( Hfile );
        bool AjouteEntete = false;
        StAmsdos * e = ( StAmsdos * )Buff;
        // Attention : longueur > 64Ko !
        if ( Lg > 0x10080 ) {
		free(cFileName);
		return false;
	}
		
	if (TypeModeImport == MODE_ASCII) {
		for (int i=0 ; i < 0x20000 ; i++) {
			// last ascii char
			if (Buff[i] > 136) {
				Buff[i] = '?'; // replace by unknown char
			}
		}
	}
        //
        // Regarde si le fichier contient une en-tete ou non
        //
        bool IsAmsdos = CheckAmsdos( Buff );
      
      	if ( ! IsAmsdos ) {
		// Creer une en-tete amsdos par defaut
		cout << "Cr�ation automatique d'une en-t�te pour le fichier ...\n";
		e = CreeEnteteAmsdos( cFileName, ( unsigned short )Lg );
		if ( loadAdress != 0)
		{
			e->Adress = (unsigned short)loadAdress;
			TypeModeImport = MODE_BINAIRE;
		}
		if ( exeAdress != 0 )
		{
			e->EntryAdress = (unsigned short)exeAdress;
			TypeModeImport = MODE_BINAIRE;
		}
		// Il faut recalculer le checksum en comptant es adresses !
		SetChecksum(e);
        // fix the endianness of the input file
        if ( isBigEndian() ) e = StAmsdosEndian(e);
	}
	else
		cout << "Le fichier a d�j� une en-t�te\n";
        //
        // En fonction du mode d'importation...
        //
        switch( TypeModeImport ) {
		case MODE_ASCII :
			//
			// Importation en mode ASCII
			//
			if ( IsAmsdos ) {
				// Supprmier en-tete si elle existe
				memcpy( Buff, &Buff[ sizeof( StAmsdos ) ], Lg - sizeof( StAmsdos ));
				Lg -= sizeof( StAmsdos );
			}
		break;
				
		case MODE_BINAIRE :
			//
			// Importation en mode BINAIRE
			//
				
			if ( ! IsAmsdos )
				//
				// Indique qu'il faudra ajouter une en-tete
				//
				AjouteEntete = true;
		break;
				
	}
		
        //
        // Si fichier ok pour etre import
        //
        if ( AjouteEntete ) {
        	// Ajoute l'en-tete amsdos si necessaire
        	
		memmove( &Buff[ sizeof( StAmsdos ) ], Buff, Lg );
               	memcpy( Buff, e, sizeof( StAmsdos ) );
               	Lg += sizeof( StAmsdos );
	}

	//if (MODE_BINAIRE) ClearAmsdos(Buff); //Remplace les octets inutilis�s par des 0 dans l'en-t�te

        if ( CopieFichier( Buff,cFileName,Lg,256, UserNumber, System_file, Read_only) != ERR_NO_ERR )
		ret = false;
	else 
		ret = true;	

	return ret;
}


bool DSK::OnViewFic(int nItem) {
	int LongFic = 0;
	memset( BufFile, 0, sizeof( BufFile ) );
	memset( Listing, 0, sizeof( Listing ) );
	char NomFic[ 16 ];
	char current[ 16 ];
	int i = nItem;
	bool FirstBlock = true;
	StDirEntry TabDir[ 64 ];
	
	for ( int j = 0; j < 64; j++ )
		memcpy( &TabDir[ j ], GetInfoDirEntry( j ), sizeof( StDirEntry ));
	
	memset( NomFic, 0 , sizeof( NomFic ) );
	strncpy( NomFic, GetNomAmsdos( TabDir[ i ].Nom ), 16);
	strncat( NomFic, GetNomAmsdos( TabDir[ i ].Ext), 3);

	int lMax = sizeof( BufFile );
	
	TailleFic = 0;
	
	
	do
	{
        // Longueur du fichier
		int l = ( TabDir[ i ].NbPages + 7 ) >> 3;
		for ( int j = 0; j < l; j++ ) {
			int TailleBloc = 1024;
			unsigned char * p = ReadBloc( TabDir[ i ].Blocks[ j ] );
			if ( FirstBlock ) {
				if ( CheckAmsdos( p ) )  {
					TailleFic = p[ 0x18 +1 ] *256 + p[ 0x18 ];
					TailleBloc -= sizeof( StAmsdos );
					memcpy( p  , &p[ 0x80 ]  , TailleBloc  );
				}
				FirstBlock = false;
				
			}
			int NbOctets = min( lMax, TailleBloc );
			if ( NbOctets > 0 ) {
				memcpy( &BufFile[ LongFic ], p, NbOctets );
				LongFic += NbOctets;     
			}
			lMax -= 1024;
		}
		memset( current , 0, sizeof( current ) );
		i++;
		strncpy(current, GetNomAmsdos( TabDir[ i ].Nom ), 16 );
		strncat(current, GetNomAmsdos( TabDir[ i ].Ext ), 3);
		if ( i > 64 ) return false;
	}while( ! strncmp( NomFic, current, max( strlen( current ), strlen( NomFic ) ) ) );
	
	if ( TailleFic == 0 )
		TailleFic = LongFic;
	return true;
}


bool DSK::Hexdecimal() {

	int TailleCourante=0;
	char OffSet[ 7 ];
	const char * CodeHexa = "0123456789ABCDEF";
	
	while (TailleCourante <=  TailleFic ) {
		// display the offset 
		memset( OffSet, 0 , 7 );
		snprintf( OffSet,6,"#%.4X:", TailleCourante );
		strcat( Listing, OffSet );
		strcat( Listing, " ");
		char Ascii[ 18 ]; 
		char Hex[ 16 *3 +1 ];
		memset( Ascii, 0 , 18 );
		memset( Hex , 0 , ( 16*3 +1) );
		for ( int i=0; i<16 ; ++i ) {
			unsigned char cur = BufFile[ TailleCourante + i ];
			// manage the ascii display
			if ( cur > 32 && cur < 125 ) 
				Ascii[ i ] = cur;
			else 
				Ascii[ i ] = '.';
			char Val[ 4 ];
			// manage the hexadeciaml display
			Val[ 0 ] = CodeHexa[ cur >> 4 ];
			Val[ 1 ] = CodeHexa[ cur & 0x0F ];
			Val[ 2 ] = ' ';
			Val[ 3 ] ='\0';
			strcat( Hex, Val );
		}
		Ascii[ 16 ] = '\n';
		strcat( Listing, Hex );
		strcat( Listing, "| ");
		strcat( Listing, Ascii );
		TailleCourante += 16; 
	}
	
	return true;
}


void DSK::RemoveFile ( int item ) {
	char NomFic[ 16 ];
	int i = item;
	StDirEntry TabDir[ 64 ];
	
	for ( int j = 0; j < 64; j++ )
		memcpy( &TabDir[ j ], GetInfoDirEntry( j ), sizeof( StDirEntry ));
		
	strcpy( NomFic, GetNomAmsdos( TabDir[ i ].Nom ) );
	char *p ;
	
	do {
		TabDir[ i ].User = USER_DELETED;
		SetInfoDirEntry( i, &TabDir[ i ]);
		p = GetNomAmsdos( TabDir[ ++i ].Nom) ;
	} while ( ! strncmp( NomFic, p , max(strlen( p ), strlen( NomFic ) )) );
	
		
	return ;
}



void DSK::RenameFile( int item , char *NewName) {
	char NomFic[ 16 ];
	StDirEntry TabDir[ 64 ];
	StDirEntry DirLoc;
	int c = item;
	for ( int j = 0; j < 64; j++ )
		memcpy( &TabDir[ j ], GetInfoDirEntry( j ), sizeof( StDirEntry ));
		
	memset( DirLoc.Nom, ' ', 8);
	memset( DirLoc.Ext, ' ', 3);
	for ( int i=0; i<(int) strlen( NewName ) ; ++i)
		NewName[ i ] = toupper( NewName[ i ] );
		
	char *p = strchr( NewName, '.');
	
	if ( p ) {
		p++;
		memcpy( DirLoc.Nom, NewName, p - NewName -1);
		memcpy( DirLoc.Ext, p, std::min((int)strlen(p),3) );
	}	
	else {
		memcpy( DirLoc.Nom, NewName, min( (int)strlen( NewName) , 8 ) );
	}
	strcpy( NomFic, GetNomAmsdos( TabDir[ c ].Nom ));
	
	do {
		memcpy( TabDir[ c ].Nom , DirLoc.Nom , 8 );
		memcpy( TabDir[ c ].Ext , DirLoc.Ext, 3 );
		SetInfoDirEntry( c, &TabDir[ c ]);
		p = GetNomAmsdos( TabDir[ ++c ].Nom );
	}while (!strncmp( NomFic, p , max(strlen(p),strlen(NomFic))));		
}


std::string DSK::ReadDskDir( void ) {
	StDirEntry TabDir[ 64 ];
	string catalogue;
	for ( int i = 0; i < 64; i++ ) {
		memcpy( &TabDir[ i ]
				, GetInfoDirEntry( i )
				, sizeof( StDirEntry )
			  );
	}
	// Trier les fichiers
	for ( int i = 0; i < 64; i++ ) {
		//
		// Afficher les fichiers non effac�s
		//
		if ( TabDir[ i ].User != USER_DELETED && ! TabDir[ i ].NumPage ) {
			char Nom[ 13 ];
			memcpy( Nom, TabDir[ i ].Nom, 8 );
			memcpy( &Nom[ 9 ], TabDir[ i ].Ext, 3 );
			Nom[ 8 ] = '.';
			Nom[ 12 ] = 0;
			//
			// Masquer les bits d'attributs
			//
			for ( int j = 0; j < 12; j++ )
			{
				Nom[ j ] &= 0x7F;

				if ( ! isprint( Nom[ j ] ) ) 
					Nom[ j ] = '?' ;
			}

			catalogue += Nom;
			catalogue += " "; 
			ostringstream c;
			c << (int)TabDir[i].User;
			catalogue += c.str();
			//
			// Calcule la taille du fichier en fonction du nombre de blocs
			//
			int p = 0, t = 0;
			do {
				if ( TabDir[ p + i ].User == TabDir[ i ].User )
					t += TabDir[ p + i ].NbPages;
				p++;
			} while( TabDir[ p + i ].NumPage && ( p + i ) < 64  );
            //string size = GetTaille( ( t + 7 ) >> 3  );
            //catalogue+= " : " + size + "\n";
            catalogue += "\n";

		}
	}
	return catalogue;
}
