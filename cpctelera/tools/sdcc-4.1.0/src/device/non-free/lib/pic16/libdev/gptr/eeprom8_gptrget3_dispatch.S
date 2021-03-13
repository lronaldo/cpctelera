	radix dec

	global	___eeprom_gptrget3
        extern  ___eeprom8_gptrget3

S___eeprom_gptrget3_stub    code
___eeprom_gptrget3:
        goto    ___eeprom8_gptrget3

	end
