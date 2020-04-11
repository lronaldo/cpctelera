#ifndef GESTDSK_H
#define GESTDSK_H

#include <string>

#define     USER_DELETED    0xE5


extern char Listing[ 0x280000 ];
extern unsigned char BufFile[ 0x10000 ];


extern int TailleFic, CurLigne;

#pragma pack(1) //evite le padding des structures qui sont utilisées dans des memcpy par la suite

//
// Structure d'une entree AMSDOS
//
typedef struct
{
    unsigned char    UserNumber;             // 00 User
    unsigned char    FileName[15];           // 01-0F Nom + extension
    unsigned char    BlockNum;               // 10    Numéro du bloc (disquette)
    unsigned char    LastBlock;              // 11    Flag "dernier bloc" bloc (disquette)
    unsigned char    FileType;               // 12    Type de fichier
    unsigned short   Length;                 // 13-14 Longueur
    unsigned short   Adress;                 // 15-16 Adresse
    unsigned char    FirstBlock;             // 17    Flag premier bloc de fichier (disquette)
    unsigned short   LogicalLength;          // 18-19 Longueur logique
    unsigned short   EntryAdress;            // 1A-1B Point d'entree
    unsigned char    Unused[0x24];
    unsigned short   RealLength;             // 40-42 Longueur reelle
    unsigned char    BigLength;              //       Longueur reelle (3 octets)
    unsigned short   CheckSum;               // 43-44 CheckSum Amsdos
    unsigned char    Unused2[0x3B];
} StAmsdos;


#define SECTSIZE    512


typedef struct
{
    char                debut[0x30];    // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
    unsigned char       NbTracks;
    unsigned char       NbHeads;
    unsigned short      DataSize;       // 0x1300 = 256 + ( 512 * nbsecteurs )
    unsigned char       Unused[0xCC];
} CPCEMUEnt;


typedef struct
{
    unsigned char       C;              // track
    unsigned char       H;              // head
    unsigned char       R;              // sect
    unsigned char       N;              // size
    short               Un1;
    short               SizeByte;       // Taille secteur en octets
} CPCEMUSect;


typedef struct
{
    char                ID[0x10];       // "Track-Info\r\n"
    unsigned char       Track;
    unsigned char       Head;
    short               Unused;
    unsigned char       SectSize;       // 2
    unsigned char       NbSect;         // 9
    unsigned char       Gap3;           // 0x4E
    unsigned char       OctRemp;        // 0xE5
    CPCEMUSect          Sect[29];
} CPCEMUTrack;

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

// see http://www.cpcwiki.eu/index.php/Disk_structure#Directory_entries
PACK(typedef struct
{
    unsigned char       User;           // 00
    char                Nom[8];         // 01-08
    char                Ext[3];         // 09-0B
    unsigned char       NumPage;        // 0C
    unsigned char       Unused[2];      // 0D-0E
    unsigned char       NbPages;        // 0F
    unsigned char       Blocks[16];     // 10-1F
}) StDirEntry;

#pragma pack()

enum { ERR_NO_ERR = 0, ERR_NO_DIRENTRY, ERR_NO_BLOCK, ERR_FILE_EXIST };

bool CheckAmsdos( unsigned char * Buf );
StAmsdos * CreeEnteteAmsdos( char * NomFic, unsigned short Length );
void ClearAmsdos( unsigned char * Buf );
void SetChecksum( StAmsdos * pEntete );
bool CheckAmsdos( unsigned char * Buf );


class DSK
{
	unsigned char ImgDsk[ 0x80000 ];
	unsigned char Bitmap[ 256 ];
	
	unsigned char * GetRawData( int Pos );
	void WriteRawData( int Pos, unsigned char * Data, int Longueur );
	int GetNbTracks( void );
	void WriteBloc( int bloc, unsigned char * BufBloc );
	void WriteSect( int Track, int Sect, unsigned char * Buff, int AmsdosMode );
	unsigned char * ReadSect( int Track, int Sect, int AmsdosMode );
	CPCEMUTrack * GetInfoTrack( int Track );
	int FillBitmap( void );
	void DskEndian();
	CPCEMUEnt* CPCEMUEntEndian ( CPCEMUEnt* Infos );
	CPCEMUTrack* CPCEMUTrackEndian ( CPCEMUTrack* tr );
	CPCEMUSect CPCEMUSectEndian ( CPCEMUSect Sect);
	const char * GetType( int Langue, StAmsdos * Ams );
	int GetMinSect( void );
	int GetPosData( int track, int sect, bool SectPhysique );
	int RechercheBlocLibre( int MaxBloc );
	void FormatTrack( CPCEMUEnt * Infos, int t, int MinSect, int NbSect );
	
public:
	DSK(){}
	DSK(const DSK& d)
	{
		for (int i=0; i< 0x80000; i++)
			ImgDsk[i]=d.ImgDsk[i];
		for (int j=0; j< 256 ; j++ )
			Bitmap[j]=d.Bitmap[j];
	}

	~DSK(){}
	
	int GetTailleDsk();
	StDirEntry * GetNomDir(std::string Nom );
	int CopieFichier( unsigned char * BufFile, char * NomFic, int TailleFic, int MaxBloc, int, bool,bool );
	bool WriteDsk( std::string NomDsk );
	unsigned char * ReadBloc( int bloc );
	bool ReadDsk( std::string NomFic );
	bool CheckDsk( void );
	int FileExist( char * Nom );
	StDirEntry * GetInfoDirEntry( int NumDir );
	int FileIsIn( std::string FileName );
	int RechercheDirLibre( void );
	void FormatDsk( int NbSect, int NbTrack );
	StAmsdos* StAmsdosEndian ( StAmsdos * pEntete );
	void SetInfoDirEntry( int NumDir, StDirEntry * Dir );
	char * GetEntryNameInCatalogue ( int num , char* Nom );
	char * GetEntrySizeInCatalogue ( int num , char* Size );
	bool GetFileInDsk( char* path, int Indice );
	bool PutFileInDsk( std::string Masque ,int TypeModeImport ,int loadAdress, int exeAdress, int,bool,bool );
	bool OnViewFic(int nItem);
	bool Hexdecimal();
	void RemoveFile ( int item );
	void FixEndianDsk( bool LittleToBig );
	void FixEndianTrack( CPCEMUEnt * Infos, int t, int NbSect );
	void RenameFile( int item , char *NewName);
	std::string ReadDskDir(void);
	
};

#endif
