/*-------------------------------------------------------------------------
  SDCCattr.h - Code file for attributes.

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

#include "SDCCattr.h"
#include "SDCCsymt.h"
#include "SDCCutil.h"
#include "newalloc.h"

attribute *newAttribute(const symbol *token_sym, const char *argument_clause)
{
  const char *token_string, *token_string_prefix;

  attribute *attr = Safe_alloc (sizeof (attribute));
  
  attr->next = 0;

  wassert (token_sym);
  
  if(token_sym->next)
    {
      token_string = token_sym->next->name;
      token_string_prefix = token_sym->name;
    }
  else
    {
      token_string = token_sym->name;
      token_string_prefix = 0;
    }

  wassert(token_string);
  strncpyz (attr->token_string, token_string, sizeof (attr->token_string));

  if (!token_string_prefix)
    {
      if(!strcmp(token_string, "nodiscard") || !strcmp(token_string, "__nodiscard__"))
        attr->token = ATTRIBUTE_NODISCARD;
      else if(!strcmp(token_string, "maybe_unused") || !strcmp(token_string, "__maybe_unused__"))
        attr->token = ATTRIBUTE_MAYBE_UNUSED;
      else if(!strcmp(token_string, "deprecated") || !strcmp(token_string, "__deprecated__"))
        attr->token = ATTRIBUTE_DEPRECATED;
      else if(!strcmp(token_string, "fallthrough") || !strcmp(token_string, "__fallthrough__"))
        attr->token = ATTRIBUTE_FALLTHROUGH;
      else // Unknown standard attribute
        attr->token = ATTRIBUTE_OTHER;
    }
  else if(!strcmp(token_string_prefix, "sdcc")) // SDCC implementation-specific attribute
    {
      attr->token = ATTRIBUTE_OTHER;
    }
  else // Unknown-implementation implementation-specific unknown attribute.
    attr->token = ATTRIBUTE_OTHER;
    
  return attr;
}

