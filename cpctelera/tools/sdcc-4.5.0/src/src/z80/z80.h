/** @file z80/z80.h
    Common definitions for the the z80-related ports.
*/
#include "common.h"
#include "ralloc.h"
#include "gen.h"
#include "peep.h"
#include "support.h"

typedef enum
  {
    SUB_Z80,
    SUB_Z180,
    SUB_R2K,
    SUB_R2KA,
    SUB_R3KA,
    SUB_SM83,
    SUB_TLCS90,
    SUB_EZ80_Z80,
    SUB_Z80N,
    SUB_R800
  }
Z80_SUB_PORT;

typedef struct
  {
    Z80_SUB_PORT sub;
    int calleeSavesBC;
    int port_mode;
    int port_back;
    int reserveIY;
    int noOmitFramePtr;
    int legacyBanking;
    int nmosZ80;
  }
Z80_OPTS;

extern Z80_OPTS z80_opts;

#define IS_Z80 (z80_opts.sub == SUB_Z80)
#define IS_Z180 (z80_opts.sub == SUB_Z180)
#define IS_R2K (z80_opts.sub == SUB_R2K)
#define IS_R2KA (z80_opts.sub == SUB_R2KA)
#define IS_R3KA (z80_opts.sub == SUB_R3KA)
#define IS_RAB (IS_R2K || IS_R2KA || IS_R3KA)
#define IS_SM83 (z80_opts.sub == SUB_SM83)
#define IS_TLCS90 (z80_opts.sub == SUB_TLCS90)
#define IS_EZ80_Z80 (z80_opts.sub == SUB_EZ80_Z80)
#define IS_Z80N (z80_opts.sub == SUB_Z80N)
#define IS_R800 (z80_opts.sub == SUB_R800)
#define HAS_IYL_INST (IS_Z80N || IS_EZ80_Z80 || IS_R800 || IS_Z80 && options.allow_undoc_inst)

#define IY_RESERVED (z80_opts.reserveIY)

#define OPTRALLOC_IY !(IY_RESERVED || IS_SM83)

enum
  {
    ACCUSE_A = 1,
    ACCUSE_SCRATCH,
    ACCUSE_IY
  };

