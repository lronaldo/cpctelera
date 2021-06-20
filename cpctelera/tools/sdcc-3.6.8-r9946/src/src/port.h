/** @file port.h
    Definitions for what a port must provide.
    All ports are referenced in SDCCmain.c.
 */
#ifndef PORT_INCLUDE
#define PORT_INCLUDE

#include "SDCCicode.h"
#include "SDCCargs.h"
#include "SDCCpeeph.h"
#include "dbuf.h"

#define TARGET_ID_MCS51    1
#define TARGET_ID_GBZ80    2
#define TARGET_ID_Z80      3
#define TARGET_ID_AVR      4
#define TARGET_ID_DS390    5
#define TARGET_ID_PIC14    6
#define TARGET_ID_PIC16    7
#define TARGET_ID_DS400    10
#define TARGET_ID_HC08     11
#define TARGET_ID_Z180     12
#define TARGET_ID_R2K      13
#define TARGET_ID_R3KA     14
#define TARGET_ID_S08      15
#define TARGET_ID_STM8     16
#define TARGET_ID_TLCS90   17

/* Macro to test the target we are compiling for.
   Can only be used after SDCCmain has defined the port
 */
#define TARGET_IS_MCS51    (port->id == TARGET_ID_MCS51)
#define TARGET_IS_AVR      (port->id == TARGET_ID_AVR)
#define TARGET_IS_DS390    (port->id == TARGET_ID_DS390)
#define TARGET_IS_DS400    (port->id == TARGET_ID_DS400)
#define TARGET_IS_PIC14    (port->id == TARGET_ID_PIC14)
#define TARGET_IS_PIC16    (port->id == TARGET_ID_PIC16)
#define TARGET_IS_Z80      (port->id == TARGET_ID_Z80)
#define TARGET_IS_Z180     (port->id == TARGET_ID_Z180)
#define TARGET_IS_R2K      (port->id == TARGET_ID_R2K)
#define TARGET_IS_R3KA     (port->id == TARGET_ID_R3KA)
#define TARGET_IS_GBZ80    (port->id == TARGET_ID_GBZ80)
#define TARGET_IS_TLCS90   (port->id == TARGET_ID_TLCS90)
#define TARGET_IS_HC08     (port->id == TARGET_ID_HC08)
#define TARGET_IS_S08      (port->id == TARGET_ID_S08)
#define TARGET_IS_STM8     (port->id == TARGET_ID_STM8)

#define TARGET_MCS51_LIKE  (TARGET_IS_MCS51 || TARGET_IS_DS390 || TARGET_IS_DS400)
#define TARGET_Z80_LIKE    (TARGET_IS_Z80 || TARGET_IS_Z180 || TARGET_IS_GBZ80 || TARGET_IS_R2K || TARGET_IS_R3KA || TARGET_IS_TLCS90)
#define TARGET_IS_RABBIT   (TARGET_IS_R2K || TARGET_IS_R3KA)
#define TARGET_HC08_LIKE   (TARGET_IS_HC08 || TARGET_IS_S08)
#define TARGET_PIC_LIKE    (TARGET_IS_PIC14 || TARGET_IS_PIC16)
/* is using sdas / sdld assembler / linker */
#define IS_SDASLD          (TARGET_Z80_LIKE || TARGET_MCS51_LIKE || TARGET_HC08_LIKE)

#define MAX_BUILTIN_ARGS        16
/* definition of builtin functions */
typedef struct builtins
{
  char *name;                   /* name of builtin function */
  char *rtype;                  /* return type as string : see typeFromStr */
  int nParms;                   /* number of parms : max 8 */
  char *parm_types[MAX_BUILTIN_ARGS];   /* each parm type as string : see typeFromStr */
} builtins;

struct ebbIndex;

/* pragma structure */
struct pragma_s
{
  const char *name;
  int id;
  char deprecated;
  int (*func) (int id, const char *name, const char *cp);
};

/* defined in SDCClex.lex */
int process_pragma_tbl (const struct pragma_s *pragma_tbl, const char *s);

/* Processor specific names */
typedef struct
{
  /** Unique id for this target */
  const int id;
  /** Target name used for -m */
  const char *const target;

  /** Target name string, used for --help */
  const char *const target_name;

  /** Specific processor for the given target family. specified by -p */
  char *processor;

  struct
  {
    /** Pointer to glue function */
    void (*do_glue) (void);
    /** TRUE if all types of glue functions should be inserted into
        the file that also defines main.
        We dont want this in cases like the z80 where the startup
        code is provided by a seperate module.
     */
    bool glue_up_main;
    /* OR of MODEL_* */
    int supported_models;
    int default_model;
    /** return the model string, used as library destination;
        port->taget is used as model string if get_model is NULL */
    const char *(*get_model) (void);
  }
  general;

  /* assembler related information */
  struct
  {
    /** Command to run and arguments (eg as-z80) */
    const char **cmd;
    /** Alternate macro based form. */
    const char *mcmd;
    /** Arguments for debug mode. */
    const char *debug_opts;
    /** Arguments for normal assembly mode. */
    const char *plain_opts;
    /* print externs as global */
    int externGlobal;
    /* assembler file extension */
    const char *file_ext;
    /** If non-null will be used to execute the assembler. */
    void (*do_assemble) (set *);
  }
  assembler;

  /* linker related info */
  struct
  {
    /** Command to run (eg link-z80) */
    const char **cmd;
    /** Alternate macro based form. */
    const char *mcmd;
    /** If non-null will be used to execute the link. */
    void (*do_link) (void);
    /** Extension for object files (.rel, .obj, ...) */
    const char *rel_ext;
    /** 1 if port needs the .lnk file, 0 otherwise */
    const int needLinkerScript;
    const char *const *crt;
    const char *const *libs;
  }
  linker;

  /** Default peephole rules */
  struct
  {
    char *default_rules;
    int (*getSize) (lineNode * line);
    bitVect *(*getRegsRead) (lineNode * line);
    bitVect *(*getRegsWritten) (lineNode * line);
    bool (*deadMove) (const char *reg, lineNode * currPl, lineNode * head);
    bool (*notUsed) (const char *reg, lineNode * currPl, lineNode * head);
    bool (*canAssign) (const char *op1, const char *op2, const char *op3);
    bool (*notUsedFrom) (const char *reg, const char *label, lineNode *head);
    bool (*symmParmStack) (void);
  }
  peep;

  /** Basic type sizes */
  struct
  {
    int char_size;
    int short_size;
    unsigned int int_size;
    int long_size;
    int longlong_size;
    int ptr_size;               //near
    int fptr_size;              //far
    int gptr_size;              //generic
    int bit_size;
    int float_size;
    int max_base_size;
  }
  s;

  /** tags for far, near, xstack, code generic pointers */
  struct
  {
    int tag_far;
    int tag_near;
    int tag_xstack;
    int tag_code;
  }
  gp_tags;

  /** memory regions related stuff */
  struct
  {
    const char *const xstack_name;
    const char *const istack_name;
    /*
     * The following 2 items can't be const pointers
     * due to ugly implementation in gbz80 target;
     * this should be fixed in src/z80/main.c (borutr)
     */
    const char *code_name;
    const char *data_name;
    const char *const idata_name;
    const char *const pdata_name;
    const char *const xdata_name;
    const char *const bit_name;
    const char *const reg_name;
    const char *const static_name;
    const char *const overlay_name;
    const char *const post_static_name;
    const char *const home_name;
    const char *const xidata_name;      // initialized xdata
    const char *const xinit_name;       // a code copy of xidata
    const char *const const_name;       // const data (code or not)
    const char *const cabs_name;        // const absolute data (code or not)
    const char *const xabs_name;        // absolute xdata/pdata
    const char *const iabs_name;        // absolute idata/data
    const char *const initialized_name; // Initialized global (and static local) variables.
    const char *const initializer_name; // A code copy of initialized_name (to be copied for fast initialization).
    struct memmap *default_local_map;   // default location for auto vars
    struct memmap *default_globl_map;   // default location for globl vars
    int code_ro;                        // code space read-only 1=yes
    unsigned int maxextalign;           // maximum extended alignment supported, nonnegative power of 2 (C11 standard, section 6.2.8).
  }
  mem;

  struct
  {
    void (*genExtraAreaDeclaration) (FILE *, bool);
    void (*genExtraAreaLinkOptions) (FILE *);
  }
  extraAreas;

  /* stack related information */
  struct
  {
    /** -1 for grows down (z80), +1 for grows up (mcs51) */
    int direction;
    /** Extra overhead when calling between banks */
    int bank_overhead;
    /** Extra overhead when the function is an ISR */
    int isr_overhead;
    /** Standard overhead for a function call */
    int call_overhead;
    /** Re-enterant space */
    int reent_overhead;
    /** 'banked' call overhead.
        Mild overlap with bank_overhead */
    int banked_overhead;
    /** 0 if sp points to last item pushed, 1 if sp points to next location to use */
    int offset;

  }
  stack;

  struct
  {
    /** Size of the biggest shift the port can handle. -1 if port can handle shifts of arbitrary size. */
    signed int shift;

    /* Has support routines for int x int -> long multiplication and unsigned int x unsigned int -> unsigned long multiplication */
    bool has_mulint2long;
  }
  support;

  struct
  {
    void (*emitDebuggerSymbol) (const char *);
    struct
    {
      int (*regNum) (const struct reg_info *);
      bitVect *cfiSame;
      bitVect *cfiUndef;
      int addressSize;
      int regNumRet;
      int regNumSP;
      int regNumBP;
      int offsetSP;
    }
    dwarf;
  }
  debugger;

  struct
  {
    int maxCount;
    int sizeofElement;
    int sizeofMatchJump[3];
    int sizeofRangeCompare[3];
    int sizeofSubtract;
    int sizeofDispatch;
  }
  jumptableCost;

  /** Prefix to add to a C function (eg "_") */
  const char *fun_prefix;

  /** Called once the processor target has been selected.
      First chance to initalise and set any port specific variables.
      'port' is set before calling this.  May be NULL.
  */
  void (*init) (void);
  /** Parses one option + its arguments */
  bool (*parseOption) (int *pargc, char **argv, int *i);
  /** Optional list of automatically parsed options.  Should be
      implemented to at least show the help text correctly. */
  OPTION *poptions;
  /** Initialise port spectific paths */
  void (*initPaths) (void);
  /** Called after all the options have been parsed. */
  void (*finaliseOptions) (void);
   /** Called after the port has been selected but before any
       options are parsed. */
  void (*setDefaultOptions) (void);
  /** Does the dirty work. */
  void (*assignRegisters) (struct ebbIndex *);

  /** Returns the register name of a symbol.
      Used so that 'reg_info' can be an incomplete type. */
  const char *(*getRegName) (const struct reg_info *reg);

  int (*getRegByName) (const char *name);

  /** Try to keep track of register contents. */
  bool (*rtrackUpdate)(const char* line);

  /* list of keywords that are used by this
     target (used by lexer) */
  char **keywords;

  /* Write any port specific assembler output. */
  void (*genAssemblerPreamble) (FILE * of);
  /* invoked at end assembler file */
  void (*genAssemblerEnd) (FILE * of);

  /* Write the port specific IVT. If genIVT is NULL or if
   * it returns zero, default (8051) IVT generation code
   * will be used.
   */
  int (*genIVT) (struct dbuf_s * oBuf, symbol ** intTable, int intCount);

  void (*genXINIT) (FILE * of);

  /* Write port specific startup code */
  void (*genInitStartup) (FILE * of);

  /* parameter passing in register related functions */
  void (*reset_regparms) (struct sym_link *);        /* reset the register count */
  int (*reg_parm) (struct sym_link *, bool reentrant);  /* will return 1 if can be passed in register */

  /** Process the pragma string 'sz'.  Returns 0 if recognised and
      processed, 1 otherwise.  May be NULL.
   */
  int (*process_pragma) (const char *sz);

  /** Mangles a support function name to reflect the calling model.
   */
  const char *(*getMangledFunctionName) (const char *szOrginial);

  /** Returns true if the port can multiply the two types nativly
      without using support functions.
   */
  bool (*hasNativeMulFor) (iCode *ic, sym_link *left, sym_link *right);

  /** Returns true if the port has implemented certain bit
      manipulation iCodes (RRC, RLC, SWAP, GETHBIT, GETABIT, GETBYTE, GETWORD)
   */
  bool (*hasExtBitOp) (int op, int size);

  /** Returns the relative expense of accessing a particular output
      storage class. Larger values indicate higher expense.
   */
  int (*oclsExpense) (struct memmap * oclass);

  /** If TRUE, then tprintf and !dw will be used for some initalisers
   */
  bool use_dw_for_init;

  /** TRUE for targets with little endian byte ordering, FALSE for
      targets with big endian byte ordering.
   */
  bool little_endian;

  /* condition transformations */
  bool lt_nge;                  /* transform (a < b)  to !(a >= b)  */
  bool gt_nle;                  /* transform (a > b)  to !(a <= b)  */
  bool le_ngt;                  /* transform (a <= b) to !(a > b)   */
  bool ge_nlt;                  /* transform (a >= b) to !(a < b)   */
  bool ne_neq;                  /* transform a != b --> ! (a == b)  */
  bool eq_nne;                  /* transform a == b --> ! (a != b)  */

  bool arrayInitializerSuppported;
  bool (*cseOk) (iCode * ic, iCode * pdic);
  builtins *builtintable;       /* table of builtin functions */
  int unqualified_pointer;      /* unqualified pointers type is  */
  int reset_labelKey;           /* reset Label no 1 at the start of a function */
  int globals_allowed;          /* global & static locals not allowed ?  0 ONLY TININative */

  int num_regs;                 /* Number of registers handled in the tree-decomposition-based register allocator in SDCCralloc.hpp */

#define PORT_MAGIC 0xAC32
  /** Used at runtime to detect if this structure has been completely filled in. */
  int magic;
}
PORT;

extern PORT *port;

#if !OPT_DISABLE_MCS51
extern PORT mcs51_port;
#endif
#if !OPT_DISABLE_Z80
extern PORT z80_port;
#endif
#if !OPT_DISABLE_Z180
extern PORT z180_port;
#endif
#if !OPT_DISABLE_R2K
extern PORT r2k_port;  /* Rabbit 2000/3000 */
#endif
#if !OPT_DISABLE_R3KA
extern PORT r3ka_port; /* Rabbit 3000A */
#endif
#if !OPT_DISABLE_GBZ80
extern PORT gbz80_port;
#endif
#if !OPT_DISABLE_TLCS90
extern PORT tlcs90_port;
#endif
#if !OPT_DISABLE_AVR
extern PORT avr_port;
#endif
#if !OPT_DISABLE_DS390
extern PORT ds390_port;
#endif
#if !OPT_DISABLE_PIC14
extern PORT pic_port;
#endif
#if !OPT_DISABLE_PIC16
extern PORT pic16_port;
#endif
#if !OPT_DISABLE_TININative
extern PORT tininative_port;
#endif
#if !OPT_DISABLE_DS400
extern PORT ds400_port;
#endif
#if !OPT_DISABLE_HC08
extern PORT hc08_port;
#endif
#if !OPT_DISABLE_S08
extern PORT s08_port;
#endif
#if !OPT_DISABLE_STM8
extern PORT stm8_port;
#endif

#endif /* PORT_INCLUDE */
