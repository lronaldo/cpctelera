/*-----------------------------------------------------------------*/
/* SDCCmem.h - header file for memory management                    */
/*-----------------------------------------------------------------*/

#ifndef SDCCMEM_H
#define SDCCMEM_H

#include "dbuf.h"

struct set;
struct value;
struct eBBlock;

typedef struct memmap
  {
    unsigned char pageno;       /* page no for this variable   */
    const char *sname;          /* character prefix for map    */
    char dbName;                /* debugger address space name */
    int ptrType;                /* pointer Type for this space */
    int slbl;                   /* label counter for space     */
    unsigned sloc;              /* starting location           */
    unsigned fmap:1;            /* 1 = 16bit addressing reqd   */
    unsigned paged:1;           /* this is a paged mem space   */
    unsigned direct:1;          /* 1 = indirect access only    */
    unsigned bitsp:1;           /* 1 = bit addressable space   */
    unsigned codesp:1;          /* 1 = code space              */
    unsigned regsp:1;           /* 1 = sfr space               */
    struct dbuf_s oBuf;         /* object buffer associated    */
    struct set *syms;           /* symbols defined in this segment */
  }
memmap;

/* For performance we might want to use a hash map instead of the linked list */
typedef struct namedspacemap
  {
    char *name;
    bool is_const;
    memmap *map;
    struct namedspacemap *next;
  }
namedspacemap;

extern FILE *junkFile;

/* memory map prefixes  MOF added the DATA,CODE,XDATA,BIT */
#define  XSTACK_NAME       port->mem.xstack_name
#define  ISTACK_NAME       port->mem.istack_name
#define  CODE_NAME         port->mem.code_name
#define  DATA_NAME         port->mem.data_name
#define  INITIALIZED_NAME  port->mem.initialized_name
#define  INITIALIZER_NAME  port->mem.initializer_name
#define  IDATA_NAME        port->mem.idata_name
#define  PDATA_NAME        port->mem.pdata_name
#define  XDATA_NAME        port->mem.xdata_name
#define  XIDATA_NAME       port->mem.xidata_name
#define  XINIT_NAME        port->mem.xinit_name
#define  BIT_NAME          port->mem.bit_name
#define  REG_NAME          port->mem.reg_name
#define  STATIC_NAME       port->mem.static_name
#define  HOME_NAME         port->mem.home_name
#define  OVERLAY_NAME      port->mem.overlay_name
#define  CONST_NAME        port->mem.const_name
#define  CABS_NAME         port->mem.cabs_name
#define  XABS_NAME         port->mem.xabs_name
#define  IABS_NAME         port->mem.iabs_name

/* forward definition for variables */
extern memmap *xstack;                 /* xternal stack data           */
extern memmap *istack;                 /* internal stack               */
extern memmap *code;                   /* code segment                 */
extern memmap *data;                   /* internal data upto 128       */
extern memmap *initialized;            /* initialized data, such as initalized, nonzero globals or local statics. */
extern memmap *initializer;            /* a copy of the values for the initalized data from initialized in code space */
extern memmap *pdata;                  /* paged external data upto 256 */
extern memmap *xdata;                  /* external data                */
extern memmap *xidata;                 /* the initialized xdata        */
extern memmap *xinit;                  /* the initializers for xidata  */
extern memmap *idata;                  /* internal data upto 256       */
extern memmap *bit;                    /* bit addressable space        */
extern memmap *statsg;                 /* static code segment          */
extern memmap *c_abs;                  /* constant absolute data       */
extern memmap *x_abs;                  /* absolute xdata/pdata         */
extern memmap *i_abs;                  /* absolute idata upto 256      */
extern memmap *d_abs;                  /* absolute data upto 128       */
extern memmap *sfr;                    /* register space               */
extern memmap *sfrbit;                 /* sfr bit space                */
extern memmap *reg;                    /* register space               */
extern memmap *generic;                /* unknown                      */
extern memmap *overlay;                /* the overlay segment          */
extern memmap *eeprom;                 /* eeprom space                 */
extern memmap *home;                   /* Non-banked home space        */

extern namedspacemap *namedspacemaps;

extern int fatalError;

extern struct set *ovrSetSets;

/* easy access macros */
#define IN_BITSPACE(map)        (map && map->bitsp)
#define IN_STACK(x)  (IS_SPEC(x) && (SPEC_OCLS(x) == xstack || SPEC_OCLS(x) == istack ))
#define IN_FARSPACE(map)        (map && map->fmap)
#define IN_DIRSPACE(map)        (map && map->direct)
#define IN_PAGEDSPACE(map)      (map && map->paged )
#define IN_CODESPACE(map)       (map && map->codesp)
#define IN_REGSP(map)           (map && map->regsp)
#define PTR_TYPE(map)           (map ? (map->ptrType ? map->ptrType : POINTER)\
                                     : port->unqualified_pointer)

/* forward decls for functions    */
memmap *allocMap (char, char, char, char, char, char, unsigned, const char *, char, int);
void initMem ();
bool defaultOClass (struct symbol *);
void allocGlobal (struct symbol *);
void allocLocal (struct symbol *);
void allocParms (struct value *, bool smallc);
void deallocParms (struct value *);
void deallocLocal (struct symbol *);
int allocVariables (struct symbol *);
void overlay2Set ();
void overlay2data ();
void redoStackOffsets ();
void printAllocInfo (struct symbol *, struct dbuf_s *);
void doOverlays (struct eBBlock **, int count);
void deleteFromSeg(struct symbol *);
#endif

