
MAIN		= banking

OTHERS		= bank0 bank1

include sdcc.mk

TARGET		= -mmcs51

LDFLAGS		= -Wl-bBANK0=0x18000 -Wl-bBANK1=0x28000 --code-size 0x1000000 -Wl-r
