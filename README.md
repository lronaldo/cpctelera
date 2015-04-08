# CPCtelera

_**Astonishing fast Amstrad CPC game engine for C developers**_

---------------------------------------------------------------

_**CPCtelera**_ is a game engine for Amstrad CPC computers whose aim is to have the fastest possible 
routines to aid developers in the creation of games. 

_**CPCtelera**_ is still under develpment, at a very early stage. Although it is possible to use _**CPCtelera**_ 
at its present status and do cool things, it will suffer great modifications until 1.0. version is
released. Moreover, if you wanted to test and use it, you will need these tools:
 
 * SDCC 3.4 or greater compiler
 * Hex2Bin 1.0.10 or greater
 * iDSK 0.13 or greater

For having all this tools i personally recommend using 
[cpc-dev-tool-chain by cpcitor](https://github.com/cpcitor/cpc-dev-tool-chain/), as it integrates all of them
(and more) in a nice and easy to use fashion. For testing purposes, it is enough to use commit 48ca1dd9a1 so,
if you want to use them do this:
 
     git clone https://github.com/cpcitor/cpc-dev-tool-chain.git
     git reset --hard 48ca1dd9a1
 
That version of cpc-dev-tool-chain will work nicely on almost any Linux machine, and may also work on Windows
with a 32-bit version of Cygwin. If you plan to use it this way (Windows+Cygwin 32), check out 
[Profesor Retroman's tutorial (ES)](https://www.youtube.com/watch?v=FvAg-xmWZHM). 

Current testing makefiles for examples and library work with this directory structure:

    /
    --/cpctelera
    --/cpc-dev-tool-chain

where `/cpc-dev-tool-chain` has SDCC, Hex2Bin and iDSK already compiled 
(see [cpcitor documentation](https://github.com/cpcitor/cpc-dev-tool-chain/blob/master/README.md) for more details)

### Contact information and support

If you have any questions, please contact me:
 
 * email:    _ronaldo (at) cheesetea (dot) com_
 * twitter:  *[@FranGallegoBR](http://twitter.com/frangallegobr)*
 
### Authors

 * **Project owner** 
   * [ronaldo](http://twitter.com/frangallegobr) / ([Cheesetea](http://www.cheesetea.com), [Fremos](http://fremos.cheesetea.com),  [ByteRealms](http://www.byterealms.com))
 * **Awesome collaborators**
   * [Fran Fernández](https://twitter.com/ronsonmaria) / [Pensando como Pollos](http://www.pensandocomopollos.com/)
   * [Diego Freniche](http://blog.freniche.com) ([@dfreniche](https://twitter.com/dfreniche))
 * **Bitarrays idea and original implementation**
   * Alberto García García (Blitzman, _albertgg93 (at) gmail (dot) com, [@algertgarcia93](http://twitter.com/algertgarcia93)_)
   * Pablo Martínez González (*pablo2093 (at) gmail (dot) com, [@pablo_2093](http://twitter.com/pablo_2093)*)
 * **Some original code and ideas for sprite drawing and firmware enabling/disabling**
   * Raúl Simarro (Artaburu / ESP Soft)
 * **Some makefile code and ideas**
   * [Cpcitor](http://github.com/cpcitor)
 * **Arkos player original code**
   * [Targhan](http://www.julien-nevo.com/) / [Arkos](http://www.cpcwiki.eu/index.php/Arkos)

### 3rdparty software authors
 
 * **[Arkos Tracker](http://www.grimware.org/doku.php/documentations/software/arkos.tracker/start)**
   * [Targhan](http://www.julien-nevo.com/) / [Arkos](http://www.cpcwiki.eu/index.php/Arkos)
 * **[Small Devices C Compiler (SDCC)](http://sdcc.sourceforge.net/)**
   * [SDCC Team](http://sdcc.sourceforge.net/index.php#Who)
 * **iDSK**
   * Sid / IMPACT 
   * [Pulkomandy](http://www.pushnpop.net/profile-47.html) / [Shinra Team](http://pulko.mandy.pagesperso-orange.fr/shinra/index.shtml)
 * **Hex2bin**
   * [Jaques Pelletier](https://www.linkedin.com/pub/jacques-pelletier/a/668/309)
 * **2CDT**
   * [Kevin Thacker](http://www.cpctech.org.uk/about.html) / [CPCTech](http://www.cpctech.org.uk/) 

#### Special Thanks

 * [Cpcitor](http://github.com/cpcitor) (for his great help and advice, and for developing [cpc-dev-tool-chain](http://github.com/cpcitor/cpc-dev-tool-chain))
 * [CPCWiki](http://cpcwiki.eu) & [Grimware](http://grimware.org/) (for providing great technical documentations and examples)
 * Raúl Simarro (for developing [CPCRSLib](http://sourceforge.net/projects/cpcrslib/) game creation library)
 * [Targhan](http://www.julien-nevo.com/) / [Arkos](http://www.cpcwiki.eu/index.php/Arkos) (for developing Arkos Traker&Player and giving great support on adding it to CPCtelera. Also for giving general advice during development of CPCtelera)
 * [Pulkomandy](http://www.pushnpop.net/profile-47.html) / [Shinra Team](http://pulko.mandy.pagesperso-orange.fr/shinra/index.shtml) (for maintaining [cpcsdk](https://github.com/cpcsdk), with tools like [iDSK](https://code.google.com/p/cpcsdk/downloads/detail?name=iDSK_015.7z&can=2&q=) or [cpcfs](https://code.google.com/p/cpcsdk/wiki/cpcfs), among others. For giving great advice and being helpful at every moment. Also for willing to give the best for CPC and maintain it alive)
