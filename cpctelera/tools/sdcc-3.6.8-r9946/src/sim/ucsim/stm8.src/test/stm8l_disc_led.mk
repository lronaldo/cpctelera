MAIN		= stm8l_disc_led

OTHERS		=

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_STM8L15x46
