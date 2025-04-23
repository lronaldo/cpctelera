EXEEXT		= @EXEEXT@

enable_mcs51_sim   = @enable_mcs51_sim@
enable_avr_sim     = @enable_avr_sim@
enable_z80_sim     = @enable_z80_sim@
enable_tlcs_sim    = @enable_tlcs_sim@
enable_xa_sim      = @enable_xa_sim@
enable_m68hc08_sim = @enable_m68hc08_sim@
enable_stm8_sim    = @enable_stm8_sim@
enable_st7_sim     = @enable_st7_sim@
enable_pdk_sim     = @enable_pdk_sim@
enable_p1516_sim   = @enable_p1516_sim@
enable_m6809_sim   = @enable_m6809_sim@
enable_m6800_sim   = @enable_m6800_sim@
enable_m68hc11_sim = @enable_m68hc11_sim@
enable_m68hc12_sim = @enable_m68hc12_sim@
enable_mos6502_sim = @enable_mos6502_sim@
enable_rxk_sim	    = @enable_rxk_sim@
enable_pblaze_sim  = @enable_pblaze_sim@
enable_i8085_sim   = @enable_i8085_sim@
enable_f8_sim      = @enable_f8_sim@
enable_i8048_sim   = @enable_i8048_sim@
enable_oisc_sim    = @enable_oisc_sim@

enable_serio        = @enable_serio@
enable_feapp        = @enable_feapp@

ifeq ($(enable_mcs51_sim),yes)
S51		= s51.src
else
S51		=
endif

ifeq ($(enable_avr_sim),yes)
SAVR		= avr.src
else
SAVR		=
endif

ifeq ($(enable_z80_sim),yes)
SZ80		= z80.src
else
SZ80		=
endif

ifeq ($(enable_tlcs_sim),yes)
TLCS		= tlcs.src
else
TLCS		=
endif

ifeq ($(enable_m68hc08_sim),yes)
M68HC08		= m68hc08.src
else
M68HC08		=
endif

ifeq ($(enable_xa_sim),yes)
XA		= xa.src
else
XA		=
endif

ifeq ($(enable_stm8_sim),yes)
STM8		= stm8.src
else
STM8		=
endif

ifeq ($(enable_st7_sim),yes)
ST7      	= st7.src
else
ST7      =
endif

ifeq ($(enable_pdk_sim),yes)
PDK    = pdk.src
else
PDKD   =
endif

ifeq ($(enable_p1516_sim),yes)
P1516      	= p1516.src
else
P1516      =
endif

ifeq ($(enable_m6809_sim),yes)
M6809      	= m6809.src
else
M6809      =
endif

ifeq ($(enable_m6800_sim),yes)
M6800      	= m6800.src
else
M6800      =
endif

ifeq ($(enable_m68hc11_sim),yes)
M68HC11      	= m68hc11.src
else
M68HC11    =
endif

ifeq ($(enable_m68hc12_sim),yes)
M68HC12      	= m68hc12.src
else
M68HC12    =
endif

ifeq ($(enable_mos6502_sim),yes)
MOS6502      	= mos6502.src
else
MOS6502      =
endif

ifeq ($(enable_rxk_sim),yes)
RXK      	= rxk.src
else
RXK      =
endif

ifeq ($(enable_pblaze_sim),yes)
PBLAZE      	= pblaze.src
else
PBLAZE	=
endif

ifeq ($(enable_i8085_sim),yes)
I8085      	= i8085.src
else
I8085	=
endif

ifeq ($(enable_f8_sim),yes)
F8      	= f8.src
else
F8		=
endif

ifeq ($(enable_i8048_sim),yes)
I8048      	= i8048.src
else
I8048		=
endif

ifeq ($(enable_oisc_sim),yes)
OISC      	= oisc.src
else
OISC		=
endif

PKGS		= $(P1516) $(F8) $(OISC) \
		  $(S51) $(I8048) $(I8085) $(XA) \
		  $(SAVR) \
		  $(SZ80) $(TLCS) $(RXK) \
		  $(STM8) $(ST7)  \
		  $(M6800) $(M68HC08) $(M6809) $(M68HC11) $(M68HC12) \
		  $(MOS6502) \
		  $(PBLAZE) \
		  $(PDK)

PKGS_ALL	= s51.src avr.src z80.src tlcs.src xa.src \
		  m68hc08.src stm8.src st7.src pdk.src p1516.src \
		  m6809.src m6800.src m68hc11.src m68hc12.src mos6502.src \
		  rxk.src pblaze.src i8085.src f8.src i8048.src oisc.src

curses_ok	= @curses_ok@

ifeq ($(enable_serio),yes)
ifeq ($(curses_ok),yes)
SERIO		= serio.src
else
SERIO		=
endif
else
SERIO		=
endif

ifeq ($(enable_feapp),yes)
UCSIM		= ucsim.src
else
UCSIM		=
endif

APPS		= $(SERIO) $(UCSIM)

APPS_ALL	= serio.src ucsim.src
