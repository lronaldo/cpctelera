	radix dec

	global	___eeprom_gptrget4
        extern  ___eeprom16_gptrget4

S___eeprom_gptrget4_stub    code
___eeprom_gptrget4:
        goto    ___eeprom16_gptrget4

	end
