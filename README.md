# CPCtelera
Astonishing fast Amstrad CPC game engine for C developers
======================================================================
CPCtelera is a game engine for Amstrad CPC computers whose aim is to have the fastest possible 
routines to aid developers in the creation of games. 

CPCtelera is still under develpment, at a very early stage. Although it is possible to use CPCtelera 
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
 
That version of cpc-dev-tool-chain will work nicely on almost any linux machine, and may also work on Windows
with a 32-bit version of Cygwin. If you plan to use it this way (Windows+Cygwin 32), check out 
[Profesor Retroman's tutorial (ES)](https://www.youtube.com/watch?v=FvAg-xmWZHM). 

Current testing makefiles for examples and library work with this directory structure:

    /
    --/cpctelera
    --/cpc-dev-tool-chain

Where `/cpc-dev-tool-chain` has SDCC, Hex2Bin and iDSK already compiled 
(see [cpcitor documentation](https://github.com/cpcitor/cpc-dev-tool-chain/blob/master/README.md) for more details)

If you have any questions, please contact me:
  email:    ronaldo (at) cheesetea (dot) com
  twitter:  @FranGallegoBR