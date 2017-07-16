MAIN		= stm8l_disc_monitor

OTHERS		= serial

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_STM8L15x46
