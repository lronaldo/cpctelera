/*-------------------------------------------------------------------------
  SDCCattr.h - Header file for attributes.

  Copyright (c) 2021 Philipp Klaus Krause pkk@spth.de

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2, or (at your option) any
  later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-------------------------------------------------------------------------*/

#ifndef SDCCATTR_H
#define SDCCATTR_H 1

struct symbol;
typedef struct symbol symbol;

typedef enum
{
  ATTRIBUTE_NODISCARD = 1,
  ATTRIBUTE_MAYBE_UNUSED,
  ATTRIBUTE_DEPRECATED,
  ATTRIBUTE_FALLTHROUGH,
  ATTRIBUTE_OTHER
} ATTRIBUTE_TOKEN;

typedef struct attribute
{
  ATTRIBUTE_TOKEN token;
  char token_string[32];
  char argument_clause[32];
  
  struct attribute *next;	// Next attribute in attribute list.
}
attribute;

attribute *newAttribute(const symbol *token_sym, const char *argument_clause);

#endif

