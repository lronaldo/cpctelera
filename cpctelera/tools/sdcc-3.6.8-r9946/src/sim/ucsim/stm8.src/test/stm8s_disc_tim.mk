MAIN		= stm8s_disc_tim

OTHERS		=

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_SDISC
