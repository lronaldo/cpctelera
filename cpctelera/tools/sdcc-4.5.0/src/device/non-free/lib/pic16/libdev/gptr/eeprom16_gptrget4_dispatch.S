	radix dec

	global	___eeprom_gptrload
	global	___eeprom_gptrget4
        extern  ___eeprom16_gptrload
        extern  ___eeprom16_gptrget4

S___eeprom_gptrload_stub    code
___eeprom_gptrload:
        goto    ___eeprom16_gptrload
S___eeprom_gptrget4_stub    code
___eeprom_gptrget4:
        goto    ___eeprom16_gptrget4

	end
