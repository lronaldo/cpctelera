#include <iostream>
using namespace std;
#include "GestDsk.h"
#include "Outils.h"
#include "Basic.h"
#include "Desass.h"
#include "Dams.h"
#include "endianPPC.h"
#include "ViewFile.h"
#include "Ascii.h"

string ViewDams()
{
	cerr << "Taille du fichier : " << TailleFic << endl;
	Dams(BufFile, TailleFic, Listing);
	return Listing;
	//cout << Listing << endl;
}

string ViewDesass()
{
	cerr << "Taille du fichier : " << TailleFic << endl;
	Desass(BufFile, Listing, TailleFic);
	return Listing;
	//cout << Listing << endl;
}

string ViewBasic(bool AddCrLf)
{
	bool IsBasic = true;
	//cout << "Entre Ici\n";
	cerr << "Taille du fichier : " << TailleFic << endl;
	Basic(BufFile, Listing, IsBasic, AddCrLf);
	//cout << Listing << endl;
	return Listing;
}

string ViewAscii()
{
	cerr << "Taille du fichier : " << TailleFic << endl;
	Ascii(BufFile, Listing, TailleFic);
	//cout << Listing << endl;
	return Listing;
}