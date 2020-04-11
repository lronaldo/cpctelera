#include <iostream>
using namespace std;
#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctype.h>

#include "MyType.h"
#include "Ascii.h"

void Ascii(unsigned char *Prg, char *Listing, int Longueur)
{
    int Adr = 0;
    * Listing = 0;
    strncpy(Listing,(const char*)Prg,Longueur);
}