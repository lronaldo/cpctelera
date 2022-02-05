//-----------------------------LICENSE NOTICE------------------------------------
//  This file is part of CPCtelera: An Amstrad CPC Game Engine
//  Copyright (C) 2022 Nestornillo (https://github.com/nestornillo)
//  Copyright (C) 2022 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
//------------------------------------------------------------------------------
#pragma ONCE

// INTERRUPT HANDLER WRAPPER DECLARATION
//    Declare an interrupt handler wrapper function named 'MyInterruptWrapper'.
//
cpctm_declareInterruptHandlerWrapper(MyInterruptWrapper);

// NOTES:
// 
// This is just a declaration, not a definition. The actual code for the wrapper
// is produced by the macro cpctm_createInterruptHandlerWrapper, in interrupts.c.
// This only produces the signature declaration for the compiler to know the
// interrupt wrapper function exists. Therefore, any translation-unit wanting
// to use (i.e. call) this interrupt handler wrapper, can do so by simple
// including this header file, hence having this declaration.
//
// This is exactly the same as having a function defined in a .c file (it's own 
// translation unit), then a declaration of the function in the header .h file.
// This enables other translation onits to call the function by simply
// including the header file, hence having the function declaration.
// It is exactly the same, because MyInterruptWrapper is indeed a function.
// Macros are just assisting you to define (create) and declare the function.
//
