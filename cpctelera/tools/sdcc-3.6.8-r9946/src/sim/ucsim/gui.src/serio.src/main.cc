/******************************************************************************
 * to emulate the serial input and output of an 8051 controller               *
 * main.cc - the main stuff                                                   *
 ******************************************************************************/
#include "ddconfig.h"

#include <sys/types.h>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#if defined(HAVE_GETOPT_H)
# include <getopt.h>
#endif

#include "fileio.hh"
#include "frontend.hh"
#include "posix_signal.hh"


// globals
int doloop = 1;

// the signal handler
void HandleSig(int info)
{
	doloop = 0;
}


// usage
void PrintUsage(char *progname)
{
std::cout << "Usage: " << progname << " [-i <filename>] [-o <filename>] [-h]\n";
std::cout << "-i <filename>\t<filename> is the pipe to the controllers' serial input\n";
std::cout << "-o <filename>\t<filename> is the pipe to the controllers' serial output\n";
std::cout << "-h\t\tshow the help\n";
std::cout << "\nTim Hurman - t.hurman@virgin.net\n";
exit(0);
}


// the main function
int main(int argc, char **argv)
{
	char *string = new char[MAX_SIZ];
	extern char *optarg;
	int errflg=0;
	int c;
	const char *infile = DEF_INFILE;
	const char *outfile = DEF_OUTFILE;

	// sort out any command line params
	while ((c = getopt(argc, argv, "i:o:h")) != EOF)
	switch(c) {
	case 'i':
		infile = optarg;
		break;
	case 'o':
		outfile = optarg;
		break;
	case 'h':
		errflg++;
		break;
	default:
		std::cerr << "Invalid or unknown switch\n";
		errflg++;
		break;
	}

	// was there a problem
	if(errflg)
		PrintUsage(argv[0]);

	// the main objects needed
	FileIO *fobj = new FileIO(infile, outfile);
	Viewer *view = new Viewer();
	SigHandler *sig = new SigHandler();

	// add a signal handler for ^C
	sig->SetSignal(SIGINT, HandleSig);

	// set the timeout for waiting for a char
	while(doloop)
	{
		string[0] = view->GetChInWin();
		if(string[0] == 4)
			break;

		if(string[0] != 0)
			fobj->SendByte(string[0]);
		
		if(fobj->RecvStr(string) > 0)
			view->AddStrOutWin(string);

		usleep(5000);
	}

	delete fobj;
	delete view;
	delete sig;
	delete string;
	return(0);
}
