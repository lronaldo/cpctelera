// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2011
//
// (c) 2011 Goethe-Universität Frankfurt
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef SDCCBTREE_H
#define SDCCBTREE_H


// Clear block tree. To be called after each function.
void btree_clear(void);

// Add child as a sub-block of parent.
void btree_add_child(short parent, short child);

// Gives the lowest common ancestor for blocks a and b.
short btree_lowest_common_ancestor(short a, short b);

// Add symbol to block tree for allocation.
void btree_add_symbol(struct symbol *s);

// Allocate all previously added symbols on the stack.
void btree_alloc(void);

#endif

