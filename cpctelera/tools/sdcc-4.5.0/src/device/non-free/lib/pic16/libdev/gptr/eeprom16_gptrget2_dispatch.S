	radix dec

	global	___eeprom_gptrget2
        extern  ___eeprom16_gptrget2

S___eeprom_gptrget2_stub    code
___eeprom_gptrget2:
        goto    ___eeprom16_gptrget2

	end
