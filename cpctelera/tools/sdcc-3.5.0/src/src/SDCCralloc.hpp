// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2013
//
// (c) 2010-2012 Goethe-Universität Frankfurt
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
// An optimal, polynomial-time register allocator.
//
// For details, see:
//
// Philipp Klaus Krause,
// "Optimal Register Allocation in Polynomial Time",
// Compiler Construction - 22nd International Conference, CC 2013, Held as Part of the European Joint Conferences on Theory and Practice of Software, ETAPS 2013. Proceedings,
// Lecture Notes in Computer Science, volume 7791, pp. 1-20.
// Springer,
// 2013.
//
// To use this from a port do the following:
//
// 1) Supply a cost function
// template <class G_t, class I_t>
// float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I);
// Which can range from
// simple, e.g. cost 1 for each byte accessed in a register, cost 4 for each byte accessed in memory
// to
// quite involved, e.g. the number of bytes of code the code generator would generate.
//
// 2) Call
// create_cfg(), thorup_tree_decomposition(), nicify(), alive_tree_dec(), tree_dec_ralloc_nodes().
//
// The Z80 port can serve as an example, see z80_ralloc2_cc() in z80/ralloc2.cc.

#ifndef SDCCRALLOC_HH
#define SDCCRALLOC_HH 1

#include <iostream>
#include <limits>
#include <utility>
#include <sstream>
#include <fstream>

// Workaround for boost bug #11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
   #include <boost/type_traits/ice.hpp>
#endif

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/connected_components.hpp>

#include "common.h"
#include "SDCCtree_dec.hpp"

#ifdef HAVE_STX_BTREE_SET_H
#include <stx/btree_set.h>
#endif
#ifdef HAVE_STX_BTREE_MAP_H
#include <stx/btree_map.h>
#endif

extern "C"
{
#include "SDCCbtree.h"
}

typedef short int var_t;
typedef signed char reg_t;

// Integer constant upper bound on port->num_regs
#define MAX_NUM_REGS 9

// Assignment at an instruction
struct i_assignment_t
{
  var_t registers[MAX_NUM_REGS][2];

  i_assignment_t(void)
  {
    for (reg_t r = 0; r < MAX_NUM_REGS; r++)
      for (unsigned int i = 0; i < 2; i++)
        registers[r][i] = -1;
  }

#if 0
  bool operator<(const i_assignment_t &i_a) const
  {
    for (reg_t r = 0; r < port->num_regs; r++)
      for (unsigned int i = 0; i < 2; i++)
        {
          if (registers[r][i] < i_a.registers[r][i])
            return(true);
          else if (registers[r][i] > i_a.registers[r][i])
            return(false);
        }
    return(false);
  }
#endif

  void add_var(var_t v, reg_t r)
  {
    if (registers[r][1] < v)
      {
        registers[r][0] = registers[r][1];
        registers[r][1] = v;
      }
    else
      registers[r][0] = v;
  }

  void remove_var(var_t v)
  {
    for (reg_t r = 0; r < port->num_regs; r++)
      {
        if (registers[r][1] == v)
          {
            registers[r][1] = registers[r][0];
            registers[r][0] = -1;
          }
        else if (registers[r][0] == v)
          registers[r][0] = -1;
      }
  }
};

#ifdef HAVE_STX_BTREE_SET_H
typedef stx::btree_set<var_t> varset_t; // Faster than std::set
#else
typedef std::set<var_t> varset_t;
#endif
//typedef std::set<var_t, std::less<var_t>, boost::fast_pool_allocator<var_t> > varset_t; // Slower than ordinary std::set

#ifdef HAVE_STX_BTREE_MAP_H
typedef stx::btree_map<int, float> icosts_t; // Faster than std::map
#else
typedef std::map<int, float> icosts_t;
#endif
//typedef std::tr1::unordered_set<var_t> varset_t; // Speed about the same as std::set

struct assignment
{
  float s;

  varset_t local;	// Entries: var
  std::vector<reg_t> global;	// Entries: global[var] = reg (-1 if no reg assigned)
  icosts_t i_costs;  // Costs for all instructions in bag (needed to avoid double counting costs at join nodes)
  i_assignment_t i_assignment; // Assignment at the instruction currently being added in an introduce node;

  bool marked;

  bool operator<(const assignment& a) const
  {
    varset_t::const_iterator i, ai, i_end, ai_end;

    i_end = local.end();
    ai_end = a.local.end();

    for (i = local.begin(), ai = a.local.begin();; ++i, ++ai)
      {
        if (i == i_end && ai == ai_end)
          return(false);
        if (i == i_end)
          return(true);
        if (ai == ai_end)
          return(false);

        if (*i < *ai)
          return(true);
        if (*i > *ai)
          return(false);

        if (global[*i] < a.global[*ai])
          return(true);
        if (global[*i] > a.global[*ai])
          return(false);
      }
  }
};

typedef std::list<assignment> assignment_list_t;
//typedef std::vector<assignment> assignment_list_t; // Probably faster, but would require some code reorganization.

struct tree_dec_node
{
  std::set<unsigned int> bag;
  std::set<var_t> alive;
  assignment_list_t assignments;
  unsigned weight; // The weight is the number of nodes at which intermediate results need to be remembered. In general, to minimize memory consumption, at join nodes the child with maximum weight should be processed first.
};

typedef std::multimap<int, var_t> operand_map_t;
//typedef stx::btree_multimap<int, var_t> operand_map_t; // Slightly slower than std::multimap.

struct cfg_node
{
  iCode *ic;
  operand_map_t operands;
  std::set<var_t> alive;
  std::set<var_t> dying;

#ifdef DEBUG_SEGV
  cfg_node(void);
#endif
};

#ifdef DEBUG_SEGV
// This only exists to track down #3506333 and #3475617.
bool default_constructor_of_cfg_node_called;
cfg_node::cfg_node(void)
{
  default_constructor_of_cfg_node_called = true;
}
#endif

struct con_node
{
  int v;
  int byte;
  int size;
  char *name;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, tree_dec_node> tree_dec_t;
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, con_node> con_t;
typedef boost::adjacency_matrix<boost::undirectedS, con_node> con2_t;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, cfg_node> cfg_t;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> cfg_sym_t;

// Cost function. Port-specific.
template <class G_t, class I_t>
static float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I);

// For early removel of assignments that cannot be extended to valid assignments. Port-specific.
template <class G_t, class I_t>
static bool assignment_hopeless(const assignment &a, unsigned short int i, const G_t &G, const I_t &I, const var_t lastvar);

// Rough cost estimate. Port-specific.
template <class G_t, class I_t>
static float rough_cost_estimate(const assignment &a, unsigned short int i, const G_t &G, const I_t &I);

// Avoid overwriting operands that are still needed by the result. Port-specific.
template <class I_t>
static void add_operand_conflicts_in_node(const cfg_node &n, I_t &I);

// Port-specific
template <class T_t>
static void get_best_local_assignment_biased(assignment &a, typename boost::graph_traits<T_t>::vertex_descriptor t, const T_t &T);

// Code for another ic is generated when generating this one. Mark the other as generated. Port-specific.
static void extra_ic_generated(iCode *ic);

inline void
add_operand_to_cfg_node(cfg_node &n, operand *o, std::map<std::pair<int, reg_t>, var_t> &sym_to_index)
{
  reg_t k;
  if (o && IS_SYMOP(o) && sym_to_index.find(std::pair<int, reg_t>(OP_SYMBOL_CONST(o)->key, 0)) != sym_to_index.end())
    {
      if (n.operands.find(OP_SYMBOL_CONST(o)->key) == n.operands.end())
        for (k = 0; k < OP_SYMBOL_CONST(o)->nRegs; k++)
          n.operands.insert(std::pair<const int, var_t>(OP_SYMBOL_CONST(o)->key, sym_to_index[std::pair<int, reg_t>(OP_SYMBOL_CONST(o)->key, k)]));
    }
}

// A quick-and-dirty function to get the CFG from sdcc.
static inline iCode *
create_cfg(cfg_t &cfg, con_t &con, ebbIndex *ebbi)
{
  eBBlock **ebbs = ebbi->bbOrder;
  iCode *start_ic;
  iCode *ic;

  std::map<int, unsigned int> key_to_index;
  std::map<std::pair<int, reg_t>, var_t> sym_to_index;

  if(currFunc)
    currFunc->div_flag_safe = 1;

  start_ic = iCodeLabelOptimize(iCodeFromeBBlock (ebbs, ebbi->count));
  {
    int i;
    var_t j;
    wassertl (!boost::num_vertices(cfg), "CFG non-empty before creation.");
    for (ic = start_ic, i = 0, j = 0; ic; ic = ic->next, i++)
      {
        if (currFunc)
          currFunc->div_flag_safe &= !(ic->op == INLINEASM || ic->op == '/' || ic->op == '%' || ic->op == PCALL ||
            ic->op == CALL && (IS_OP_LITERAL (IC_LEFT (ic)) || !OP_SYMBOL(IC_LEFT (ic))->div_flag_safe) ||
            ic->op == RIGHT_OP && IS_OP_LITERAL (IC_RIGHT (ic))); // Right shift might be implemented using division.

#ifdef DEBUG_SEGV
        default_constructor_of_cfg_node_called = false;
#endif
        boost::add_vertex(cfg);

#ifdef DEBUG_SEGV
        wassertl (default_constructor_of_cfg_node_called, "add_vertex failed to call default constructor of cfg_node!");
#endif
        wassertl (cfg[i].alive.empty(), "Alive set non-empty upon creation.");
        key_to_index[ic->key] = i;

        if(ic->op == SEND && ic->builtinSEND) // Ensure that only the very first send iCode is active.
          {
            operand *bi_parms[MAX_BUILTIN_ARGS];
            int nbi_parms;
            getBuiltinParms(ic, &nbi_parms, bi_parms);
          }

        extra_ic_generated(ic);

        cfg[i].ic = ic;

        if (ic->generated)
          continue;

        for (int j2 = 0; j2 <= operandKey; j2++)
          {
            if (bitVectBitValue(ic->rlive, j2))
              {
                symbol *sym = (symbol *)(hTabItemWithKey(liveRanges, j2));

                if (!sym->for_newralloc)
                  continue;

                // Add node to conflict graph:
                if (sym_to_index.find(std::pair<int, reg_t>(j2, 0)) != sym_to_index.end())
                  continue;

                // Other parts of the allocator may rely on the variables corresponding to bytes from the same sdcc variable to have subsequent numbers.
                for (reg_t k = 0; k < sym->nRegs; k++)
                  {
                    boost::add_vertex(con);
                    con[j].v = j2;
                    con[j].byte = k;
                    con[j].size = sym->nRegs;
                    con[j].name = sym->name;
                    sym_to_index[std::pair<int, reg_t>(j2, k)] = j;
                    for (reg_t l = 0; l < k; l++)
                      boost::add_edge(j - l - 1, j, con);
                    j++;
                  }
              }
          }
      }
  }

  // Get control flow graph from sdcc.
  for (ic = start_ic; ic; ic = ic->next)
    {
      wassertl (key_to_index[ic->key] < boost::num_vertices(cfg), "Node not in CFG.");

      if (ic->op != GOTO && ic->op != RETURN && ic->op != JUMPTABLE && ic->next)
        {
          wassertl (key_to_index[ic->next->key] < boost::num_vertices(cfg), "Next node not in CFG.");
          boost::add_edge(key_to_index[ic->key], key_to_index[ic->next->key], cfg);
        }

      if (ic->op == GOTO)
        {
          wassertl (key_to_index[eBBWithEntryLabel(ebbi, ic->label)->sch->key] < boost::num_vertices(cfg), "GOTO target not in CFG.");
          boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, ic->label)->sch->key], cfg);
        }
      else if (ic->op == RETURN)
        {
          wassertl (key_to_index[eBBWithEntryLabel(ebbi, returnLabel)->sch->key] < boost::num_vertices(cfg), "RETURN target not in CFG.");
          boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, returnLabel)->sch->key], cfg);
        }
      else if (ic->op == IFX)
        {
          wassertl (key_to_index[eBBWithEntryLabel(ebbi, IC_TRUE(ic) ? IC_TRUE(ic) : IC_FALSE(ic))->sch->key] < boost::num_vertices(cfg), "IFX target not in CFG.");
          boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, IC_TRUE(ic) ? IC_TRUE(ic) : IC_FALSE(ic))->sch->key], cfg);
        }
      else if (ic->op == JUMPTABLE)
        for (symbol *lbl = (symbol *)(setFirstItem (IC_JTLABELS (ic))); lbl; lbl = (symbol *)(setNextItem (IC_JTLABELS (ic))))
          {
            wassertl (key_to_index[eBBWithEntryLabel(ebbi, lbl)->sch->key] < boost::num_vertices(cfg), "GOTO target not in CFG.");
            boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, lbl)->sch->key], cfg);
          }

      for (int i = 0; i <= operandKey; i++)
        {
          if (sym_to_index.find(std::pair<int, reg_t>(i, 0)) == sym_to_index.end())
            continue;

          if (bitVectBitValue(ic->rlive, i))
            {
              symbol *isym = (symbol *)(hTabItemWithKey(liveRanges, i));
              for (reg_t k = 0; k < isym->nRegs; k++)
                {
                  wassert (key_to_index.find(ic->key) != key_to_index.end());
                  wassert (sym_to_index.find(std::pair<int, int>(i, k)) != sym_to_index.end());
                  wassertl (key_to_index[ic->key] < boost::num_vertices(cfg), "Node not in CFG.");
                  cfg[key_to_index[ic->key]].alive.insert(sym_to_index[std::pair<int, int>(i, k)]);
                }

              // TODO: Move this to a place where it also works when using the old allocator!
              if(isym->block)
                isym->block = btree_lowest_common_ancestor(isym->block, ic->block);
              else
                isym->block = ic->block;
            }
        }

      if (ic->op == IFX)
        add_operand_to_cfg_node(cfg[key_to_index[ic->key]], IC_COND(ic), sym_to_index);
      else if (ic->op == JUMPTABLE)
        add_operand_to_cfg_node(cfg[key_to_index[ic->key]], IC_JTCOND(ic), sym_to_index);
      else
        {
          add_operand_to_cfg_node(cfg[key_to_index[ic->key]], IC_RESULT(ic), sym_to_index);
          add_operand_to_cfg_node(cfg[key_to_index[ic->key]], IC_LEFT(ic), sym_to_index);
          add_operand_to_cfg_node(cfg[key_to_index[ic->key]], IC_RIGHT(ic), sym_to_index);
        }

      // TODO: Extend live-ranges of returns of built-in function calls back to first SEND.

      add_operand_conflicts_in_node(cfg[key_to_index[ic->key]], con);
    }

#if 0
  // Get conflict graph from sdcc
  for (var_t i = 0; static_cast<boost::graph_traits<cfg_t>::vertices_size_type>(i) < num_vertices(con); i++)
    {
      symbol *isym = (symbol *)(hTabItemWithKey(liveRanges, con[i].v));
      for (int j = 0; j <= operandKey; j++)
        if (bitVectBitValue(isym->clashes, j))
          {
            symbol *jsym = (symbol *)(hTabItemWithKey(liveRanges, j));
            if (sym_to_index.find(std::pair<int, reg_t>(j, 0)) == sym_to_index.end())
              continue;
            for (reg_t k = 0; k < jsym->nRegs; k++)
              boost::add_edge(i, sym_to_index[std::pair<int, reg_t>(j, k)], con);
          }
    }
#endif

  // Check for unconnected live ranges, some might have survived earlier stages.
  // This check is too expensive - Profiling shows that compiling the Dhrystone benchmark for stm8 with default options, we spend about a quarter of compiler runtime in here!
  // Profiling shows that we spend a significant amount of time on the first call to copy_graph()
  // Todo: Improve efficiency, e.g. using subgraph or filtered_graph to avoid the costly first call to copy_graph()
  for (var_t i = boost::num_vertices(con) - 1; i >= 0; i--)
    {
      cfg_sym_t cfg2;
      boost::copy_graph(cfg, cfg2, boost::vertex_copy(forget_properties()).edge_copy(forget_properties())); // This call to copy_graph is expensive!
      for (int j = boost::num_vertices(cfg) - 1; j >= 0; j--)
        {
          if (cfg[j].alive.find(i) == cfg[j].alive.end())
            {
              boost::clear_vertex(j, cfg2);
              boost::remove_vertex(j, cfg2);
            }
        }
      std::vector<boost::graph_traits<cfg_t>::vertices_size_type> component(num_vertices(cfg2));
      if (boost::connected_components(cfg2, &component[0]) > 1)
        {
          // Non-connected CFGs are created by at least GCSE and lospre. We now have a live-range splitter that fixes them, so this should no longer be necessary, but we leave this code here for now, so in case one gets through, we can still generate correct code.
          std::cerr << "Warning: Non-connected liverange found and extended to connected component of the CFG:" << con[i].name << ". Please contact sdcc authors with source code to reproduce.\n";
          
          cfg_sym_t cfg2;
          boost::copy_graph(cfg, cfg2, boost::vertex_copy(forget_properties()).edge_copy(forget_properties()));
          std::vector<boost::graph_traits<cfg_t>::vertices_size_type> component(num_vertices(cfg2));
          boost::connected_components(cfg2, &component[0]);

          for (boost::graph_traits<cfg_t>::vertices_size_type j = 0; j < boost::num_vertices(cfg) - 1; j++)
            {
              if(cfg[j].alive.find(i) != cfg[j].alive.end())
                {
                  for (boost::graph_traits<cfg_t>::vertices_size_type k = 0; k < boost::num_vertices(cfg) - 1; k++)
                    {
                      if (component[j] == component[k])
                        cfg[k].alive.insert(i);
                    }
                }
            }
        }
    }

  for (boost::graph_traits<cfg_t>::vertices_size_type i = 0; i < num_vertices(cfg); i++)
    {
      cfg[i].dying = cfg[i].alive;
      typedef boost::graph_traits<cfg_t>::adjacency_iterator adjacency_iter_t;
      adjacency_iter_t j, j_end;
      for (boost::tie(j, j_end) = adjacent_vertices(i, cfg); j != j_end; ++j)
        {
          std::set<var_t>::const_iterator v, v_end;
          for (v = cfg[*j].alive.begin(), v_end = cfg[*j].alive.end(); v != v_end; ++v)
            {
              const symbol *const vsym = (symbol *)(hTabItemWithKey(liveRanges, con[*v].v));

              const operand *const left = IC_LEFT(cfg[*j].ic);
              const operand *const right = IC_RIGHT(cfg[*j].ic);
              const operand *const result = IC_RESULT(cfg[*j].ic);

              if (!POINTER_SET(cfg[*j].ic) && 
                (!left || !IS_SYMOP(left) || OP_SYMBOL_CONST(left)->key != vsym->key) &&
                (!right || !IS_SYMOP(right) || OP_SYMBOL_CONST(right)->key != vsym->key) &&
                result && IS_SYMOP(result) && OP_SYMBOL_CONST(result)->key == vsym->key)
                continue;

              cfg[i].dying.erase(*v);
            }
        }
    }
    
  // Construct conflict graph
  for (boost::graph_traits<cfg_t>::vertices_size_type i = 0; i < num_vertices(cfg); i++)
    {
      std::set<var_t>::const_iterator v, v_end;
      const iCode *ic = cfg[i].ic;
      
      for (v = cfg[i].alive.begin(), v_end = cfg[i].alive.end(); v != v_end; ++v)
        {
          std::set<var_t>::const_iterator v2, v2_end;
          
          // Conflict between operands are handled by add_operand_conflicts_in_node().
          if (cfg[i].dying.find (*v) != cfg[i].dying.end())
            continue;
          if (ic->op != IFX && ic->op != JUMPTABLE && IC_RESULT(ic) && IS_SYMOP(IC_RESULT(ic)))
            {
              operand_map_t::const_iterator oi, oi_end; 
              for(boost::tie(oi, oi_end) = cfg[i].operands.equal_range(OP_SYMBOL_CONST(IC_RESULT(ic))->key); oi != oi_end; ++oi)
                if(oi->second == *v)
                  goto next_var;
            }
          
          // Here, v is a variable that survives cfg[i].
          // TODO: Check if we can use v, ++v2 instead of cfg[i].alive.begin() to speed things up.
          for (v2 = cfg[i].alive.begin(), v2_end = cfg[i].alive.end(); v2 != v2_end; ++v2)
            {
              if(*v == *v2)
                continue;
              if (cfg[i].dying.find (*v2) != cfg[i].dying.end())
                continue;

              boost::add_edge(*v, *v2, con);
            }
          
          next_var:
            ;
        }
    }

  return(start_ic);
}

// Computes live ranges for tree decomposition from live ranges from cfg.
inline void alive_tree_dec(tree_dec_t &tree_dec, const cfg_t &cfg)
{
  for (unsigned int i = 0; i < num_vertices(tree_dec); i++)
    {
      std::set<unsigned int>::const_iterator v;
      for (v = tree_dec[i].bag.begin(); v != tree_dec[i].bag.end(); ++v)
        tree_dec[i].alive.insert(cfg[*v].alive.begin(), cfg[*v].alive.end());
    }
}

#if defined(DEBUG_RALLOC_DEC) || defined (DEBUG_RALLOC_DEC_ASS)
static void print_assignment(const assignment &a)
{
  varset_t::const_iterator i;
  std::cout << "[";
  for (i = a.local.begin(); i != a.local.end(); ++i)
    std::cout << "(" << int(*i) << ", " << int(a.global[*i]) << "), ";
  std::cout << "c: " << a.s << "]";
}
#endif

template <class I_t>
bool assignment_conflict(const assignment &a, const I_t &I, var_t v, reg_t r)
{
  varset_t::const_iterator i, i_end;

  for (i = a.local.begin(), i_end = a.local.end(); i != i_end; ++i)
    {
      if (a.global[*i] != r)
        continue;
      if (boost::edge(*i, v, I).second)
        return(true);
    }
    
  return(false);
}

template<class G_t>
void assignments_introduce_instruction(assignment_list_t &alist, unsigned short int i, const G_t &G)
{
  assignment_list_t::iterator ai, ai_end;

  for (ai = alist.begin(), ai_end = alist.end(); ai != ai_end; ++ai)
    {
      std::set<var_t> i_variables;

      std::set_intersection(ai->local.begin(), ai->local.end(), G[i].alive.begin(), G[i].alive.end(), std::inserter(i_variables, i_variables.end()));

      i_assignment_t ia;

      std::set<var_t>::const_iterator v, v_end;
      for (v = i_variables.begin(), v_end = i_variables.end(); v != v_end; ++v)
        if (ai->global[*v] >= 0)
          ia.add_var(*v, ai->global[*v]);

      ai->i_assignment = ia;
    }
}

template <class G_t, class I_t>
static void assignments_introduce_variable(assignment_list_t &alist, unsigned short int i, short int v, const G_t &G, const I_t &I)
{
  assignment_list_t::iterator ai;
  bool a_initialized;
  assignment a;
  size_t c, c_end;

  for (ai = alist.begin(), c = 0, c_end = alist.size(); c < c_end; c++, ai++)
    {
      a_initialized = false;

      for (reg_t r = 0; r < port->num_regs; r++)
        {
          if (!assignment_conflict(*ai, I, v, r))
            {
              if(!a_initialized)
                {
                  a = *ai;
                  ai->marked = true;
                  a.marked = false;
                  a.local.insert(v);
                }
              a.global[v] = r;
              a.i_assignment.add_var(v, r);
              if(!assignment_hopeless(a, i, G, I, v))
                alist.push_back(a);
              a.i_assignment.remove_var(v);
            }
        }
    }
}

struct assignment_rep
{
  assignment_list_t::iterator i;
  float s;

  bool operator<(const assignment_rep& a) const
  {
    return(s < a.s);
  }
};

template <class I_t>
float compability_cost(const assignment& a, const assignment& ac, const I_t &I)
{
  typedef typename boost::graph_traits<I_t>::adjacency_iterator adjacency_iter_t;
  
  float c = 0.0f;
  
  varset_t::const_iterator vi, vi_end;
  
  for(vi = ac.local.begin(), vi_end = ac.local.end(); vi != vi_end; ++vi)
    {
      const var_t v = *vi;
      if(a.global[v] != ac.global[v])
      {
        c += 1000.0f;
        continue;
      }
#if 0 // This improves the quality of assignments, but it has a big runtime overhead for some cases.
      adjacency_iter_t j, j_end;
      for (boost::tie(j, j_end) = adjacent_vertices(v, I); j != j_end; ++j)
        if(ac.global[v] != -1 && a.global[*j] == ac.global[v])
        {
          c += 1000.0f;
          break;
        }
#endif
    }
  
  return(c);
}

// Ensure that we never get more than options.max_allocs_per_node assignments at a single node of the tree decomposition.
// Tries to drop the worst ones first (but never drop the empty assignment, as it's the only one guaranteed to be always valid).
template <class G_t, class I_t>
static void drop_worst_assignments(assignment_list_t &alist, unsigned short int i, const G_t &G, const I_t &I, const assignment& ac, bool *const assignment_optimal)
{
  unsigned int n;
  size_t alist_size;
  assignment_list_t::iterator ai, an;

  if ((alist_size = alist.size()) * port->num_regs <= static_cast<size_t>(options.max_allocs_per_node) || alist_size <= 1)
    return;

  *assignment_optimal = false;

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Too many assignments here (" << i << "):" << alist_size << " > " << options.max_allocs_per_node / port->num_regs << ". Dropping some.\n"; std::cout.flush();
#endif

  assignment_rep *arep = new assignment_rep[alist_size];

  for (n = 0, ai = alist.begin(); n < alist_size; ++ai, n++)
    {
      arep[n].i = ai;
      arep[n].s = ai->s + rough_cost_estimate(*ai, i, G, I) + compability_cost(*ai, ac, I);
    }

  std::nth_element(arep + 1, arep + options.max_allocs_per_node / port->num_regs, arep + alist_size);

  //std::cout << "nth elem. est. cost: " << arep[options.max_allocs_per_node / port->num_regs].s << "\n"; std::cout.flush();

  for (n = options.max_allocs_per_node / port->num_regs + 1; n < alist_size; n++)
    alist.erase(arep[n].i);
    
  delete[] arep;
}

// Handle Leaf nodes in the nice tree decomposition
template <class T_t, class G_t, class I_t>
static void tree_dec_ralloc_leaf(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G, const I_t &I)
{
#ifdef DEBUG_RALLOC_DEC
  std::cout << "Leaf (" << t << "):\n"; std::cout.flush();
#endif

  assignment a;
  assignment_list_t &alist = T[t].assignments;

  a.s = 0;
  a.global.resize(boost::num_vertices(I), -1);
  alist.push_back(a);
  
#ifdef DEBUG_RALLOC_DEC_ASS
  assignment_list_t::iterator ai;
  for(ai = alist.begin(); ai != alist.end(); ++ai)
    {
      print_assignment(*ai);
      std::cout << "\n";
    }
  assignment best;
  get_best_local_assignment(best, t, T);
  std::cout << "Best: "; print_assignment(best); std::cout << "\n";
#endif
}

// Handle introduce nodes in the nice tree decomposition
template <class T_t, class G_t, class I_t>
static void tree_dec_ralloc_introduce(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G, const I_t &I, const assignment& ac, bool *const assignment_optimal)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  assignment_list_t::iterator ai;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Introduce (" << t << "):\n"; std::cout.flush();
  std::cout << "ac: "; print_assignment(ac); std::cout << "\n";
#endif

  assignment_list_t &alist = T[t].assignments;

  std::swap(alist, T[*c].assignments);

  std::set<var_t> new_vars;
  std::set_difference(T[t].alive.begin(), T[t].alive.end(), T[*c].alive.begin(), T[*c].alive.end(), std::inserter(new_vars, new_vars.end()));

  std::set<unsigned short> new_inst;
  std::set_difference(T[t].bag.begin(), T[t].bag.end(), T[*c].bag.begin(), T[*c].bag.end(), std::inserter(new_inst, new_inst.end()));
  unsigned short int i = *(new_inst.begin());

  // Extend to new instruction.
  assignments_introduce_instruction(alist, i, G);

  std::set<var_t>::const_iterator v;
  for (v = new_vars.begin(); v != new_vars.end(); ++v)
    {
      drop_worst_assignments(alist, i, G, I, ac, assignment_optimal);
      assignments_introduce_variable(alist, i, *v, G, I);
    }

  // Summation of costs and early removal of assignments.
  for (ai = alist.begin(); ai != alist.end();)
    {
      if ((ai->s += (ai->i_costs[i] = instruction_cost(*ai, i, G, I))) == std::numeric_limits<float>::infinity())
        ai = alist.erase(ai);
      else
        ++ai;
    }

  // Free memory in the std::set<var_t, boost::pool_allocator<var_t> > that live in the assignments in the list.
  //boost::singleton_pool<boost::fast_pool_allocator_tag, sizeof(var_t)>::release_memory();

#ifdef DEBUG_RALLOC_DEC_ASS
  for(ai = alist.begin(); ai != alist.end(); ++ai)
    {
      print_assignment(*ai);
      std::cout << "\n";
    }

  assignment best;
  get_best_local_assignment(best, t, T);
  std::cout << "Best: "; print_assignment(best); std::cout << "\n";
#endif
}

static bool assignments_locally_same(const assignment &a1, const assignment &a2)
{
  if (a1.local != a2.local)
    return(false);

  varset_t::const_iterator i, i_end;
  for (i = a1.local.begin(), i_end = a1.local.end(); i != i_end; ++i)
    if (a1.global[*i] != a2.global[*i])
      return(false);

  return(true);
}

// Handle forget nodes in the nice tree decomposition
template <class T_t, class G_t, class I_t>
static void tree_dec_ralloc_forget(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G, const I_t &I)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Forget (" << t << "):\n"; std::cout.flush();
#endif

  assignment_list_t &alist = T[t].assignments;

  std::swap(alist, T[*c].assignments);

  std::set<unsigned short int> old_inst;
  std::set_difference(T[*c].bag.begin(), T[*c].bag.end(), T[t].bag.begin(), T[t].bag.end(), std::inserter(old_inst, old_inst.end()));
  unsigned short int i = *(old_inst.begin());

  std::set<var_t> old_vars;
  std::set_difference(T[*c].alive.begin(), T[*c].alive.end(), T[t].alive.begin(), T[t].alive.end(), std::inserter(old_vars, old_vars.end()));

  assignment_list_t::iterator ai, aif;

  // Restrict assignments (locally) to current variables.
  for (ai = alist.begin(); ai != alist.end(); ++ai)
    {
      // Erasing by iterators doesn't work with B-Trees, and erasing by value invalidates iterators.
      std::set<var_t>::const_iterator oi, oi_end;
      for (oi = old_vars.begin(), oi_end = old_vars.end(); oi != oi_end; ++oi)
        ai->local.erase(*oi);

      ai->i_costs.erase(i);
    }

  alist.sort();
  //std::sort(alist.begin(), alist.end());

  // Collapse (locally) identical assignments.
  for (ai = alist.begin(); ai != alist.end();)
    {
      aif = ai;

      for (++ai; ai != alist.end() && assignments_locally_same(*aif, *ai);)
        {
          if (aif->s > ai->s)
            {
              alist.erase(aif);
              aif = ai;
              ++ai;
            }
          else
            {
              alist.erase(ai);
              ai = aif;
              ++ai;
            }
        }
    }

  // Free memory in the std::set<var_t, boost::pool_allocator<var_t> > that live in the assignments in the list.
  //boost::singleton_pool<boost::fast_pool_allocator_tag, sizeof(var_t)>::release_memory();

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Remaining assignments: " << alist.size() << "\n"; std::cout.flush();
#endif

#ifdef DEBUG_RALLOC_DEC_ASS
  for(ai = alist.begin(); ai != alist.end(); ++ai)
    {
      print_assignment(*ai);
      std::cout << "\n";
    }
  
  assignment best;
  get_best_local_assignment(best, t, T);
  std::cout << "Best: "; print_assignment(best); std::cout << "\n";
#endif
}

// Handle join nodes in the nice tree decomposition
template <class T_t, class G_t, class I_t>
static void tree_dec_ralloc_join(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G, const I_t &I)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end, c2, c3;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Join (" << t << "):\n"; std::cout.flush();
#endif

  c2 = c;
  ++c;
  c3 = c;

  assignment_list_t &alist1 = T[t].assignments;
  assignment_list_t &alist2 = T[*c2].assignments;
  assignment_list_t &alist3 = T[*c3].assignments;

  alist2.sort();
  //std::sort(alist2.begin(), alist2.end());
  alist3.sort();
  //std::sort(alist3.begin(), alist3.end());

  assignment_list_t::iterator ai2, ai3;
  for (ai2 = alist2.begin(), ai3 = alist3.begin(); ai2 != alist2.end() && ai3 != alist3.end();)
    {
      if (assignments_locally_same(*ai2, *ai3))
        {
          ai2->s += ai3->s;
          // Avoid double-counting instruction costs.
          std::set<unsigned int>::iterator bi;
          for (bi = T[t].bag.begin(); bi != T[t].bag.end(); ++bi)
            ai2->s -= ai2->i_costs[*bi];
          for (size_t i = 0; i < ai2->global.size(); i++)
            ai2->global[i] = ((ai2->global[i] != -1) ? ai2->global[i] : ai3->global[i]);
          alist1.push_back(*ai2);
          ++ai2;
          ++ai3;
        }
      else if (*ai2 < *ai3)
        {
          ++ai2;
          continue;
        }
      else if (*ai3 < *ai2)
        {
          ++ai3;
          continue;
        }
    }

  alist2.clear();
  alist3.clear();

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Remaining assignments: " << alist1.size() << "\n"; std::cout.flush();
#endif

#ifdef DEBUG_RALLOC_DEC_ASS
  std::list<assignment>::iterator ai;
  for(ai = alist1.begin(); ai != alist1.end(); ++ai)
    {
  	  print_assignment(*ai);
      std::cout << "\n";
    }
#endif
}

template <class T_t>
void get_best_local_assignment(assignment &a, typename boost::graph_traits<T_t>::vertex_descriptor t, const T_t &T)
{
  const assignment_list_t &alist = T[t].assignments;

  assignment_list_t::const_iterator ai, ai_end, ai_best;
  for(ai = ai_best = alist.begin(), ai_end = alist.end(); ai != ai_end; ++ai)
    if(ai->s < ai_best->s)
      ai_best = ai;
	
  a = *ai_best;
}

// Handle nodes in the tree decomposition, by detecting their type and calling the appropriate function. Recurses.
template <class T_t, class G_t, class I_t>
static void tree_dec_ralloc_nodes(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G, const I_t &I, const assignment& ac, bool *const assignment_optimal)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;

  adjacency_iter_t c, c_end;
  typename boost::graph_traits<T_t>::vertex_descriptor c0, c1;

  boost::tie(c, c_end) = adjacent_vertices(t, T);

  switch (out_degree(t, T))
    {
    case 0:
      tree_dec_ralloc_leaf(T, t, G, I);
      break;
    case 1:
      c0 = *c;
      tree_dec_ralloc_nodes(T, c0, G, I, ac, assignment_optimal);
      T[c0].bag.size() < T[t].bag.size() ? tree_dec_ralloc_introduce(T, t, G, I, ac, assignment_optimal) : tree_dec_ralloc_forget(T, t, G, I);
      break;
    case 2:
      c0 = *c++;
      c1 = *c;

      if (T[c0].weight < T[c1].weight) // Minimize memory consumption needed for keeping intermediate results. As a side effect, this also helps the ac mechanism in the heuristic.
        std::swap (c0, c1);

      tree_dec_ralloc_nodes(T, c0, G, I, ac, assignment_optimal);
        {
          assignment *ac2 = new assignment;
          get_best_local_assignment_biased(*ac2, c0, T);
          tree_dec_ralloc_nodes(T, c1, G, I, *ac2, assignment_optimal);
          delete ac2;
        }
      tree_dec_ralloc_join(T, t, G, I);
      break;
    default:
      std::cerr << "Not nice.\n";
      break;
    }
}

// Find the best root selecting from t_old and the leafs under t.
template <class T_t>
static std::pair<typename boost::graph_traits<T_t>::vertex_descriptor, size_t> find_best_root(const T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, size_t t_s, typename boost::graph_traits<T_t>::vertex_descriptor t_old, size_t t_old_s)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  typename boost::graph_traits<T_t>::vertex_descriptor c0, c1, t0;
  size_t t0_s;

  boost::tie(c, c_end) = adjacent_vertices(t, T);

  switch (out_degree(t, T))
    {
    case 0:
      return(t_s > t_old_s ? std::pair<typename boost::graph_traits<T_t>::vertex_descriptor, size_t>(t, t_s) : std::pair<typename boost::graph_traits<T_t>::vertex_descriptor, size_t>(t_old, t_old_s));
    case 1:
      return(find_best_root(T, *c, T[*c].alive.size() ? T[*c].alive.size() : t_s, t_old, t_old_s));
    case 2:
      c0 = *c++;
      c1 = *c;
      boost::tie(t0, t0_s) = find_best_root(T, c0, T[c0].alive.size() ? T[c0].alive.size() : t_s, t_old, t_old_s);
      return(find_best_root(T, c1, T[c1].alive.size() ? T[c1].alive.size() : t_s, t0_s > t_old_s ? t0 : t_old, t0_s > t_old_s ? t0_s : t_old_s));
      break;
    default:
      std::cerr << "Not nice.\n";
      break;
    }

  return(std::pair<typename boost::graph_traits<T_t>::vertex_descriptor, size_t>(t_old, t_old_s));
}

// Change the root to t.
template <class T_t>
static void re_root(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t)
{
  typename boost::graph_traits<T_t>::vertex_descriptor s0, s1, s2;
  typename boost::graph_traits<T_t>::in_edge_iterator e, e_end;

  boost::tie(e, e_end) = boost::in_edges(t, T);
  if (e == e_end)
    return;

  s0 = t;
  s1 = boost::source(*e, T);

  for (boost::tie(e, e_end) = boost::in_edges(s1, T); e != e_end; boost::tie(e, e_end) = boost::in_edges(s1, T))
    {
      s2 = boost::source(*e, T);
      boost::remove_edge(s1, s0, T);
      boost::add_edge(s0, s1, T);
      s0 = s1;
      s1 = s2;
    }
  boost::remove_edge(s1, s0, T);
  boost::add_edge(s0, s1, T);
}

// Change the root to improve the assignment removal heuristic.
template <class T_t>
static void good_re_root(T_t &T)
{
  typename boost::graph_traits<T_t>::vertex_descriptor t;

  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;

  t = find_root(T);

  for (boost::tie(c, c_end) = boost::adjacent_vertices(t, T); c != c_end && !T[*c].alive.size();)
    boost::tie(c, c_end) = boost::adjacent_vertices(*c, T);

  size_t t_s = (c != c_end ? T[*c].alive.size() : 0);
  t = find_best_root(T, t, t_s, t, t_s).first;

  if (T[t].alive.size())
    {
      std::cerr << "Error: Invalid root.\n";
      return;
    }

  re_root(T, t);
}

// Dump conflict graph, with numbered nodes, show live variables at each node.
static void dump_con(const con_t &con)
{
  std::ofstream dump_file((std::string(dstFileName) + ".dumpcon" + currFunc->rname + ".dot").c_str());

  std::string *name = new std::string[num_vertices(con)];
  for (var_t i = 0; static_cast<boost::graph_traits<cfg_t>::vertices_size_type>(i) < boost::num_vertices(con); i++)
    {
      std::ostringstream os;
      os << i;
      if (con[i].name)
        os << " : " << con[i].name << ":" << con[i].byte;
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, con, boost::make_label_writer(name));
  delete[] name;
}

// Dump cfg, with numbered nodes, show live variables at each node.
static void dump_cfg(const cfg_t &cfg)
{
  std::ofstream dump_file((std::string(dstFileName) + ".dumpcfg" + currFunc->rname + ".dot").c_str());

  std::string *name = new std::string[num_vertices(cfg)];
  for (unsigned int i = 0; i < boost::num_vertices(cfg); i++)
    {
      std::ostringstream os;
      os << i << ", " << cfg[i].ic->key << ": ";
      std::set<var_t>::const_iterator v;
      for (v = cfg[i].alive.begin(); v != cfg[i].alive.end(); ++v)
        os << *v << " ";
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, cfg, boost::make_label_writer(name));
  delete[] name;
}

// Dump tree decomposition, show bag and live variables at each node.
static void dump_tree_decomposition(const tree_dec_t &tree_dec)
{
  std::ofstream dump_file((std::string(dstFileName) + ".dumpdec" + currFunc->rname + ".dot").c_str());

  unsigned int w = 0;

  std::string *name = new std::string[num_vertices(tree_dec)];
  for (unsigned int i = 0; i < boost::num_vertices(tree_dec); i++)
    {
      if (tree_dec[i].bag.size() > w)
        w = tree_dec[i].bag.size();
      std::ostringstream os;
      std::set<unsigned int>::const_iterator v1;
      os << i << " | ";
      for (v1 = tree_dec[i].bag.begin(); v1 != tree_dec[i].bag.end(); ++v1)
        os << *v1 << " ";
      os << ": ";
      std::set<var_t>::const_iterator v2;
      for (v2 = tree_dec[i].alive.begin(); v2 != tree_dec[i].alive.end(); ++v2)
        os << *v2 << " ";
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, tree_dec, boost::make_label_writer(name));
  delete[] name;

#ifdef D_RALLOC_DEC
  std::cout << "Width: " << (w  - 1) << "(" << currFunc->name << ")\n";
#endif
}

#endif

