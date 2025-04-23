	radix dec

	global	___eeprom_gptrload
	global	___eeprom_gptrget4
        extern  ___eeprom8_gptrget4
        extern  ___eeprom8_gptrload

S___eeprom_gptrload_stub    code
___eeprom_gptrload:
        goto    ___eeprom8_gptrload

S___eeprom_gptrget4_stub    code
___eeprom_gptrget4:
        goto    ___eeprom8_gptrget4

	end
