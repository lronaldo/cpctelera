	radix dec

	global	___eeprom_gptrget1
        extern  ___eeprom16_gptrget1

S___eeprom_gptrget1_stub    code
___eeprom_gptrget1:
        goto    ___eeprom16_gptrget1

	end
