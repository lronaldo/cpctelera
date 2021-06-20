	radix dec

	global	___eeprom_gptrput2
        extern  ___eeprom16_gptrput2

S___eeprom_gptrput2_stub    code
___eeprom_gptrput2:
        goto    ___eeprom16_gptrput2

	end
