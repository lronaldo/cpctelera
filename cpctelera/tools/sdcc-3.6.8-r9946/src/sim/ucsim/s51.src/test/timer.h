#ifndef TIMER_HEADER
#define TIMER_HEADER

/*
  Oszcillator orajele MHz-ben. Kezdoerteke 11.0592
*/
extern float Fosc;

/*
  A megadott ideig tarto idoziteshez szukseges
  szamlalo kezdoerteket szamolja ki. Az idot
  msec-ben kell megadni, kb 0.02-71.1 kozott.
  Felhasznalja az Fosc erteket a szamitashoz.
*/
extern unsigned int T_kezdoertek(float ido);

#endif
