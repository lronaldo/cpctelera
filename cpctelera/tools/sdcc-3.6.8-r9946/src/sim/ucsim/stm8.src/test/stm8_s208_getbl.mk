APP		= stm8_s208_getbl

MAIN		= stm8s_disc_getbl

OTHERS		= serial

include sdcc.mk

CPPFLAGS	= -DDEVICE=DEV_STM8S208
