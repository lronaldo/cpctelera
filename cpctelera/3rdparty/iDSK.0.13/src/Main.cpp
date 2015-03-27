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
    		if ( ( Indice = MyDsk.FileIsIn( amsdosfile ) ) != -1 && !Force_Overwrite) {
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
		}
		else if(Force_Overwrite)
			MyDsk.RemoveFile(Indice);
	
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
  cout <<  VERSION <<" (auteurs  : Demoniak, Sid, PulkoMandy), Contact SiD@Gmail.CoM" << endl;
  cout << "################################################################################"<< endl;
  cout << endl;
  cout << "Usage : " << endl;
  cout << "\t"<< PROGNAME << " <fichier DSK> [OPTION] [fichiers a traiter]" << endl;
  cout << "OPTIONS :" << endl;
  cout << "-l : list le contenu du catalogue ex: -l mon_dsk.dsk" << endl;
  cout << "-i : importe un fichier (-t type 0 pour un fichier ASCII et 1 pour un BINAIRE) ex: -i mon_fichier.bin -t 1 -s mon_fichier.dsk" << endl; 
  cout << "-e : donne l'adresse d'execution du fichier (en hexadecimal) a inserer ex: -i mon_fichier.bin -e C000 -t 1 -s mon_fichier.dsk" << endl;
  cout <<"-c : donne l'adresse de chargement du fichier(en hexadecimal)  a inserer ex: -i mon_fichier.bin -e C000 -c 4000 -t 1 -s mon_fichier.dsk" << endl;
  cout << "-g : exporte un fichier ex: -g mon-fichier.bas -s mon_fichie.dsk"<<endl;
  cout << "-r : enlève un fichier" << endl;
  cout << "-n : cree une nouvelle image dsk ex: -n mon_fichier.dsk" << endl;
  cout << "-z : desassemble un binaire ex: -z mon-fichier.bin -s mon_fichier.dsk" << endl;
  cout << "-b : liste un fichier Basic ex: -b mon-fichier.bas -s mon_fichier.dsk" << endl;
  cout << "-d : liste un fichier Dams  ex: -d mon-fichier.dms -s mon_fichier.dsk" << endl;
  cout << "-h : liste un fichier en Hexadecimale ex: -h mon-fichier.bin -s mon_fichier.dsk" << endl;
  cout << "-f : force l'écrasement lorsqu'un fichier existe déjà" << endl;
  cout << "--------------------------------------------------------------------------------" << endl;
  cout << "*/\\/\\/SiD oF ImPAct/\\/\\/*" << endl;
  exit (0);
}

