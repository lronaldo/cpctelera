#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm> // pour contourner un bug de std::vector ...

#include "getopt_pp.h" /* Command line handling */

using namespace std;

#include "MyType.h"
#include "GestDsk.h"
#include "Outils.h"
#include "Main.h"
#include "endianPPC.h"
#include "ViewFile.h"

int main(int argc, char **argv)
{
	bool IsDskLoc, IsDskSet,
		ModeListDsk, ModeImportFile,
		ModeRemoveFile,
		ModeDisaFile, ModeListBasic,
		ModeListDams, ModeListHex,
		ModeGetFile, ModeNewDsk, Force_Overwrite,
		Read_only, System_file, Split_lines, ModeListAscii, NoOptionSet;

	ModeListDsk = ModeImportFile = ModeListAscii =
		ModeRemoveFile = ModeDisaFile =
			ModeListBasic = ModeListDams = ModeListHex = ModeNewDsk =
				ModeGetFile = IsDskLoc = IsDskSet = Force_Overwrite = Read_only = System_file =  false;
	NoOptionSet = true;			

	string DskFile, AmsdosFile;
	vector<string> AmsdosFileList;

	int exeAdress = 0, loadAdress = 0, AmsdosType = 1, UserNumber = 0;

	DSK MyDsk;

	IsDsk = IsDskValid = false;
	IsDskSaved = true;

	// Récupération des arguments avec getopt_pp
	{
		using namespace GetOpt;
		GetOpt_pp opts(argc, argv);

		opts >> GlobalOption(DskFile);
		if (DskFile != "")
			IsDskSet = true;

		opts >> OptionPresent('l', "list", ModeListDsk)

			>> OptionPresent('i', "import", ModeImportFile) >> Option('i', "import", AmsdosFileList)

			>> OptionPresent('r', "remove", ModeRemoveFile) >> Option('r', "remove", AmsdosFileList)

			>> OptionPresent('n', "new", ModeNewDsk)

			>> OptionPresent('z', "disassemble", ModeDisaFile) >> Option('z', "disassemble", AmsdosFileList)

			>> OptionPresent('a', "ascii", ModeListAscii) >> Option('a', "ascii", AmsdosFileList)

			>> OptionPresent('b', "basic", ModeListBasic) >> Option('b', "basic", AmsdosFileList)

			>> OptionPresent('d', "dams", ModeListDams) >> Option('d', "dams", AmsdosFileList)

			>> OptionPresent('h', "hex", ModeListHex) >> Option('h', "hex", AmsdosFileList)

			>> std::hex >> Option('e', "exec", exeAdress) >> Option('c', "load", loadAdress) >> std::dec >> Option('t', "type", AmsdosType)

			>> OptionPresent('g', "get", ModeGetFile) >> Option('g', "get", AmsdosFileList)

			>> OptionPresent('f', "force", Force_Overwrite) >> OptionPresent('o', "write-protect", Read_only) >> OptionPresent('s', "system", System_file) >> OptionPresent('p', "split-lines", Split_lines) >> Option('u', "user", UserNumber);

		if (opts.options_remain())
		{
			cout << "Unhandled option ! Check the syntax." << endl;
			exit(EXIT_FAILURE);
		}

	} //namespace getopt

	if (!IsDskSet)
	{
		cerr << "You did not select a DSK file to work with !" << endl;
		help();
	}
	else
		cerr << "DSK : " << DskFile << endl;

	if (ModeListBasic || ModeListHex || ModeListDams || ModeDisaFile || ModeListAscii)
	{
		NoOptionSet = false;
		if (!MyDsk.ReadDsk(DskFile))
		{
			cerr << "Error reading file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		if (!MyDsk.CheckDsk())
		{
			cerr << "Fichier image non supporté (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		int Indice;
		for (vector<string>::iterator iter = AmsdosFileList.begin(); iter != AmsdosFileList.end(); iter++)
		{
			char *amsdosF = GetNomAmsdos(iter->c_str());
			cerr << "Amsdos file : " << amsdosF << endl;
			if ((Indice = MyDsk.FileIsIn(amsdosF)) < 0)
			{
				cerr << "Error: File " << amsdosF << " not found." << endl;
				exit(EXIT_FAILURE);
			}
			MyDsk.OnViewFic(Indice);

			if (ModeListBasic)
				cout << ViewBasic(Split_lines) << endl;
			else if (ModeListDams)
				cout << "Not yet coded ! Please try a newer version of iDSK ! Sorry !" << endl;
			else if (ModeListHex)
			{
				MyDsk.Hexdecimal();
				cout << Listing << endl;
			}
			else if (ModeDisaFile)
				cout << ViewDesass() << endl;
			else if (ModeListAscii)
				cout << ViewAscii() << endl;
		}
	}

	if (ModeNewDsk)
	{
		NoOptionSet = false;
		MyDsk.FormatDsk(9, 42);
		if (!MyDsk.WriteDsk(DskFile))
		{
			cerr << "Error writing file " << DskFile << endl;
			exit(EXIT_FAILURE);
		}
	}

	

	if (ModeImportFile)
	{ // Ajouter fichiers sur dsk
		NoOptionSet = false;
		if (!MyDsk.ReadDsk(DskFile))
		{
			cerr << "Error reading file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}

		if (!MyDsk.CheckDsk())
		{
			cerr << "Unsupported dsk file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}

		for (vector<string>::iterator iter = AmsdosFileList.begin(); iter != AmsdosFileList.end(); iter++)
		{
			string amsdosfile = GetNomAmsdos(iter->c_str());
			int Indice;
			// Ensure Indice is valid (the file is in the disk already) before atempting to remove it
			if ((Indice = MyDsk.FileIsIn(amsdosfile)) != -1)
			{
				if (!Force_Overwrite)
				{
					cerr << "(" << amsdosfile << ") File exists, replace ? (Y/N) (try -f switch for autoreplace...):";
					string answer;
					cin >> answer;
					if (toupper(answer[0]) == 'Y')
						MyDsk.RemoveFile(Indice);
					else
					{
						cerr << "Import cancelled, dsk unchanged." << endl;
						cout << MyDsk.ReadDskDir();
						exit(EXIT_SUCCESS);
					}
				}
				else
					MyDsk.RemoveFile(Indice);
			}
			cerr << "Amsdos file : " << *iter << endl;

			MyDsk.PutFileInDsk(*iter, AmsdosType, loadAdress, exeAdress, UserNumber, System_file, Read_only);
		}
		if (MyDsk.WriteDsk(DskFile))
			cout << MyDsk.ReadDskDir();
		else
			cerr << "Error writing file : " << DskFile << endl;
	}

	if (ModeRemoveFile)
	{
		NoOptionSet = false;
		if (!MyDsk.ReadDsk((char *)DskFile.c_str()))
		{
			cerr << "Error reading file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		if (!MyDsk.CheckDsk())
		{
			cerr << "unsupported DSK file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		int Indice;
		for (vector<string>::iterator iter = AmsdosFileList.begin(); iter != AmsdosFileList.end(); iter++)
		{
			char *amsdosF = GetNomAmsdos(iter->c_str());
			cerr << "Amsdos file : " << amsdosF << endl;
			if ((Indice = MyDsk.FileIsIn(amsdosF)) < 0)
			{
				cerr << "Error : file " << amsdosF << " not found." << endl;
				exit(EXIT_FAILURE);
			}
			MyDsk.RemoveFile(Indice);
			if (MyDsk.WriteDsk((char *)DskFile.c_str()))
				cout << MyDsk.ReadDskDir();
			else
				cerr << "Error writing file " << (*iter) << endl;
		}
	}

	if (ModeGetFile)
	{
		NoOptionSet = false;
		if (!MyDsk.ReadDsk((char *)DskFile.c_str()))
		{
			cerr << "Error reading dskfile (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		if (!MyDsk.CheckDsk())
		{
			cerr << "Unsupported dsk (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		int Indice;

		for (vector<string>::iterator iter = AmsdosFileList.begin(); iter != AmsdosFileList.end(); iter++)
		{
			char *amsdosF = GetNomAmsdos(iter->c_str());
			cerr << "Fichier Amsdos : " << amsdosF << endl;
			if ((Indice = MyDsk.FileIsIn(amsdosF)) < 0)
			{
				cerr << "Error : file " << amsdosF << " not found." << endl;
				exit(EXIT_FAILURE);
			}
			if (!MyDsk.GetFileInDsk((char *)(*iter).c_str(), Indice))
			{
				cerr << "System error : unable to copy (" << AmsdosFile << ")." << endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	if (ModeListDsk || NoOptionSet)
	{ // lire Dsk
		if (!MyDsk.ReadDsk(DskFile))
		{
			cerr << "Error reading file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		if (!MyDsk.CheckDsk())
		{
			cerr << "Unsupported dsk file (" << DskFile << ")." << endl;
			exit(EXIT_FAILURE);
		}
		cout << MyDsk.ReadDskDir();
	}
	
	cerr << "------------------------------------" << endl;

	return (EXIT_SUCCESS);
}

void help(void)
{
	cout << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "################################################################################" << endl;
	cout << VERSION << " (by Demoniak, Sid, PulkoMandy), http://github.com/cpcsdk " << endl;
	cout << "################################################################################" << endl;
	cout << endl;
	cout << "Usage : " << endl;
	cout << "\t" << PROGNAME << " <DSKfile> [OPTIONS] [files to process]" << endl;
	cout << "OPTIONS :                              EXAMPLE" << endl;
	cout << "-l : List disk catalog                 iDSK floppy.dsk -l (default option is no option is set)" << endl;
	cout << "-g : export ('Get') file               iDSK floppy.dsk -g myprog.bas" << endl;
	cout << "-r : Remove file                       iDSK floppy.dsk -r myprog.bas" << endl;
	cout << "-n : create New dsk file               iDSK floppy2.dsk -n" << endl;
	cout << "-z : disassemble a binary file         iDSK floppy.dsk -z myprog.bin" << endl;
	cout << "-b : list a Basic file                 iDSK floppy.dsk -b myprog.bas" << endl
		 << "-p : split lines after 80 char             ... -p" << endl;
	cout << "-a : list a Ascii file                 iDSK floppy.dsk -a myprog.txt" << endl;
	cout << "-d : list a Dams file                  iDSK floppy.dsk -d myprog.dms" << endl;
	cout << "-h : list a binary file as Hexadecimal iDSK floppy.dsk -h myprog.bin" << endl;
	cout << "-i : Import file                       iDSK floppy.dsk -i myprog.bas" << endl
		 << " -t : fileType (0=ASCII/1=BINARY)           ... -t 1" << endl;
	cout << " -e : hex Execute address of file           ... -e C000 -t 1" << endl;
	cout << " -c : hex loading address of file           ... -e C000 -c 4000 -t 1" << endl;
	cout << " -f : Force overwriting if file exists      ... -f" << endl
		 << " -o : insert a read-Only file               ... -o" << endl
		 << " -s : insert a System file                  ... -s" << endl
		 << " -u : insert file with User number          ... -u 3" << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "Please report bugs ! - Demoniak/Sid/PulkoMandy" << endl;
	exit(0);
}
