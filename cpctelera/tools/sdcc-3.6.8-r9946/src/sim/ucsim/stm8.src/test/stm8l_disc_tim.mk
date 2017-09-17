MAIN		= stm8l_disc_tim

OTHERS		=

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_STM8L15x46
