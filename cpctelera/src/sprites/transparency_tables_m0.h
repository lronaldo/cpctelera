//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine 
//  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------

#ifndef _TRANSPARENCY_TABLES_MODE0_H
#define _TRANSPARENCY_TABLES_MODE0_H

//----------------------------------------------------------------------------------------
// Title: Transparency Tables for Mode 0
//----------------------------------------------------------------------------------------

//
// Macro: CPCTM_MASKTABLE0M0
//    256-table (assembly definition) with mask values for mode 0 using pen 0 as transparent
//
#define CPCTM_MASKTABLE0M0 \
      .db 0xFF, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE1M0
//    256-table (assembly definition) with mask values for mode 0 using pen 1 as transparent
//
#define CPCTM_MASKTABLE1M0 \
      .db 0x00, 0x55, 0xAA, 0xFF, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE2M0
//    256-table (assembly definition) with mask values for mode 0 using pen 2 as transparent
//
#define CPCTM_MASKTABLE2M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xFF, 0xAA, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE3M0
//    256-table (assembly definition) with mask values for mode 0 using pen 3 as transparent
//
#define CPCTM_MASKTABLE3M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x55, 0xAA, 0xFF \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE4M0
//    256-table (assembly definition) with mask values for mode 0 using pen 4 as transparent
//
#define CPCTM_MASKTABLE4M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xFF, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE5M0
//    256-table (assembly definition) with mask values for mode 0 using pen 5 as transparent
//
#define CPCTM_MASKTABLE5M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0xAA, 0xFF, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE6M0
//    256-table (assembly definition) with mask values for mode 0 using pen 6 as transparent
//
#define CPCTM_MASKTABLE6M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xFF, 0xAA, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE7M0
//    256-table (assembly definition) with mask values for mode 0 using pen 7 as transparent
//
#define CPCTM_MASKTABLE7M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x55, 0xAA, 0xFF \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
//
// Macro: CPCTM_MASKTABLE8M0
//    256-table (assembly definition) with mask values for mode 0 using pen 8 as transparent
//
#define CPCTM_MASKTABLE8M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xFF, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE9M0
//    256-table (assembly definition) with mask values for mode 0 using pen 9 as transparent
//
#define CPCTM_MASKTABLE9M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0xAA, 0xFF, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE10M0
//    256-table (assembly definition) with mask values for mode 0 using pen 10 as transparent
//
#define CPCTM_MASKTABLE10M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xFF, 0xAA, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE11M0
//    256-table (assembly definition) with mask values for mode 0 using pen 11 as transparent
//
#define CPCTM_MASKTABLE11M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x55, 0xAA, 0xFF \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE12M0
//    256-table (assembly definition) with mask values for mode 0 using pen 12 as transparent
//
#define CPCTM_MASKTABLE12M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xFF, 0xAA, 0x55, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE13M0
//    256-table (assembly definition) with mask values for mode 0 using pen 13 as transparent
//
#define CPCTM_MASKTABLE13M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x55, 0xAA, 0xFF, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x55, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00

//
// Macro: CPCTM_MASKTABLE14M0
//    256-table (assembly definition) with mask values for mode 0 using pen 14 as transparent
//
#define CPCTM_MASKTABLE14M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55, 0x00 \
      .db 0xAA, 0xAA, 0x00, 0x00, 0xFF, 0xAA, 0x55, 0x00

//
// Macro: CPCTM_MASKTABLE15M0
//    256-table (assembly definition) with mask values for mode 0 using pen 15 as transparent
//
#define CPCTM_MASKTABLE15M0 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x00, 0xAA, 0xAA \
      .db 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x55 \
      .db 0x00, 0x00, 0xAA, 0xAA, 0x00, 0x55, 0xAA, 0xFF

#endif