	radix dec

	global	___eeprom_gptrput3
        extern  ___eeprom16_gptrput3

S___eeprom_gptrput3_stub    code
___eeprom_gptrput3:
        goto    ___eeprom16_gptrput3

	end
