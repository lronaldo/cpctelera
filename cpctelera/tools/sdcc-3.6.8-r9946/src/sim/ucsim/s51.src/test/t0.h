/*
  T0.h
  Segedlet a T0 idozito hasznalatahoz
  (c) Drotos Daniel, 2005
*/

#ifndef T0_HEADER
#define T0_HEADER

#ifndef bool
#define bool char
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


/*
  A megadott ideig tarto idoziteshez szukseges
  szamlalo kezdoerteket szamolja ki. Az idot
  msec-ben kell megadni, kb 0.02-71.1 kozott.
  Felhasznalja az Fosc erteket a szamitashoz.
*/
extern unsigned int T0kezdoertek(float ido);


/*
  Beallitja a T0-t idozito 1 uzemmodba (leallitja
  a szamlalot)
*/
extern void T0idozito1(bool kapuzott);


/*
  Beallitja a T0-t szamlalo uzemmodba (leallitja
  a szamlalot.
*/
extern void T0szamlalo(bool kapuzott);


/*
  Ez a ket fuggveny a szamlalo aktualis erteket
  modositja. Egyforma a ketto, az egyik pl a foprogrambol,
  mig a masik a megszakitas kezelobol hivhato.
*/
extern void T0beallit(unsigned int kezdoertek);
extern void T0ujratolt(unsigned int kezdoertek);

#define T0set(x) { TH0=(x)>>8;TL0=(x)&0xff; }


/*
  A szamlalo elinditasahoz es leallitasahoz hasznalhato
  makrok
*/
#define T0start() TR0=1
#define T0stop() TR0=0


/*
  A szamlalo aktualis allapota es a tulcsordulasig hatralevo
  ido usec-ben
*/
extern float T0eltelt_us(void);
extern float T0hatravan_us(void);

/*
  A szamlalo aktualis allapota es a tulcsordulasig hatralevo
  ido msec-ben
*/
#define T0eltelt() (T0eltelt_us()/1000.0)
#define T0hatravan() (T0hatravan_us()/1000.0)


#endif

/* End of T0.h */
