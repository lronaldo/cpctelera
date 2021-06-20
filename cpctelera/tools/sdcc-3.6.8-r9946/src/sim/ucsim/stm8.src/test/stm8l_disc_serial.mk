MAIN		= stm8l_disc_serial

OTHERS		=

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_STM8L15x46
