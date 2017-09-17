MAIN		= stm8s_disc_serial

OTHERS		= serial

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_STM8S105

