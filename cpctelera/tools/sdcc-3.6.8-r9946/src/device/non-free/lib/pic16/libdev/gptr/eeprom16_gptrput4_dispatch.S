	radix dec

	global	___eeprom_gptrput4
        extern  ___eeprom16_gptrput4

S___eeprom_gptrput4_stub    code
___eeprom_gptrput4:
        goto    ___eeprom16_gptrput4

	end
