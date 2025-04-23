// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2011-2018
//
// (c) 2011-2012 Goethe-Universität Frankfurt
// (c) 2018 Albert-Ludwigs-Universität Frankfurt
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
//
// A Chaitin-style stack allocator.

#ifndef SDCCSALLOC_HH
#define SDCCSALLOC_HH 1

#include <boost/graph/adjacency_list.hpp>

#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>

extern "C"
{
#include "SDCCmem.h"
#include "SDCCglobl.h"
}

// #define DEBUG_SALLOC

struct scon_node_t
{
  symbol *sym;
  int color;
  boost::icl::interval_set<int> free_stack;
  std::set<boost::icl::discrete_interval<int> > alignment_conflicts;
};

struct scon_edge_t
{
  bool alignment_conflict_only;
};

typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, scon_node_t, scon_edge_t> scon_t; // Conflict graph for on-stack variables

static bool clash (const symbol *s1, const symbol *s2)
{
  wassert(s1);
  wassert(s2);

  if(!s1->isspilt && !(IS_AGGREGATE(s1->type) || s1->allocreq && (s1->addrtaken || isVolatile(s1->type)))) // Spill location
    {
      for(const symbol *s = (const symbol *)setFirstItem (s1->usl.itmpStack); s; s = (const symbol *)setNextItem (s1->usl.itmpStack))
        if(clash(s, s2))
           return(true);
      return(false);
    }
  if(!s2->isspilt && !(IS_AGGREGATE(s2->type) || s2->allocreq && (s2->addrtaken || isVolatile(s2->type)))) // Spill location
    {
      for(const symbol *s = (const symbol *)setFirstItem (s2->usl.itmpStack); s; s = (const symbol *)setNextItem (s2->usl.itmpStack))
        if(clash(s1, s))
           return(true);
      return(false);
    }

  return(bitVectBitValue (s1->clashes, s2->key));
}

static var_t var_from_operand(const std::map<const symbol *, var_t>& symbol_to_sindex, const operand *const op)
{
  if(!op || !IS_SYMOP(op))
    return(-1);
  std::map<const symbol *, var_t>::const_iterator si = symbol_to_sindex.find(OP_SYMBOL_CONST(op));
  if (si == symbol_to_sindex.end())
    return(-1);

  return(si->second);
}

template<class G_t, class I_t, class SI_t>
static void set_spilt(G_t &G, const I_t &I, SI_t &scon)
{
  std::map<const symbol *, var_t> symbol_to_sindex;
  std::map<int, var_t> iindex_to_sindex;
  symbol *sym;
  var_t j, j_mark;

  // Add variables that need to be on the stack due to having had their address taken (or for a few other reasons, such as being too large or too many to behandled by the register allocator).
  for(sym = static_cast<symbol *>(setFirstItem(istack->syms)), j = 0; sym; sym = static_cast<symbol *>(setNextItem(istack->syms)))
    {
      if(sym->_isparm)
        continue;

      // std::cout << "set_spilt() 1: Considering " << sym->name << "[" << sym->isspilt << ", " << IS_AGGREGATE(sym->type) << ", " << sym->allocreq << ", " << sym->addrtaken << "]\n";

      if(/*!(IS_AGGREGATE(sym->type) || sym->allocreq && (sym->addrtaken || isVolatile(sym->type)))*/sym->for_newralloc)
        continue;

      if(!sym->isspilt && !(IS_AGGREGATE(sym->type) || sym->allocreq && (sym->addrtaken || isVolatile(sym->type)))) // Looks like a spill location - check if it is already covered by live ranges below.
        {
          bool covered = true;
          for (const symbol *s = (const symbol *)setFirstItem (sym->usl.itmpStack); s; s = (const symbol *)setNextItem (sym->usl.itmpStack))
            if (!s->for_newralloc)
              {
#ifdef DEBUG_SALLOC  
                std::cout << "Adding " << sym->name << " for " << s->name << "(" << s << ") to be allocated to stack. (" << s->for_newralloc << ")\n";
		std::cout.flush();
#endif
                covered = false;
                symbol_to_sindex[s] = j;
                break;
              }
          if(covered)
            continue;
        }
      
      boost::add_vertex(scon);
      symbol_to_sindex[sym] = j;
      scon[j].sym = sym;
      scon[j].color = -1;
      j++;
    }
  j_mark = j;

  // Add edges due to scope (see C99 standard, verse 1233, which requires things to have different addresses, not allowing us to allocate them to the same location, even if we otherwise could).
  for(unsigned int i = 0; i < boost::num_vertices(scon); i++)
     for(unsigned int j = i + 1; j < boost::num_vertices(scon); j++)
        {
          if (!(scon[i].sym->addrtaken) || !(scon[i].sym->addrtaken))
            continue;
          short p = btree_lowest_common_ancestor(scon[i].sym->block, scon[j].sym->block);
          if(p == scon[i].sym->block || p == scon[j].sym->block)
            boost::add_edge(i, j, scon);
        }

  // Set stack live ranges
  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      G[i].ic->localEscapeAlive = false;
      G[i].ic->parmEscapeAlive = false;

      for(unsigned int j = 0; j < boost::num_vertices(scon); j++)
        {
          short p = btree_lowest_common_ancestor(G[i].ic->block, scon[j].sym->block);
          if(p == G[i].ic->block || p == scon[j].sym->block)
            {
              G[i].stack_alive.insert(j);
              if (scon[j].sym->addrtaken || IS_AGGREGATE(scon[j].sym->type)) // TODO: More accurate analysis.
                G[i].ic->localEscapeAlive = true;
            }
        }
    }

  // Add variables that have been spilt in register allocation.
  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      cfg_alive_t::const_iterator v, v_end;
      for (v = G[i].alive.begin(), v_end = G[i].alive.end(); v != v_end; ++v)
        {
          var_t vs;

          symbol *const sym = (symbol *)(hTabItemWithKey(liveRanges, I[*v].v));

          if ((sym->regs[0] && !sym->isspilt) || sym->accuse || sym->remat || !sym->nRegs || sym->usl.spillLoc && sym->usl.spillLoc->_isparm)
            continue;

          if (iindex_to_sindex.find(I[*v].v) == iindex_to_sindex.end())
            {
              wassert(boost::add_vertex(scon) == j);
              scon[j].sym = sym;
              scon[j].color = -1;
              iindex_to_sindex[I[*v].v] = j;
              symbol_to_sindex[sym] = j;
              j++;
            }

          vs = iindex_to_sindex[I[*v].v];

          G[i].stack_alive.insert(vs); // Needs to be allocated on the stack.
        }
    }

  // Add edges to conflict graph.
  typename boost::graph_traits<I_t>::edge_iterator e, e_end;
  for (boost::tie(e, e_end) = boost::edges(I); e != e_end; ++e)
    {
      if (I[boost::source(*e, I)].v == I[boost::target(*e, I)].v || iindex_to_sindex.find(I[boost::source(*e, I)].v) == iindex_to_sindex.end() || iindex_to_sindex.find(I[boost::target(*e, I)].v) == iindex_to_sindex.end())
        continue;

      boost::add_edge(iindex_to_sindex[I[boost::source(*e, I)].v], iindex_to_sindex[I[boost::target(*e, I)].v], scon);
    }
    
  // Add conflicts between variables that had their address taken and those that have been spilt by register allocation.
  // TODO: More exact live range analysis for variables that had their address taken (to reduce stack space consumption further, by reducing the number of conflicts here).
  for(unsigned int i = 0; i < j_mark; i++)
    for(unsigned int j = 0; j < boost::num_vertices(scon); j++)
      {
        if (i == j)
          continue;
        if(!scon[i].sym->isspilt && !(IS_AGGREGATE(scon[i].sym->type) || scon[i].sym->allocreq && (scon[i].sym->addrtaken || isVolatile(scon[i].sym->type)))) // Spill location
          {
            if (clash (scon[i].sym, scon[j].sym))
              boost::add_edge(i, j, scon);
            continue;
          }
        short p = btree_lowest_common_ancestor(scon[i].sym->block, scon[j].sym->block);
        if(p == scon[i].sym->block || p == scon[j].sym->block)
          boost::add_edge(i, j, scon);
      }

  // Ugly hack: Regparms.
  for(sym = static_cast<symbol *>(setFirstItem(istack->syms)), j = boost::num_vertices(scon); sym; sym = static_cast<symbol *>(setNextItem(istack->syms)))
    {
      if (sym->_isparm && !IS_REGPARM(sym->etype) && sym->addrtaken)
        for(unsigned int i = 0; i < boost::num_vertices(G); i++)
          G[i].ic->parmEscapeAlive = true;
      
      if(!sym->_isparm || !IS_REGPARM(sym->etype) || !sym->onStack || !sym->allocreq)
        continue;
      
      boost::add_vertex(scon);
      scon[j].sym = sym;
      scon[j].color = -1;

      // Extend liverange to cover everything.
      for(unsigned int i = 0; i < boost::num_vertices(G); i++)
        {
          G[i].stack_alive.insert(j);
          if (sym->addrtaken)
            G[i].ic->localEscapeAlive = true;
        }

      // Conflict with everything.
      for(unsigned int i = 0; i < j; i++)
        boost::add_edge(i, j, scon);

      j++;
    }

  // Edges for alignment conflict
  typename SI_t::edge_iterator ei, ei_end;
  for(boost::tie(ei, ei_end) = boost::edges(scon); ei != ei_end; ++ei)
    scon[*ei].alignment_conflict_only = false;
  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    {
      const var_t result = var_from_operand (symbol_to_sindex, IC_RESULT(G[i].ic));

      if(result < 0)
        continue;

      const var_t left = var_from_operand (symbol_to_sindex, IC_LEFT(G[i].ic));
      const var_t right = var_from_operand (symbol_to_sindex, IC_RIGHT(G[i].ic));

      if(left >= 0 && !boost::edge (result, left, scon).second)
        {
          scon[(boost::add_edge(result, left, scon)).first].alignment_conflict_only = true;
          if (TARGET_PDK_LIKE && G[i].ic->op == GET_VALUE_AT_ADDRESS && getSize(scon[result].sym->type) > 2) // Padauk still needs pointer read operand, since pointer read of more than 2 bytes is broken into multiple support routine calls.
            scon[(boost::add_edge(result, left, scon)).first].alignment_conflict_only = false;
          if ((TARGET_IS_STM8 || TARGET_IS_F8) && (G[i].ic->op == RIGHT_OP || G[i].ic->op == LEFT_OP) && IS_OP_LITERAL(IC_RIGHT(G[i].ic)) && ulFromVal (OP_VALUE_CONST (IC_RIGHT(G[i].ic))) >= 6) // Byte shifting in shift by constant might fail for partially spilt variables. Currently only the stm8 and f8 register allocators might partially spill variables.
            scon[(boost::add_edge(result, left, scon)).first].alignment_conflict_only = false;
        }
      if(right >= 0 && !boost::edge (result, right, scon).second)
        scon[(boost::add_edge(result, right, scon)).first].alignment_conflict_only = true;
    }
}

template <class SI_t>
void color_stack_var(const var_t v, SI_t &SI, int start, int *ssize)
{
  symbol *const sym = SI[v].sym;
  const int size = getSize(sym->type);
  
  SI[v].color = start;

  const int sloc = (port->stack.direction > 0) ? start : -start - size;
  symbol *const ssym = (sym->isspilt && sym->usl.spillLoc) ? sym->usl.spillLoc : sym;

  SPEC_STAK(ssym->etype) = ssym->stack = sloc;
    
  if(ssize)
    *ssize = (start + size > *ssize) ? start + size : *ssize;

#ifdef DEBUG_SALLOC    
  std::cout << "Placing " << sym->name << " (really " << ssym->name << ") at [" << start << ", " << (start + size - 1) << "]\n";
  std::cout.flush();
#endif
    
  // Mark stack location as used for all conflicting variables.
  typename boost::graph_traits<SI_t>::adjacency_iterator n, n_end;
  for(boost::tie(n, n_end) = boost::adjacent_vertices(v, SI); n != n_end; ++n)
    if (!SI[boost::edge(v, *n, SI).first].alignment_conflict_only)
      SI[*n].free_stack -= boost::icl::discrete_interval<int>::type(start, start + size);
    else
      SI[*n].alignment_conflicts.insert(boost::icl::discrete_interval<int>::type(start, start + size));
}

// Place a single variable on the stack greedily.
template <class SI_t>
void color_stack_var_greedily(const var_t v, SI_t &SI, int alignment, int *ssize)
{
  int start;
  symbol *const sym = SI[v].sym;
  const int size = getSize(sym->type);
 
  // Find a suitable free stack location.
  boost::icl::interval_set<int>::iterator si, si_end;
  for(si = SI[v].free_stack.begin(), si_end = SI[v].free_stack.end(); si != si_end; ++si)
    {
       start = boost::icl::first(*si);

       bool alignment_issue;
       do
         {
           // Adjust start address for alignment conflict
           std::set<boost::icl::discrete_interval<int> >::const_iterator ai, ai_end;
           for(ai = SI[v].alignment_conflicts.begin(), ai_end = SI[v].alignment_conflicts.end(); ai != ai_end; ++ai)
             {
               if(ai->upper() < start || ai->lower() > start + size - 1)
                 continue;
               if(ai->lower() == start)
                 continue;

#ifdef DEBUG_SALLOC   
               std::cerr << "Resolving alignment conflict at " << SI[v].sym->name << "\n";
#endif

               start = ai->upper() + 1; // Resolve conflict.
             }

           // Adjust start address for alignment
           alignment_issue = start % alignment;
           if(start % alignment)
             start = start + alignment - start % alignment;
         }
       while (alignment_issue);
   
       if(boost::icl::last(*si) >= start + size - 1)
         break; // Found one.
    }

  if (si == si_end)
    werror (E_CANNOT_ALLOC, SI[v].sym->name);
  else
    color_stack_var(v, SI, start, ssize);
}

static
int get_alignment(sym_link *type)
{
#if 1
  return(1);
#else
  for(; IS_ARRAY (type); type = type->next);

  switch(getSize(type))
    {
    case 0: // ?
    case 1:
      return(1);
    case 2:
      return(2);
    case 3:
    case 4:
      return(4);
    default:
      return(8);
    }
#endif
}

template <class SI_t>
void chaitin_ordering(const SI_t &SI, std::list<var_t> &ordering)
{
  std::vector<bool> marked(boost::num_vertices(SI));
  unsigned int num_marked, i, d, mind, minn;
  std::stack<var_t> stack;
  
  for(num_marked = 0; num_marked < boost::num_vertices(SI); num_marked++)
    {
      mind = UINT_MAX;
      minn = -1;
      for(i = 0; i < boost::num_vertices(SI); i++)
        {
          if(marked[i])
            continue;
          
          typename boost::graph_traits<const SI_t>::adjacency_iterator n, n_end;
          for(boost::tie(n, n_end) = boost::adjacent_vertices(i, SI), d = 0; n != n_end; ++n)
             d += !marked[*n];
             
          if(d < mind || d == mind && get_alignment(SI[i].sym->type) < get_alignment(SI[minn].sym->type)) // Coloring aligned variables first tends to keep gaps from alignment small.
            {
              mind = d;
              minn = i;
            }
        }
        
      stack.push(minn);
      marked[minn] = true;
    }
    
  while(!stack.empty())
    {
      ordering.push_back(stack.top());
      stack.pop();
    }
}

template <class SI_t>
void chaitin_salloc(SI_t &SI)
{
  std::list<var_t> ordering;
  
  chaitin_ordering(SI, ordering);
  
  for(unsigned int i = 0; i < boost::num_vertices(SI); i++)
      SI[i].free_stack.insert(boost::icl::discrete_interval<int>::type(0, 1 << 15));
    
  int ssize = 0;
  
  clearStackOffsets();
  
  std::list<var_t>::const_iterator i, i_end;
  for(i = ordering.begin(), i_end = ordering.end(); i != i_end; ++i)
    {
      // Alignment, even when not required by the hardware helps avoid partially overlapping stack operands (which are not supported by code generation in some backends).
      color_stack_var_greedily(*i, SI, get_alignment (SI[*i].sym->type), &ssize);
    }
  
  if(currFunc)
    {
      if (TARGET_PDK_LIKE && (ssize % 2)) // Padauk requires even stack alignment.
        ssize++;
#ifdef DEBUG_SALLOC
      std::cout << "Function " << currFunc->name << " currFunc->stack: old " << currFunc->stack << ", new " << (currFunc->stack + ssize) << "\n";
#endif
      currFunc->stack += ssize;
      SPEC_STAK (currFunc->etype) += ssize;
    }
}

static
void dump_scon(const scon_t &scon)
{
  if(!currFunc)
    return;

  std::ofstream dump_file((std::string(dstFileName) + ".dumpsalloccon" + currFunc->rname + ".dot").c_str());

  std::string *name = new std::string[boost::num_vertices(scon)];
  for(var_t i = 0; static_cast<boost::graph_traits<scon_t>::vertices_size_type>(i) < boost::num_vertices(scon); i++)
    {
      int start = scon[i].color;
      std::ostringstream os;
      os << i;
      if (scon[i].sym)
        os << " : " << scon[i].sym->name << " : " << getSize(scon[i].sym->type) << " [" << start << "," << (start + getSize(scon[i].sym->type) - 1) << "]";
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, scon, boost::make_label_writer(name));
  delete[] name;
}

#if BOOST_VERSION / 100000 == 1 && BOOST_VERSION / 100 % 1000 < 79
#error boost 1.71 (and possibly earlier, but apparently fixed as of boost 1.79.0) bug: clear_vertex followed by remove_vertex could introduce loops into the graph (and omit non-loop edges). See SDCC bug #3772.
#endif

// Save stack space by merging spilt variables into spill location of stack parameters.
// Currently handles only a single variable per function, with some additional restrictions.
template <class SI_t>
void mergeSpiltParms(SI_t &SI)
{
  for(unsigned int i = 0; i < boost::num_vertices(SI); i++)
    {
      symbol *psym = 0;
      operand *parmop = 0;
      iCode *dic = 0;
      bitVect *defs = bitVectCopy (SI[i].sym->defs);

      for(int key = bitVectFirstBit (defs); bitVectnBitsOn (defs); bitVectUnSetBit (defs, key), key = bitVectFirstBit (defs))
        {
          dic = (iCode *)hTabItemWithKey (iCodehTab, key);
          if (!dic || !dic->result) // Something went wrong.
            break;
          if(dic->op == '=' && !POINTER_SET(dic) && IS_PARM(dic->right) && !IS_REGPARM (OP_SYMBOL(dic->right)->etype))
            {
              psym = OP_SYMBOL (dic->right);
              break;
            }
        }
      freeBitVect (defs);

      if (!psym || bitVectnBitsOn(psym->defs) > 0 || bitVectnBitsOn(psym->uses) > 1)
        continue;

      for(iCode *ic = dic->prev; ic; ic = ic->prev)
        {
          if (ic->op == LABEL)
            goto no;
          if (isOperandEqual(ic->left, dic->result) ||
            isOperandEqual(ic->right, dic->result) ||
            POINTER_SET(ic) && isOperandEqual(ic->result, dic->result))
            goto no;
          if (ic->op == FUNCTION)
            break;
        }

      // Merge spill location of spilt variable into stack parameter.
      SI[i].sym->usl.spillLoc = psym;
      SI[i].sym->stack = psym->stack; // Needed for volatile local variables.
      clear_vertex(i, SI);
      remove_vertex(i, SI);
      break;
no:
      ;
    }
}
#endif

