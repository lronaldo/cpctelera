	radix dec

	global	___eeprom_gptrput1
        extern  ___eeprom8_gptrput1

S___eeprom_gptrput1_stub    code
___eeprom_gptrput1:
        goto    ___eeprom8_gptrput1

	end
