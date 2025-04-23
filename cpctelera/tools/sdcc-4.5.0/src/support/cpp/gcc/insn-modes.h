#ifndef GCC_INSN_MODES_H
#define GCC_INSN_MODES_H

// sdcpp hacks
#define BITS_PER_UNIT 8
#define MAX_BITSIZE_MODE_ANY_INT (64*BITS_PER_UNIT)
#define NUM_INT_N_ENTS 1


enum machine_mode
{
  E_VOIDmode,              /* machmode.def:193 */
#define HAVE_VOIDmode
#ifdef USE_ENUM_MODES
#define VOIDmode E_VOIDmode
#else
#define VOIDmode ((void) 0, E_VOIDmode)
#endif
  E_BLKmode,               /* machmode.def:197 */
#define HAVE_BLKmode
#ifdef USE_ENUM_MODES
#define BLKmode E_BLKmode
#else
#define BLKmode ((void) 0, E_BLKmode)
#endif
  MAX_MACHINE_MODE,
  E_HFmode,                /* config/i386/i386-modes.def:26 */
  E_TFmode,                /* config/i386/i386-modes.def:25 */
  E_SDmode,                /* machmode.def:271 */
  E_TDmode,                /* machmode.def:273 */
  MIN_MODE_DECIMAL_FLOAT = E_SDmode,
  MAX_MODE_DECIMAL_FLOAT = E_TDmode,
  MIN_MODE_FLOAT = E_HFmode,
  MAX_MODE_FLOAT = E_TFmode,
  NUM_MACHINE_MODES = MAX_MACHINE_MODE
};

// sdcpp insn-modes ..
#define NUM_MODE_FLOAT (MAX_MODE_FLOAT - MIN_MODE_FLOAT + 1)
#define NUM_MODE_DECIMAL_FLOAT (MAX_MODE_DECIMAL_FLOAT - MIN_MODE_DECIMAL_FLOAT + 1)


#endif
