#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <algorithm> // pour contourner un bug de std::vector ...
#include <libgen.h>

#include "getopt_pp.h" /* Command line handling */

using namespace std;

#include "MyType.h"
#include "GestDsk.h"
#include "Outils.h"
#include "Main.h"
#include "endianPPC.h"
#include "Itoa.h"
#include "ViewFile.h"



int main(int argc, char** argv) {
  bool IsDskLoc, IsDskSet,
       ModeListDsk, ModeImportFile, 
       ModeRemoveFile,
       ModeDisaFile, ModeListBasic, 
       ModeListDams,ModeListHex, 
       ModeGetFile, ModeNewDsk, Force_Overwrite;
       
  ModeListDsk =  ModeImportFile =
  ModeRemoveFile  = ModeDisaFile = 
  ModeListBasic = ModeListDams = ModeListHex = ModeNewDsk =
  ModeGetFile = IsDskLoc = IsDskSet = Force_Overwrite = false ;
  
  string DskFile, AmsdosFile;
  vector<string> AmsdosFileList;

  int exeAdress=0,loadAdress=0,AmsdosType=1;
  
  DSK MyDsk;
  
  IsDsk = IsDskValid = false;
  IsDskSaved = true;

  // Récupération des arguments avec getopt_pp
{using namespace GetOpt;
  GetOpt_pp opts(argc,argv);

  opts 	>> OptionPresent(GetOpt_pp::EMPTY_OPTION,IsDskSet)
  	>> Option(GetOpt_pp::EMPTY_OPTION,DskFile)

  	>> OptionPresent('l',"list",ModeListDsk)

	>> OptionPresent('i',"import",ModeImportFile)
	>> Option('i',"import",AmsdosFileList)

	>> OptionPresent('r',"remove",ModeRemoveFile)
	>> Option('r',"remove",AmsdosFileList)

	>> OptionPresent('n',"new",ModeNewDsk)

	>> OptionPresent('z',"disassemble",ModeDisaFile)
	>> Option('z',"disassemble",AmsdosFileList)

	>> OptionPresent('b',"basic",ModeListBasic)
	>> Option('b',"basic",AmsdosFileList)

	>> OptionPresent('d',"dams",ModeListDams)
	>> Option('d',"dams",AmsdosFileList)

	>> OptionPresent('h',"hex",ModeListHex)
	>> Option('h',"hex",AmsdosFileList)

	>> std::hex 	>> Option('e',"exec",exeAdress)
			>> Option('c',"load",loadAdress)
	>> std::dec	>> Option('t',"type",AmsdosType)

	>> OptionPresent('g',"get",ModeGetFile)
	>> Option('g',"get",AmsdosFileList)

	>> OptionPresent('f',"force",Force_Overwrite)
	;

	if(opts.options_remain())
	{
		cout << "Trop d'options !" << endl;
		exit(EXIT_FAILURE);
	}

}//namespace getopt

    if ( ! IsDskSet ) {
	    cerr << "Vous n'avez pas selectionné de fichier image DSK" << endl;
	    help();
    }
    else cerr << "DSK : " << DskFile <<  endl;
    
    if ( ModeListBasic || ModeListHex || ModeListDams || ModeDisaFile )
    {
    	if ( ! MyDsk.ReadDsk(DskFile))
	{
    		cerr<< "Erreur de lecture du fichier ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}
    	if ( ! MyDsk.CheckDsk() ) {
    		cerr <<"Fichier image non supporté ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}
    	int Indice;
	for(vector<string>::iterator iter=AmsdosFileList.begin(); iter!=AmsdosFileList.end(); iter++)
	{
		char* amsdosF = GetNomAmsdos(basename((char *)(*iter).c_str()));			 
		cerr << "Fichier Amsdos : " << amsdosF << endl;
		if ( (Indice= MyDsk.FileIsIn( amsdosF ))<0) {
			cerr << "Erreur Fichier : "<< amsdosF << " non trouve."<< endl;
			exit(EXIT_FAILURE);
		}
		MyDsk.OnViewFic(Indice);
	
		if ( ModeListBasic ) 
			cout << ViewBasic( ) << endl;
		else if ( ModeListDams )
			cout << "Pas implementé pour le moment."<<endl;
		else if ( ModeListHex ) {
			MyDsk.Hexdecimal();
			cout << Listing << endl;
		}
		else if ( ModeDisaFile )
			cout << ViewDesass(  )<<endl;
    	}
    }

    if(ModeNewDsk)
    {
   	MyDsk.FormatDsk( 9, 42 );
	if (! MyDsk.WriteDsk(DskFile))
	{
		cerr <<  "Erreur Ecriture fichier " << DskFile << endl;
		exit(EXIT_FAILURE);
	}
    }

    if ( ModeListDsk ) { // lire Dsk 
	   cerr << "Mode : List Dsk " << endl;
	   cerr <<  "------------------------------------" << endl;
	   if ( ! MyDsk.ReadDsk(DskFile) ) {
    		cerr<< "Erreur de lecture du fichier ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}
    	if ( ! MyDsk.CheckDsk() ) {
    		cerr <<"Fichier image non supporter ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}
    	cout << MyDsk.ReadDskDir();
    }

    if ( ModeImportFile ) { // Ajouter fichiers sur dsk
 	cerr << "Mode : Import fichier dans Dsk " << endl;
 	if ( ! MyDsk.ReadDsk(DskFile) ) {
		cerr<< "Erreur de lecture du fichier ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}

    	if ( ! MyDsk.CheckDsk() ) {
    		cerr <<"Fichier image non supporté ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}

   	for(vector<string>::iterator iter=AmsdosFileList.begin(); iter!=AmsdosFileList.end(); iter++)
   	{
   		char* nomBase=basename((char*)iter->c_str());
    		string amsdosfile = GetNomAmsdos( nomBase );
    		int Indice;
         if ( ( Indice = MyDsk.FileIsIn( amsdosfile ) ) != -1 ) {
            if ( !Force_Overwrite ) {
               cerr << "(" << amsdosfile <<") Fichier existe, voulez vous ajouter quand meme ? (O/Oui)(N/Non) :";
               string answer ;
               cin >> uppercase >> answer;
               if ( answer != "O" && answer != "N" ) {
                  cerr << "Soyez plus explicite ;)."<<endl;
                  continue;
               }
               if ( answer == "O" )
                  MyDsk.RemoveFile(Indice);
               else {
                  cerr<<"Import aborde, Dsk non modifiee."<<endl;
                  cout << MyDsk.ReadDskDir();
                  exit(EXIT_SUCCESS);
               }
            } else {  // Force_Overwrite = true
               std::cerr << "Removing file!\n";
               MyDsk.RemoveFile(Indice);
            }
         }
	
    		cerr << "Fichier Amsdos : "<< nomBase << endl;
	
		   MyDsk.PutFileInDsk(*iter,AmsdosType,loadAdress,exeAdress);
    	}
	if ( MyDsk.WriteDsk (DskFile) )
		cout << MyDsk.ReadDskDir(); 
	else cerr<< "Erreur ecriture fichier : " << DskFile << endl;
    }

    if ( ModeRemoveFile ) {
		cerr << "Mode : Effacement fichier dans la DSK"<<endl;
		if ( ! MyDsk.ReadDsk( (char*)DskFile.c_str() ) ) {
    			cerr<< "Erreur de lecture du fichier ("<< DskFile << ")."<<endl;
    			exit(EXIT_FAILURE);
    		}
    		if ( ! MyDsk.CheckDsk() ) {
    			cerr <<"Fichier image non supporté ("<< DskFile << ")."<<endl;
    			exit(EXIT_FAILURE);
    		}
    		int Indice;
		for(vector<string>::iterator iter=AmsdosFileList.begin(); iter!=AmsdosFileList.end(); iter++)
		{
			char* amsdosF = GetNomAmsdos(basename( (char*)(*iter).c_str()));			 
			cerr << "Fichier Amsdos : " << amsdosF << endl;
			if ( (Indice= MyDsk.FileIsIn( amsdosF ))<0) {
				cerr << "Erreur Fichier : "<< amsdosF << " non trouvé."<< endl;
				exit(EXIT_FAILURE);
			}
			MyDsk.RemoveFile(Indice);
			if ( MyDsk.WriteDsk ((char*)DskFile.c_str()) )
				cout << MyDsk.ReadDskDir(); 
			else cerr<< "Erreur ecriture fichier : " << (*iter) << endl;
		}
   }

   if ( ModeGetFile ) {
	cerr << "Mode : export fichier Amsdos " << endl;
	if ( ! MyDsk.ReadDsk( (char*)DskFile.c_str() ) ) {
    		cerr<< "Erreur de lecture du fichier ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}
    	if ( ! MyDsk.CheckDsk() ) {
    		cerr <<"Fichier image non supporter ("<< DskFile << ")."<<endl;
    		exit(EXIT_FAILURE);
    	}
    	int Indice;

	for(vector<string>::iterator iter=AmsdosFileList.begin(); iter!=AmsdosFileList.end(); iter++)
	{
    		char* amsdosF = GetNomAmsdos(basename( (char*)(*iter).c_str()));			 
		cerr << "Fichier Amsdos : " << amsdosF << endl;
		if ( (Indice= MyDsk.FileIsIn( amsdosF ))<0) {
			cerr << "Erreur Fichier : "<< amsdosF << " non trouve."<< endl;
			exit(EXIT_FAILURE);
		}
		if ( ! MyDsk.GetFileInDsk((char*)(*iter).c_str(),Indice) ) {
			cerr <<"Erreur systeme, ne peut copier ("<<AmsdosFile<<")."<<endl;
			exit(EXIT_FAILURE);
		}
	}
   }	
	
   cerr <<  "------------------------------------" << endl;

 	return(EXIT_SUCCESS);
}




void help(void)
{
  cout <<endl;
  cout << "--------------------------------------------------------------------------------" << endl;
  cout << "################################################################################"<< endl;
  cout <<  VERSION <<" (by Demoniak, Sid, PulkoMandy), http://github.com/cpcsdk " << endl;
  cout << "################################################################################"<< endl;
  cout << endl;
  cout << "Usage : " << endl;
  cout << "\t"<< PROGNAME << " <DSKfile> [OPTIONS] [files to process]" << endl;
  cout << "OPTIONS :                              EXAMPLE" << endl;
  cout << " -l : List disk catalog                 iDSK floppy.dsk -l" << endl;
  cout << " -g : export ('Get') file               iDSK floppy.dsk -g myprog.bas"<<endl;
  cout << " -r : Remove file                       iDSK floppy.dsk -r myprog.bas" << endl;
  cout << " -n : create New dsk file               iDSK floppy2.dsk -n" << endl;
  cout << " -z : disassemble a binary file         iDSK floppy.dsk -z myprog.bin" << endl;
  cout << " -b : list a Basic file                 iDSK floppy.dsk -b myprog.bas" << endl;
  cout << " -d : list a Dams file                  iDSK floppy.dsk -d myprog.dms" << endl;
  cout << " -h : list a binary file as Hexadecimal iDSK floppy.dsk -h myprog.bin" << endl;
  cout << " -i : Import file                       iDSK floppy.dsk -i myprog.bas" << endl
      <<  " -t : fileType (0=ASCII/1=BINARY)           ... -t 1" << endl; 
  cout << " -e : hex Execute address of file           ... -e C000 -t 1" << endl;
  cout << " -c : hex loading address of file           ... -e C000 -c 4000 -t 1" << endl;
  cout << " -f : Force overwriting if file exists      ... -f" << endl
       << " -o : insert a read-Only file               ... -o" << endl
       << " -s : insert a System file                  ... -s" << endl
       << " -u : insert file with User number          ... -u 3" << endl;
  cout << "--------------------------------------------------------------------------------" << endl;
  cout << "Please report bugs ! - Demoniak/Sid/PulkoMandy" << endl;
  exit (0);
}

