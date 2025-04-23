/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */



#ifndef REGPBLAZE_HEADER
#define REGPBLAZE_HEADER

#include "ddconfig.h"

#define REGISTER_BANK_A 0x01
#define REGISTER_BANK_B 0x02

#define FLAGS_GET(bit_position) (sfr->get(FLAGS) & bit_position)
#define FLAGS_GET_C (sfr->get(FLAGS) & bmC)
#define FLAGS_GET_Z (sfr->get(FLAGS) & bmZ)
#define FLAGS_GET_I (sfr->get(FLAGS) & bmI)
/*
#define FLAGS_SET(flag_value, bit_position) \
  if (flag_value) \
    sfr->set_bit1(FLAGS, bit_position); \
  else \
    sfr->set_bit0(FLAGS, bit_position);
*/
#define FLAGS_SET(flag_value, bit_position) \
  if (flag_value) \
    sfr->write(FLAGS, sfr->read(FLAGS) | bit_position);	\
  else \
    sfr->write(FLAGS, sfr->read(FLAGS) & ~bit_position);
#define FLAGS_SET_C(flag_value) FLAGS_SET(flag_value, bmC)
#define FLAGS_SET_Z(flag_value) FLAGS_SET(flag_value, bmZ)
#define FLAGS_SET_I(flag_value) FLAGS_SET(flag_value, bmI)


#define S0 0x00
#define S1 0x01
#define S2 0x02
#define S3 0x03
#define S4 0x04
#define S5 0x05
#define S6 0x06
#define S7 0x07
#define S8 0x08
#define S9 0x09
#define SA 0x0a
#define SB 0x0b
#define SC 0x0c
#define SD 0x0d
#define SE 0x0e
#define SF 0x0f

#define S0b 0x10
#define S1b 0x11
#define S2b 0x12
#define S3b 0x13
#define S4b 0x14
#define S5b 0x15
#define S6b 0x16
#define S7b 0x17
#define S8b 0x18
#define S9b 0x19
#define SAb 0x1a
#define SBb 0x1b
#define SCb 0x1c
#define SDb 0x1d
#define SEb 0x1e
#define SFb 0x1f

#define FLAGS 0x20
#define SP 0x21

// bit position of each flag in FLAGS register
#define bmC 0x01
#define bmZ 0x02
#define bmI 0x04

#endif

/* Emd of pblaze.src/regspblaze.h */
