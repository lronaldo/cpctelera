/* Generated automatically from machmode.def and config/i386/i386-modes.def
   by genmodes.  */

#include "bconfig.h"
#include "system.h"
#include "coretypes.h"

const char *const mode_name[NUM_MACHINE_MODES] =
{
  "VOID",
  "BLK",
//   "CC",
//   "CCGC",
//   "CCGOC",
};

const unsigned char mode_class[NUM_MACHINE_MODES] =
{
  MODE_RANDOM,             /* VOID */
  MODE_RANDOM,             /* BLK */
};

#if 0 // sdcpp
const poly_uint16_pod mode_nunits[NUM_MACHINE_MODES] = 
{
  { 0 },                   /* VOID */
  { 0 },                   /* BLK */
};
#endif

const unsigned char mode_wider[NUM_MACHINE_MODES] =
{
  E_VOIDmode,              /* VOID */
  E_VOIDmode,              /* BLK */
};

const unsigned char mode_2xwider[NUM_MACHINE_MODES] =
{
  E_VOIDmode,              /* VOID */
  E_BLKmode,               /* BLK */
};

const unsigned char mode_inner[NUM_MACHINE_MODES] =
{
  E_VOIDmode,              /* VOID */
  E_BLKmode,               /* BLK */
};

const unsigned char class_narrowest_mode[MAX_MODE_CLASS] =
{
  MODE_RANDOM,         /* VOID */
  MODE_CC,             /* CC */
};
