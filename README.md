# CPCtelera

_**Astonishing fast Amstrad CPC game engine for C developers**_

---------------------------------------------------------------

_**CPCtelera**_ is a game engine for Amstrad CPC computers whose aim is to have the fastest possible 
routines to aid developers in the creation of games. The main aims of _**CPCtelera**_ are producing the fastest possible code, being very easy to install and use in every modern platform and having an extensively commented code, to aid those wanting to learn from code.

_**CPCtelera**_ is still under develpment, at a very early stage. However, it is completely usable at the moment.

**Installing and using** _**CPCtelera**_ is very easy:
 1. Open a terminal (under Windows, you will have to install [Cygwin](https://cygwin.com/) previously)
 2. Clone _**CPCtelera**_ repository
   * `git clone https://github.com/lronaldo/cpctelera`
 3. Enter _**CPCtelera**_ folder
   * `cd cpctelera/`
 4. Launch `setup.sh`
   * `sh setup.sh`

Follow setup instructions. Setup checks that required software is previously installed, and shows messages if some prerrequisite is missing. If that were the case, install required software and launch setup again. Currenly, _**CPCtelera**_ requires these software to be installed:
 * Compilers: `gcc`, `g++`
 * Parsers: `bison`, `flex`
 * Libraries: `libboost`

Once `setup.sh` completes without errors, _**CPCtelera**_ will be ready to use. Just enter `examples/` folder, go to the any one of them and type `make` to generate CDT and DSK files. You may use `examples/` as a starting point to build your own projects.

If you are a Mac user, you will want to consider installing [CPCtelera-samples-Xcode](https://github.com/dfreniche/cpctelera_samples_xcode) from [Diego Freniche](http://blog.freniche.com), along with CPCtelera. This package creates Xcode projects from examples and lets you build them directly from Xcode.

_**CPCtelera**_ is actually being tested under Windows (with Cygwin), OS X and Linux (Ubuntu/Debian, and Arch). It is almost fully usable. If you test it in any platform (listed here or not) and have problems, please feel free to report them to us. 

### Contact information and support

If you have any questions, please contact us:
 
 * email:    _cpctelera (at) cheesetea (dot) com_
 * twitter:  *[@FranGallegoBR](http://twitter.com/frangallegobr)*
 
### Authors

 * **Project owner** 
   * [ronaldo](http://twitter.com/frangallegobr) / ([Cheesetea](http://www.cheesetea.com), [Fremos](http://fremos.cheesetea.com),  [ByteRealms](http://www.byterealms.com))
 * **Awesome collaborators**
   * [Fran Fernández](https://twitter.com/ronsonmaria) / [Pensando como Pollos](http://www.pensandocomopollos.com/) ([@ronsonmaria](https://twitter.com/ronsonmaria))
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
