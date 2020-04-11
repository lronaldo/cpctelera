/* sdas.h

   Copyright (C) 2009 Borut Razem

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef __SDAS_H
#define __SDAS_H

enum sdas_target_e {
  TARGET_ID_UNKNOWN,
  TARGET_ID_GB,
  TARGET_ID_Z80,
  TARGET_ID_8051,
  TARGET_ID_DS390,
  TARGET_ID_6808,
  TARGET_ID_STM8,
  TARGET_ID_PDK13 = 13,
  TARGET_ID_PDK14 = 14,
  TARGET_ID_PDK15 = 15,
  TARGET_ID_PDK16 = 16,
};

void sdas_init (char *path);
int is_sdas(void);
enum sdas_target_e get_sdas_target(void);
void set_sdas_target(enum sdas_target_e);
int is_sdas_target_z80_like(void);
int is_sdas_target_8051_like(void);
int is_sdas_target_stm8(void);
int is_sdas_target_pdk(void);

#endif  /* __SDAS_H */
