	radix dec

	global	___eeprom_gptrget3
        extern  ___eeprom16_gptrget3

S___eeprom_gptrget3_stub    code
___eeprom_gptrget3:
        goto    ___eeprom16_gptrget3

	end
