MAIN		= stm8s_disc_led

OTHERS		=

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_SDISC
