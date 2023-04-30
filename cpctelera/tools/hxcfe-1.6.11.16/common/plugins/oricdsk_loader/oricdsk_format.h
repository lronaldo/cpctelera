/*
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Jean-Fran�ois DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#pragma pack(1)

// 256 bytes file header
typedef struct oricdsk_fileheader_
{
 char headertag[8]; // "ORICDSK" ou "MFM_DISK"
 unsigned long number_of_side;
 unsigned long number_of_tracks;
 unsigned long number_of_sectors_geometrie;
}oricdsk_fileheader;

#pragma pack()

/*

  Here a small discussion about the oric dsk format found 
  in the CEO-Mag (Club Europe Oric) number 153 (01/2003)


Retour sur les en-t�tes des fichiers dsk
par Fabrice Franc�s  <xxxxxxxxxxxxxxxxxxxxxxx>

Le 29-11-02, de Andr� Ch�ramy :  Fabrice, si je peux me permettre d'abuser encore 
de ta gentillesse,pourrais-tu encore me d�crire les headers des fichiers dsk 
"old" et "mfm" ?

Le 03-12-02, r�ponse de Fabrice : Voici le format des fichiers "old" et "mfm":

Pour "OLD", un en-t�te de 256 octets: 
- une signature de 8 octets: ORICDISK
- le nombre de faces (sur 32 bits little-endian).
- le nombre de pistes (32 bits)
- le nombre de secteurs (32 bits)
- le reste de l'ent�te inutilis�

Viennent ensuite les donn�es des secteurs de toutes les pistes de la premi�re face, 
puis de la seconde Y face s'il y en a une, etc. 
Implicitement, les secteurs ont tous une taille de 256 octets.

[Exemple, voici les 20 octets d'en-t�te de la disquette Sedoric 3.0 au format "old" : 
4F5249434449534B 02000000 50000000 11000000 soit:
ORICDISK (8 octets), 2 faces (4 octets), 80 pistes (4 octets) et 17 secteurs (4 octets).
Les 246 octets suivants sont inutilis�s (en g�n�ral des z�ros). 
Pour le nombre de face #01=une et #02=deux.]

POUR LE FORMAT "MFM", toujours un en-t�te de 256 octets :

- une signature de 8 octets: MFM_DISK
- le nombre de faces (toujours 32 bits little-endian)
- le nombre de pistes (32 bits)
- le type de g�om�trie (32 bits)
- le reste de l'ent�te est inutilis� actuellement mais r�serv� pour un �ventuel 
usage futur (par ex.: une extension du format pour prendre en compte correctement 
les disquettes BD-500 !)...

Viennent ensuite les contenus des pistes: implicitement chaque piste contient 6250 octets, 
compl�t�e par des octets inutilis�s pour avoir une taille multiple de 256, soit 6400 
(ce sont ces octets inutilis�s que j'ai r�quisitionn�s pour faire entrer 
la disquette BD-500 dans le format existant !). 
Le type de g�om�trie indique dans quel ordre viennent les pistes: 
la g�om�trie 1 donne d'abord toutes les pistes de la premi�re face, puis celle 
de la seconde, etc. ; la g�om�trie 2 donne d'abord les pistes du cylindre 0, puis celles 
des cylindres 1, 2, 3, etc. (la g�om�trie 1 est celle utilis�e par les OS Oric, 
la g�om�trie 2 celle utilis�e dans le monde hors-Oric). 
Comme je le disais, je risque d'�tendre l'ent�te MFM en rajoutant un champ donnant 
le nombre d'octets dans chaque piste, la valeur 0 actuelle signifiera 6250.

[Exemple, voici les 20 octets d'en-t�te de la disquette Sedoric 3.0 au format "mfm":  
4D464D5F4449534B 02000000 50000000 01000000 soit: 
MFM_DISK (8 octets), 2 faces (4 octets), 80 pistes (4 octets) et g�om�trie n�1 (4 octets).
Les 246 octets suivants sont inutilis�s pour l'instant. 
Pour le nombre de face #01=une et #02=deux. 
La g�om�trie utilis�e par Oric est toujours #01.]

Suite de Andr�: Merci pour ces indications. J'ai v�rifi� avec mes fichiers dsk et 
je m'y suis retrouv� (sauf que je ne sais pas ce qu'est un "petit indien") (little-endian). 
Si seulement 20 octets (plus potentiellement 4 pour le nombre d'octets par piste) sont r�serv�s,
peut-on utiliser le reste ? Je remarque que certains ent�te contiennent pas mal de "garbage". 
Et si j'y �crivais des commentaires sur la disquette?

R�ponse de Fabrice: Concernant la place inutilis�e dans les ent�tes de fichiers dsk, 
on peut se mettre d'accord pour que seulement les 24 ou 32 premiers octets soient r�serv�s 
pour des extensions futures et que le reste puisse �tre utilis� "librement" 
(si chacun fait ses propres extensions, �a risque de poser des probl�mes: 
tu as raison, un commentaire est peut-�tre le plus indiqu�).

Conclusion de Andr� : Bon, 32 octets me semblent plus s�r �tant donn� ton imagination...
Dans le Ceo-Mag, j'indiquerai donc que le reste est utilisable pour mettre des commentaires personnels

*/
