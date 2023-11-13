/*
  t0.c
  Segedlet a T0 idozito/szamlalo kezelesehez
  (c) Drotos Daniel, 2005
*/

#include "hw.h"
#include <stdio.h>

#include "timer.h"
#include "t0.h"



/* Adott idoziteshez szukseges szamlalo kezdoerteket szamolja ki.
   A parametert msec-ben kell megadni (kb 0.02-71 kozott) */

unsigned int T0kezdoertek(float ido /* msec */)
{
  return T_kezdoertek(ido);
}


/* Idozito uzemmod */

void T0idozito1(bool kapuzott)
{
  TR0= 0;
  TMOD&= 0xf0;
  TMOD|= 0x01;
  if (kapuzott)
    TMOD|= 0x08;
}


/* Szamlalo uzemmod */

void T0szamlalo(bool kapuzott)
{
  TR0= 0;
  TMOD&= 0xf0;
  TMOD|= 0x01;
  TMOD|= 0x04;
  if (kapuzott)
    TMOD|= 0x08;
}


/* Szamlalo ertek beallitasa */

void T0beallit(unsigned int kezdoertek)
{
  TH0= kezdoertek >> 8;
  TL0= kezdoertek & 0xff;
}


/* Szamlalo ertek beallitasa */

void T0ujratolt(unsigned int kezdoertek)
{
  TH0= kezdoertek >> 8;
  TL0= kezdoertek & 0xff;
}


/* A szamlalo aktualis allapota usec-ben */

float T0eltelt_us(void)
{
  unsigned char h, l;
  h= TH0;
  l= TL0;
  return (12.0/Fosc) * ((unsigned int)(h*256+l));
}


/* A szamlalo tulcsordulasaig hatralevo ido usec-ben */

float T0hatravan_us(void)
{
  unsigned char h, l;
  h= TH0;
  l= TL0;
  return (12.0/Fosc) * (0x10000 - (unsigned int)(h*256+l));
}


/* End of t0.c */
