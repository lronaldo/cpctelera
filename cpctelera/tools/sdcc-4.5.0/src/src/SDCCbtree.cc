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

#include <list>
#include <algorithm>
#include <map>
#include <iostream>

#include <boost/graph/adjacency_list.hpp>

#include "common.h"

extern "C"
{
#include "SDCCbtree.h"
}

#undef BTREE_DEBUG

// We used to use an std::set<symbol *> instead of std::list<symbol *>.
// That was faster, but resulted in non-reproducible compilation result, especially on systems with address space randomizatrion, such as macOS.
// When performance becomes an issue here, we might want to switch to boost::multi_index.
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, std::pair<std::list<symbol *>, int> > btree_t;

typedef std::map<int, btree_t::vertex_descriptor> bmap_t;
typedef std::map<btree_t::vertex_descriptor, int> bmaprev_t;

static btree_t btree;
static bmap_t bmap;
static bmaprev_t bmaprev;

void btree_clear_subtree(btree_t::vertex_descriptor v)
{
  btree[v].first.clear();

  boost::graph_traits<btree_t>::out_edge_iterator e, e_end;
  for(boost::tie(e, e_end) = boost::out_edges(v, btree); e != e_end; ++e)
    btree_clear_subtree(boost::target(*e, btree));
}

void btree_clear(void)
{
#ifdef BTREE_DEBUG
  std::cout << "Clearing.\n"; std::cout.flush();
#endif
  btree_clear_subtree(0);
}

void btree_add_child(int parent, int child)
{
#ifdef BTREE_DEBUG
  std::cout << "Adding child " << child << " at parent " << parent << "\n"; std::cout.flush();
#endif

  if(!boost::num_vertices(btree))
    {
      boost::add_vertex(btree);
      bmap[0] = 0;
      bmaprev[0] = 0;
    }
  
  wassert(parent != child);
  wassert(bmap.find(parent) != bmap.end());

  btree_t::vertex_descriptor c = boost::add_vertex(btree);
  bmap[child] = c;
  bmaprev[c] = child;

  wassert(bmap[parent] != c);

  boost::add_edge(bmap[parent], c, btree);
}

static btree_t::vertex_descriptor btree_lowest_common_ancestor_impl(btree_t::vertex_descriptor a, btree_t::vertex_descriptor b)
{
  if(a == b)
    return(a);
  else if (a > b)
    a = boost::source(*boost::in_edges(a, btree).first, btree);
  else // (a < b)
    b = boost::source(*boost::in_edges(b, btree).first, btree);
		
  return(btree_lowest_common_ancestor_impl(a, b));
}

int btree_lowest_common_ancestor(int a, int b)
{
  return(bmaprev[btree_lowest_common_ancestor_impl(bmap[a], bmap[b])]);
}

void btree_add_symbol(struct symbol *s)
{
  int block;
  wassert(s);
  block = s->_isparm ? 0 : s->block; // This is essentially a workaround. TODO: Ensure that the parameter block is placed correctly in the btree instead!

#ifdef BTREE_DEBUG
  std::cout << "Adding symbol " << s->name << " at " << block << "\n";
#endif

  wassert(bmap.find(block) != bmap.end());
  wassert(bmap[block] < boost::num_vertices(btree));
  if (std::find(btree[bmap[block]].first.begin(), btree[bmap[block]].first.end(), s) == btree[bmap[block]].first.end())
  	btree[bmap[block]].first.push_back(s);
}

static void btree_alloc_subtree(btree_t::vertex_descriptor v, int sPtr, int cssize, int *ssize)
{
  std::list<symbol *>::iterator s, s_end;
  wassert(v < boost::num_vertices(btree));
  for(s = btree[v].first.begin(), s_end = btree[v].first.end(); s != s_end; ++s)
    {
      struct symbol *const sym = *s;
      const int size = getSize (sym->type);

#ifdef BTREE_DEBUG
      std::cout << "Allocating symbol " << sym->name << " (" << v << ") of size " << size << " to " << sPtr << "\n";
#endif

      if(port->stack.direction > 0)
        {
          SPEC_STAK (sym->etype) = sym->stack = (sPtr + !TARGET_PDK_LIKE);
          sPtr += size;
        }
      else
        {
          sPtr -= size;
          SPEC_STAK (sym->etype) = sym->stack = sPtr;
        }
      
      cssize += size;
    }
  btree[v].second = cssize;
  if(cssize > *ssize)
    *ssize = cssize;
    
  boost::graph_traits<btree_t>::out_edge_iterator e, e_end;
  for(boost::tie(e, e_end) = boost::out_edges(v, btree); e != e_end; ++e)
    btree_alloc_subtree(boost::target(*e, btree), sPtr, cssize, ssize);
}

void btree_alloc(void)
{
  int ssize = 0;

  if(!boost::num_vertices(btree))
    return;

  btree_alloc_subtree(0, 0, 0, &ssize);

  if(currFunc)
    {
#ifdef BTREE_DEBUG
      std::cout << "btree stack allocation used total of " << ssize << " bytes\n";
#endif
      if (TARGET_PDK_LIKE) // Stack pointer needs to be aligned to multiple of 2
        if (ssize & 1) ssize++;
      currFunc->stack += ssize;
      SPEC_STAK (currFunc->etype) += ssize;
    }
}

