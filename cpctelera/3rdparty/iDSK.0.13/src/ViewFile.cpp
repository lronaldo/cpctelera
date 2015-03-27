#include <iostream>
using namespace std;
#include "GestDsk.h"
#include "Outils.h"
#include "Basic.h"
#include "Desass.h"
#include "Dams.h"
#include "endianPPC.h"
#include "ViewFile.h"

string ViewDams(  )
{
	cerr << "Taille du fichier : " << TailleFic << endl;
	Dams( BufFile, TailleFic , Listing );
	return Listing;
	//cout << Listing << endl;
}


string ViewDesass( )
{
	cerr << "Taille du fichier : " << TailleFic << endl;
    Desass( BufFile, Listing, TailleFic );
    return Listing;
	//cout << Listing << endl;
}
string ViewBasic( )
{
	bool IsBasic=true;
	//cout << "Entre Ici\n";
	cerr << "Taille du fichier : " << TailleFic << endl;
	Basic( BufFile, Listing, IsBasic, true );
	//cout << Listing << endl;
	return Listing;
	
}
