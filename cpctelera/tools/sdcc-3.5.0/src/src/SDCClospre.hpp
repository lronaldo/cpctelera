// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2012
//
// (c) 2012 Goethe-Universität Frankfurt
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
//
// Lifetime-optimal speculative partial redundancy elimination.

// Workaround for boost bug #11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
   #include <boost/type_traits/ice.hpp>
#endif

#include <boost/graph/graphviz.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include "SDCCtree_dec.hpp"

extern "C"
{
#include "SDCCsymt.h"
#include "SDCCicode.h"
#include "SDCCgen.h"
#include "SDCCBBlock.h"
#include "SDCCopt.h"
#include "SDCCy.h"
#include "SDCCasm.h"
#include "port.h"
}

#ifdef HAVE_STX_BTREE_SET_H
#include <stx/btree_set.h>
#endif

#if 0 //def HAVE_STX_BTREE_SET_H
typedef stx::btree_set<unsigned short int> lospreset_t; // Faster than std::set
#else
typedef std::set<unsigned short int> lospreset_t;
#endif

struct assignment_lospre
{
  boost::tuple<float, float> s; // First entry: Calculation costs, second entry: Lifetime costs.
  lospreset_t local;
  std::vector<bool> global;

  bool operator<(const assignment_lospre& a) const
  {
    lospreset_t::const_iterator i, ai, i_end, ai_end;

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

bool assignments_lospre_locally_same(const assignment_lospre &a1, const assignment_lospre &a2)
{
  if (a1.local != a2.local)
    return(false);

  lospreset_t::const_iterator i, i_end;
  for (i = a1.local.begin(), i_end = a1.local.end(); i != i_end; ++i)
    if (a1.global[*i] != a2.global[*i])
      return(false);

  return(true);
}

struct cfg_lospre_node
{
  iCode *ic;

  bool uses;
  bool invalidates;

  std::pair<int, int> forward;
};

typedef std::list<assignment_lospre> assignment_list_lospre_t;

struct tree_dec_lospre_node
{
  std::set<unsigned int> bag;
  assignment_list_lospre_t assignments;
  unsigned weight; // The weight is the number of nodes at which intermediate results need to be remembered. In general, to minimize memory consumption, at join nodes the child with maximum weight should be processed first.
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, cfg_lospre_node, float> cfg_lospre_t; // The edge property is the cost of subdividing the edge and inserting an instruction (for now we always use 1, optimizing for code size, but relative execution frequency could be used when optimizing for speed or total energy consumption; aggregates thereof can be a good idea as well).
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, tree_dec_lospre_node> tree_dec_lospre_t;

#if 1
void print_assignment(const assignment_lospre &a, cfg_lospre_t G)
{
  wassert(a.global.size() == boost::num_vertices (G));
  for(unsigned int i = 0; i < boost::num_vertices (G); i++)
    std::cout << "(" << i << ", " << a.global[i] << "),";
  std::cout << "\n";
  std::cout << "Cost: " << a.s << "\nLocal:";
  for(lospreset_t::const_iterator i = a.local.begin(); i != a.local.end(); ++i)
    std::cout << *i << " ";
  std::cout << "\n";
  std::cout.flush();
}
#endif

// Handle Leaf nodes in the nice tree decomposition
template <class T_t, class G_t>
void tree_dec_lospre_leaf(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  assignment_lospre a;
  assignment_list_lospre_t &alist = T[t].assignments;

  a.s.get<0>() = 0;
  a.s.get<1>() = 0;
  a.global.resize(boost::num_vertices(G));
  alist.push_back(a);
}

// Handle introduce nodes in the nice tree decomposition
template <class T_t, class G_t>
int tree_dec_lospre_introduce(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  assignment_list_lospre_t::iterator ai;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

  assignment_list_lospre_t &alist2 = T[t].assignments;
  assignment_list_lospre_t &alist = T[*c].assignments;

  if(alist.size() > size_t(options.max_allocs_per_node) / 2)
    {
      alist.clear();
      return(-1);
    }

  std::set<unsigned short> new_inst;
  std::set_difference(T[t].bag.begin(), T[t].bag.end(), T[*c].bag.begin(), T[*c].bag.end(), std::inserter(new_inst, new_inst.end()));
  unsigned short int i = *(new_inst.begin());

  for(ai = alist.begin(); ai != alist.end(); ++ai)
    {
      ai->local.insert(i);
      ai->global[i] = false;
      alist2.push_back(*ai);
      ai->global[i] = true;
      alist2.push_back(*ai);
    }

  alist.clear();

  return(0);
}

// Handle forget nodes in the nice tree decomposition
template <class T_t, class G_t>
void tree_dec_lospre_forget(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

  assignment_list_lospre_t &alist = T[t].assignments;

  std::swap(alist, T[*c].assignments);

  std::set<unsigned short int> old_inst;
  std::set_difference(T[*c].bag.begin(), T[*c].bag.end(), T[t].bag.begin(), T[t].bag.end(), std::inserter(old_inst, old_inst.end()));
  unsigned short int i = *(old_inst.begin());

  assignment_list_lospre_t::iterator ai, aif;

  for (ai = alist.begin(); ai != alist.end(); ++ai)
    {
      ai->local.erase(i);
      ai->s.get<1>() += ai->global[i]; // Add lifetime cost.
      {
        typedef typename boost::graph_traits<cfg_lospre_t>::out_edge_iterator n_iter_t;
        n_iter_t n, n_end;
        for (boost::tie(n, n_end) = boost::out_edges(i, G);  n != n_end; ++n)
          {
            if (ai->local.find(boost::target(*n, G)) == ai->local.end() || (ai->global[i] && !G[i].invalidates) >= (ai->global[boost::target(*n, G)] || G[boost::target(*n, G)].uses))
              continue;

            ai->s.get<0>() += G[*n]; // Add calculation cost.
          }
      }
      {
        typedef typename boost::graph_traits<cfg_lospre_t>::in_edge_iterator n_iter_t;
        n_iter_t n, n_end;
        for (boost::tie(n, n_end) = boost::in_edges(i, G);  n != n_end; ++n)
          {
            if (ai->local.find(boost::source(*n, G)) == ai->local.end() || (ai->global[boost::source(*n, G)] && !G[boost::source(*n, G)].invalidates) >= (ai->global[i] || G[i].uses))
              continue;

            ai->s.get<0>() += G[*n]; // Add calculation cost.
          }
      }
    }

  alist.sort();

  // Collapse (locally) identical assignments.
  for (ai = alist.begin(); ai != alist.end();)
    {
      aif = ai;

      for (++ai; ai != alist.end() && assignments_lospre_locally_same(*aif, *ai);)
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

  if(!alist.size())
    std::cerr << "No surviving assignments at forget node (lospre).\n";
}

// Handle join nodes in the nice tree decomposition
template <class T_t, class G_t>
void tree_dec_lospre_join(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end, c2, c3;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

  c2 = c;
  ++c;
  c3 = c;

  assignment_list_lospre_t &alist1 = T[t].assignments;
  assignment_list_lospre_t &alist2 = T[*c2].assignments;
  assignment_list_lospre_t &alist3 = T[*c3].assignments;

  alist2.sort();
  alist3.sort();

  assignment_list_lospre_t::iterator ai2, ai3;
  for (ai2 = alist2.begin(), ai3 = alist3.begin(); ai2 != alist2.end() && ai3 != alist3.end();)
    {
      if (assignments_lospre_locally_same(*ai2, *ai3))
        {
          ai2->s.get<0>() += ai3->s.get<0>();
          ai2->s.get<1>() += ai3->s.get<1>();
          for (size_t i = 0; i < ai2->global.size(); i++)
            ai2->global[i] = (ai2->global[i] || ai3->global[i]);
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
}

template <class T_t, class G_t>
int tree_dec_lospre_nodes(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;

  adjacency_iter_t c, c_end;
  typename boost::graph_traits<T_t>::vertex_descriptor c0, c1;

  boost::tie(c, c_end) = adjacent_vertices(t, T);

  switch (out_degree(t, T))
    {
    case 0:
      tree_dec_lospre_leaf(T, t, G);
      break;
    case 1:
      c0 = *c;
      if(tree_dec_lospre_nodes(T, c0, G) < 0)
        return(-1);
      if (T[c0].bag.size() < T[t].bag.size())
        {
        if (tree_dec_lospre_introduce(T, t, G))
          return(-1);
        }
      else
        tree_dec_lospre_forget(T, t, G);
      break;
    case 2:
      c0 = *c++;
      c1 = *c;
      if(tree_dec_lospre_nodes(T, c0, G) < 0)
        return(-1);
      if(tree_dec_lospre_nodes(T, c1, G) < 0)
        {
          T[c0].assignments.clear();
          return(-1);
        }
      tree_dec_lospre_join(T, t, G);
      break;
    default:
      std::cerr << "Not nice.\n";
      break;
    }
  return(0);
}

template <class T_t, class G_t>
void tree_dec_safety_forget(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;
  adjacency_iter_t c, c_end;
  boost::tie(c, c_end) = adjacent_vertices(t, T);

  assignment_list_lospre_t &alist = T[t].assignments;

  std::swap(alist, T[*c].assignments);

  std::set<unsigned short int> old_inst;
  std::set_difference(T[*c].bag.begin(), T[*c].bag.end(), T[t].bag.begin(), T[t].bag.end(), std::inserter(old_inst, old_inst.end()));
  unsigned short int i = *(old_inst.begin());

  assignment_list_lospre_t::iterator ai, aif;

  for(ai = alist.begin(); ai != alist.end();)
    {
      ai->local.erase(i);

      if (!ai->global[i])
        {
          ++ai;
          continue;
        }

      // Since we want the union of all paths between invalidating nodes without uses,
      // by definition there may not be a use in it.
      if (G[i].uses)
        {
          ai = alist.erase(ai);
          continue;
        }

      ai->s.get<1>() -= 1; // Maximize the subsets: Find all paths

      // At least one successor needs to be in the path or invalid.
      {
        typedef typename boost::graph_traits<cfg_lospre_t>::out_edge_iterator n_iter_t;
        n_iter_t n, n_end;
        bool ok;

        for (ok = false, boost::tie(n, n_end) = boost::out_edges(i, G);  !ok && n != n_end; ++n)
          if (ai->global[boost::target(*n, G)] || G[boost::target(*n, G)].invalidates)
            ok = true;

        if(!ok)
          {
            ai = alist.erase(ai);
            continue;
          }
      }
      // At least one predecessor needs to be in the path or invalid.
      {
        typedef typename boost::graph_traits<cfg_lospre_t>::in_edge_iterator n_iter_t;
        n_iter_t n, n_end;
        bool ok;

        for (ok = false, boost::tie(n, n_end) = boost::in_edges(i, G);  !ok && n != n_end; ++n)
          if (ai->global[boost::source(*n, G)] || G[boost::source(*n, G)].invalidates)
            ok = true;

        if(!ok)
          {
            ai = alist.erase(ai);
            continue;
          }
      }

      ++ai;
    }

  alist.sort();

  // Collapse (locally) identical assignments.
  for(ai = alist.begin(); ai != alist.end();)
    {
      aif = ai;

      for(++ai; ai != alist.end() && assignments_lospre_locally_same(*aif, *ai);)
        {
          if(aif->s > ai->s)
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

  if(!alist.size())
    std::cerr << "No surviving assignments at forget node.\n";
}

template <class T_t, class G_t>
int tree_dec_safety_nodes(T_t &T, typename boost::graph_traits<T_t>::vertex_descriptor t, const G_t &G)
{
  typedef typename boost::graph_traits<T_t>::adjacency_iterator adjacency_iter_t;

  adjacency_iter_t c, c_end;
  typename boost::graph_traits<T_t>::vertex_descriptor c0, c1;

  boost::tie(c, c_end) = adjacent_vertices(t, T);

  switch (out_degree(t, T))
    {
    case 0:
      tree_dec_lospre_leaf(T, t, G);
      break;
    case 1:
      c0 = *c;
      if(tree_dec_safety_nodes(T, c0, G) < 0)
        return(-1);
      if (T[c0].bag.size() < T[t].bag.size())
        {
        if (tree_dec_lospre_introduce(T, t, G))
          return(-1);
        }
      else
        tree_dec_safety_forget(T, t, G);
      break;
    case 2:
      c0 = *c++;
      c1 = *c;

      if (T[c0].weight < T[c1].weight) // Minimize memory consumption.
        std::swap (c0, c1);

      if(tree_dec_safety_nodes(T, c0, G) < 0)
        return(-1);
      if(tree_dec_safety_nodes(T, c1, G) < 0)
        {
          T[c0].assignments.clear();
          return(-1);
        }
      tree_dec_lospre_join(T, t, G);
      break;
    default:
      std::cerr << "Not nice.\n";
      break;
    }
  return(0);
}

template <class T_t, class G_t>
static void split_edge(T_t &T, G_t &G, typename boost::graph_traits<G_t>::edge_descriptor e, const iCode *ic, operand *tmpop)
{
  // Insert new iCode into chain.
  iCode *newic = newiCode (ic->op, IC_LEFT (ic), IC_RIGHT (ic));
  IC_RESULT(newic) = tmpop;
  newic->filename = ic->filename;
  newic->lineno = ic->lineno;
  newic->prev = G[boost::source(e, G)].ic;
  newic->next = G[boost::target(e, G)].ic;
  G[boost::source(e, G)].ic->next = newic;
  G[boost::target(e, G)].ic->prev = newic;

  //if (ic->op != ADDRESS_OF && IC_LEFT (ic) && IS_ITEMP (IC_LEFT (ic)))
  //  bitVectSetBit (OP_SYMBOL (IC_LEFT (ic))->uses, ic->key);
  //if (IC_RIGHT (ic) && IS_ITEMP (IC_RIGHT (ic)))
  //  bitVectSetBit (OP_SYMBOL (IC_RIGHT (ic))->uses, ic->key);
  //bitVectSetBit (OP_SYMBOL (IC_RESULT (ic))->defs, ic->key);

  // Insert node into cfg.
  typename boost::graph_traits<G_t>::vertex_descriptor n = boost::add_vertex(G);
  // TODO: Exact cost.
  G[n].ic = newic;
  G[n].uses = false;
  boost::add_edge(boost::source(e, G), n, G[e], G);
  boost::add_edge(n, boost::target(e, G), 3.0, G);

#ifdef DEBUG_LOSPRE
  std::cout << "Calculating " << OP_SYMBOL_CONST(tmpop)->name << " at ic " << newic->key << "\n";
#endif

  // Update tree-decomposition.
  // TODO: More efficiently.
  for(typename boost::graph_traits<T_t>::vertex_descriptor n1 = 0; n1 < boost::num_vertices(T); ++n1)
    {
      if(T[n1].bag.find(boost::source(e, G)) == T[n1].bag.end())
        continue;
      if(T[n1].bag.find(boost::target(e, G)) == T[n1].bag.end())
        continue;
      // Found bag that contains both endpoints of original edge.

      // Add new tree node with bag there. Let nicify() sort things out later.
      typename boost::graph_traits<T_t>::vertex_descriptor n2 = boost::add_vertex(T);
      T[n2].bag.insert(boost::source(e, G));
      T[n2].bag.insert(boost::target(e, G));
      T[n2].bag.insert(n);
      boost::add_edge(n1, n2, T);
      break;
    }

  // Remove old edge from cfg.
  boost::remove_edge(e, G);
}


template <class G_t>
static void forward_lospre_assignment(G_t &G, typename boost::graph_traits<G_t>::vertex_descriptor i, const iCode *ic, const assignment_lospre& a)
{
  typedef typename boost::graph_traits<G_t>::adjacency_iterator adjacency_iter_t;

  adjacency_iter_t c, c_end;

  operand *tmpop = IC_RIGHT(ic);
  const std::pair<int, int> forward(IC_RESULT(ic)->key, IC_RIGHT(ic)->key);

  for(;;)
    {
      if (G[i].forward == forward)
        break; // Was here before.

      iCode *nic = G[i].ic;

      if (isOperandEqual(IC_RESULT(ic), IC_LEFT(nic)) && nic->op != ADDRESS_OF && (!POINTER_GET(nic) || !IS_PTR(operandType(IC_RESULT(nic))) || !IS_BITFIELD(operandType(IC_LEFT(nic))->next) || compareType(operandType(IC_LEFT(nic)), operandType(tmpop)) == 1))
        {
          bool isaddr = IC_LEFT (nic)->isaddr;
#ifdef DEBUG_LOSPRE
          std::cout << "Forward substituted left operand " << OP_SYMBOL_CONST(IC_LEFT(nic))->name << " at " << nic->key << "\n";
#endif
          //bitVectUnSetBit (OP_SYMBOL (IC_LEFT (nic))->uses, nic->key);
          IC_LEFT(nic) = operandFromOperand (tmpop);
          //bitVectSetBit (OP_SYMBOL (IC_LEFT (nic))->uses, nic->key);
          IC_LEFT (nic)->isaddr = isaddr;
        }
      if (isOperandEqual(IC_RESULT(ic), IC_RIGHT(nic)))
        {
#ifdef DEBUG_LOSPRE
          std::cout << "Forward substituted right operand " << OP_SYMBOL_CONST(IC_RIGHT(nic))->name << " at " << nic->key << "\n";
#endif
          //bitVectUnSetBit (OP_SYMBOL (IC_RIGHT (nic))->uses, nic->key);
          IC_RIGHT(nic) = operandFromOperand (tmpop);
          //bitVectSetBit (OP_SYMBOL (IC_RIGHT (nic))->uses, nic->key);
        }
      if (POINTER_SET(nic) && isOperandEqual(IC_RESULT(ic), IC_RESULT(nic)) && (!IS_PTR(operandType(IC_RESULT(nic))) || !IS_BITFIELD(operandType(IC_RESULT(nic))->next) || compareType(operandType(IC_RESULT(nic)), operandType(tmpop)) == 1))
        {
#ifdef DEBUG_LOSPRE
          std::cout << "Forward substituted result operand " << OP_SYMBOL_CONST(IC_RESULT(nic))->name << " at " << nic->key << "\n";
#endif
          //bitVectUnSetBit (OP_SYMBOL (IC_RESULT (nic))->uses, nic->key);
          IC_RESULT(nic) = operandFromOperand (tmpop);
          IC_RESULT(nic)->isaddr = true;
          //bitVectSetBit (OP_SYMBOL (IC_RESULT (nic))->uses, nic->key);
        }

      if (nic->op == LABEL) // Reached label. Continue only if all edges goining here are safe.
        {
          typedef typename boost::graph_traits<G_t>::in_edge_iterator in_edge_iter_t;
          in_edge_iter_t e, e_end;
          for (boost::tie(e, e_end) = boost::in_edges(i, G); e != e_end; ++e)
            if (G[boost::source(*e, G)].forward != forward)
              break;
          if(e != e_end)
            break;
        }
      if (isOperandEqual(IC_RESULT (ic), IC_RESULT(nic)) && !POINTER_SET(nic) /*|| G[i].uses*/)
        break;
      if ((nic->op == CALL || nic->op == PCALL || POINTER_SET(nic)) && IS_TRUE_SYMOP(IC_RESULT(ic)))
        break;

      G[i].forward = forward;

      if (nic->op == GOTO || nic->op == IFX || nic->op == JUMPTABLE)
        {
          adjacency_iter_t c, c_end;
          for(boost::tie(c, c_end) = boost::adjacent_vertices(i, G); c != c_end; ++c)
            {
              if(!((a.global[i] & true) && !G[i].invalidates) && (a.global[*c] & true)) // Calculation edge
                continue;
              forward_lospre_assignment(G, *c, ic, a);
            }
          break;
        }

      boost::tie(c, c_end) = adjacent_vertices(i, G);
      if(c == c_end)
        break;
      if(!((a.global[i] & true) && !G[i].invalidates) && (a.global[*c] & true)) // Calculation edge
        break;
      i = *c;
    }
}

template <class T_t, class G_t>
static int implement_lospre_assignment(const assignment_lospre a, T_t &T, G_t &G, const iCode *ic) // Assignment has to be passed as a copy (not reference), since the transformations on the tree-decomposition will invalidate it otherwise.
{
  operand *tmpop;
  unsigned substituted = 0, split = 0;

  typedef typename boost::graph_traits<G_t>::edge_iterator edge_iter_t;
  typedef typename boost::graph_traits<G_t>::edge_descriptor edge_desc_t;
  std::set<edge_desc_t> calculation_edges; // Use descriptor, not iterator due to possible invalidation of iterators when inserting vertices or edges.
  edge_iter_t e, e_end;
  for(boost::tie(e, e_end) = boost::edges(G); e != e_end; ++e)
    if(!((a.global[boost::source(*e, G)] & true) && !G[boost::source(*e, G)].invalidates) && (a.global[boost::target(*e, G)] & true))
      calculation_edges.insert(*e);

  if(!calculation_edges.size())
    return(0);

#ifdef DEBUG_LOSPRE
  std::cout << "Optimizing at " << ic->key << "\n"; std::cout.flush();
#endif

  tmpop = newiTempOperand (operandType (IC_RESULT (ic)), TRUE);
  tmpop->isvolatile = false;
#ifdef DEBUG_LOSPRE
  std::cout << "New tmpop: " << OP_SYMBOL_CONST(tmpop)->name << " "; printTypeChain(operandType (IC_RESULT(ic)), stdout); std::cout << "\n";
#endif

  for(typename std::set<edge_desc_t>::iterator i = calculation_edges.begin(); i != calculation_edges.end(); ++i)
  {
    split_edge(T, G, *i, ic, tmpop);
    split++;
  }

  typedef typename boost::graph_traits<G_t>::vertex_iterator vertex_iter_t;
  vertex_iter_t v, v_end;

  for(boost::tie(v, v_end) = boost::vertices(G); v != v_end; ++v)
    {
      if(!G[*v].uses)
        continue;
      typename boost::graph_traits<G_t>::in_edge_iterator e = in_edges(*v, G).first;
      if (a.global.size() <= *v)
        continue;
      if(!((a.global[*v] & true) && !G[*v].invalidates || boost::source(*e, G) < a.global.size() && (a.global[boost::source(*e, G)] & true)))
        continue;
#ifdef DEBUG_LOSPRE
      std::cout << "Substituting ic " << G[*v].ic->key << "\n";
#endif
      substituted++;

      iCode *ic = G[*v].ic;
      //if (IC_LEFT (ic) && IS_ITEMP (IC_LEFT (ic)))
      //  bitVectUnSetBit (OP_SYMBOL (IC_LEFT (ic))->uses, ic->key);
      //if (IC_RIGHT (ic) && IS_ITEMP (IC_RIGHT (ic)))
       // bitVectUnSetBit (OP_SYMBOL (IC_RIGHT (ic))->uses, ic->key);
      IC_RIGHT(ic) = tmpop;
      //bitVectSetBit (OP_SYMBOL (IC_RIGHT(ic))->uses, ic->key);
      if (!POINTER_SET (ic))
        {
          IC_LEFT(ic) = 0;
          ic->op = '=';
          IC_RESULT(ic) = operandFromOperand (IC_RESULT (ic));
          IC_RESULT(ic)->isaddr = 0;
        }
      if(IS_OP_VOLATILE(IC_RESULT (ic)))
        continue;

      {
        typedef typename boost::graph_traits<G_t>::adjacency_iterator adjacency_iter_t;
        adjacency_iter_t c, c_end;
        boost::tie(c, c_end) = adjacent_vertices(*v, G);
        if (c != c_end)
          forward_lospre_assignment(G, *c, ic, a);
      }
    }

  if(substituted <= 0)
    {
      std::cerr << "Introduced " << OP_SYMBOL_CONST(tmpop)->name << ", but did not substitute any calculations.\n";
      return (-1);
    }

  if(substituted < split) // Todo: Remove this warning when optimization for speed instead of code size is implemented!
    std::cout << "Introduced " << OP_SYMBOL_CONST(tmpop)->name << ", but did substitute only " << substituted << " calculations, while introducing "<< split << ".\n"; std::cout.flush();

  return(1);
}

/* Using a template here confuses debugging tools such as valgrind. */
/*template <class T_t, class G_t>*/
static int tree_dec_lospre (tree_dec_lospre_t/*T_t*/ &T, cfg_lospre_t/*G_t*/ &G, const iCode *ic)
{
  if(tree_dec_lospre_nodes(T, find_root(T), G))
    return(-1);

  wassert(T[find_root(T)].assignments.begin() != T[find_root(T)].assignments.end());
  const assignment_lospre &winner = *(T[find_root(T)].assignments.begin());

  //std::cout << "Winner (lospre): ";
  //print_assignment(winner, G);

  int change;
  if (change = implement_lospre_assignment(winner, T, G, ic))
    nicify (T);
  T[find_root(T)].assignments.clear();
  return(change);
}

template <class G_t>
static void implement_safety(const assignment_lospre &a, G_t &G)
{
  typedef typename boost::graph_traits<G_t>::vertex_iterator vertex_iter_t;
  vertex_iter_t v, v_end;

  for(boost::tie(v, v_end) = boost::vertices(G); v != v_end; ++v)
    G[*v].invalidates |= a.global[*v];
}

/* Using a template here confuses debugging tools such as valgrind. */
/*template <class T_t, class G_t>*/
static int tree_dec_safety (tree_dec_lospre_t/*T_t*/ &T, cfg_lospre_t/*G_t*/ &G, const iCode *ic)
{
  if(tree_dec_safety_nodes(T, find_root(T), G))
    return(-1);

  wassert(T[find_root(T)].assignments.begin() != T[find_root(T)].assignments.end());
  const assignment_lospre &winner = *(T[find_root(T)].assignments.begin());

  implement_safety(winner, G);

#ifdef DEBUG_LOSPRE
  std::cout << "Winner (safety): ";
  print_assignment(winner, G);
#endif

  T[find_root(T)].assignments.clear();
  return (0);
}

