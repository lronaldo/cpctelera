#include "timer.h"


float Fosc= 11.0592; /* MHz, Oszcillator orajel */


/* Adott idoziteshez szukseges szamlalo kezdoerteket szamolja ki.
   A parametert msec-ben kell megadni (kb 0.02-71 kozott) */

unsigned int T_kezdoertek(float ido /* msec */)
{
  float tper;	/* szamlalo periodusideje */
  //float per;	/* szukseges periodusok szama */

  tper= 12.0/Fosc;
  /* atvaltas usec-be */
  ido*= 1000.0;
  /* Max 65529 lepes */
  if (ido > tper*65529.0)
    return 0;
  /* Min 20 lepes */
  if (ido < tper*20.0)
    return 0xffff-20;
  //per= ido/tper;
  return (unsigned int)(65541.0-ido/tper);
}

