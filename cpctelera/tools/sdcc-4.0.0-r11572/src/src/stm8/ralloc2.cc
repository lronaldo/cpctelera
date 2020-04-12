// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2013
//
// (c) 2010 - 2013 Goethe-Universität Frankfurt
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

// #define DEBUG_RALLOC_DEC // Uncomment to get debug messages while doing register allocation on the tree decomposition.
// #define DEBUG_RALLOC_DEC_ASS // Uncomment to get debug messages about assignments while doing register allocation on the tree decomposition (much more verbose than the one above).

#include "SDCCralloc.hpp"
#include "SDCCsalloc.hpp"

extern "C"
{
  #include "ralloc.h"
  #include "gen.h"
  float drySTM8iCode (iCode *ic);
  bool stm8_assignment_optimal;
  long int stm8_call_stack_size;
  bool stm8_extend_stack;
}

#define REG_A 0
#define REG_XL 1
#define REG_XH 2
#define REG_YL 3
#define REG_YH 4
#define REG_C 5

template <class I_t>
static void add_operand_conflicts_in_node(const cfg_node &n, I_t &I)
{
  const iCode *ic = n.ic;
  
  const operand *result = IC_RESULT(ic);
  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);

  if(!result || !IS_SYMOP(result))
    return;

  // Todo: More fine-grained control for these.
  if (!(ic->op == '+' || ic->op == '-' || ic->op == UNARYMINUS && !IS_FLOAT (operandType (left)) || ic->op == '~' ||
    ic->op == '^' || ic->op == '|' || ic->op == BITWISEAND ||
    ic->op == GET_VALUE_AT_ADDRESS))
    return;

  operand_map_t::const_iterator oir, oir_end, oirs; 
  boost::tie(oir, oir_end) = n.operands.equal_range(OP_SYMBOL_CONST(result)->key);
  if(oir == oir_end)
    return;
    
  operand_map_t::const_iterator oio, oio_end;
  
  if(left && IS_SYMOP(left))
    for(boost::tie(oio, oio_end) = n.operands.equal_range(OP_SYMBOL_CONST(left)->key); oio != oio_end; ++oio)
      for(oirs = oir; oirs != oir_end; ++oirs)
        {
          var_t rvar = oirs->second;
          var_t ovar = oio->second;
          if(I[rvar].byte < I[ovar].byte)
            boost::add_edge(rvar, ovar, I);
        }
        
  if(right && IS_SYMOP(right))
    for(boost::tie(oio, oio_end) = n.operands.equal_range(OP_SYMBOL_CONST(right)->key); oio != oio_end; ++oio)
      for(oirs = oir; oirs != oir_end; ++oirs)
        {
          var_t rvar = oirs->second;
          var_t ovar = oio->second;
          if(I[rvar].byte < I[ovar].byte)
            boost::add_edge(rvar, ovar, I);
        }
}

// Return true, iff the operand is placed (partially) in r.
template <class G_t>
static bool operand_in_reg(const operand *o, reg_t r, const i_assignment_t &ia, unsigned short int i, const G_t &G)
{
  if(!o || !IS_SYMOP(o))
    return(false);

  if(r >= port->num_regs)
    return(false);

  operand_map_t::const_iterator oi, oi_end;
  for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key); oi != oi_end; ++oi)
    if(oi->second == ia.registers[r][1] || oi->second == ia.registers[r][0])
      return(true);

  return(false);
}

template <class G_t, class I_t>
static bool Ainst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
  const operand *const left = IC_LEFT(ic);

  const i_assignment_t &ia = a.i_assignment;

  if(ia.registers[REG_A][1] < 0)
    return(true);       // Register a not in use.

  if(ic->op == IPUSH)
    {
      if (ia.registers[REG_XL][1] < 0 || ia.registers[REG_YL][1] < 0 && !stm8_extend_stack)
        return(true);   // Register xl or yl free; code generation can use them when a is not available.

      // push a does not disturb a.
      if (getSize(operandType(IC_LEFT(ic))) <= 1 && operand_in_reg(left, REG_A, ia, i, G))
        return(true);

      // push #byte does not disturb a.
      if (IS_OP_LITERAL(left))
        return(true);

      // push longmem does not disturb a.
      if (IS_OP_GLOBAL(left))
        return(true);

      // Only look at itemp pushes below.
      if (!IS_ITEMP(left))
        return(false);

      // Register pushes do not disturb a.
      for (unsigned short i = 0; i < getSize(operandType(IC_LEFT(ic)));)
        {
          if(operand_in_reg(left, REG_A, ia, i, G))
            i++;
          else if(operand_in_reg(left, REG_XL, ia, i, G) && operand_in_reg(left, REG_XH, ia, i + 1, G))
            i += 2;
          else if(operand_in_reg(left, REG_YL, ia, i, G) && operand_in_reg(left, REG_YH, ia, i + 1, G))
            i += 2;
          else if(operand_in_reg(left, REG_XL, ia, i, G) || operand_in_reg(left, REG_YL, ia, i, G))
            i++;
          else
            return(false);
        }

      return(true);
    }

  return(true);
}

template <class G_t, class I_t>
static bool Yinst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
  const operand *const left = IC_LEFT(ic);

  const i_assignment_t &ia = a.i_assignment;

  if(!stm8_extend_stack)
    return(true);   // Only an extended stack can make Y unavailable.

  if(ia.registers[REG_YL][1] < 0 && ia.registers[REG_YH][1] < 0)
    return(true);   // Register Y not in use.

  return(false);
}

template <class G_t, class I_t>
static void set_surviving_regs(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;

  bitVectClear(ic->rMask);
  bitVectClear(ic->rSurv);

  cfg_alive_t::const_iterator v, v_end;
  for (v = G[i].alive.begin(), v_end = G[i].alive.end(); v != v_end; ++v)
    {
      if(a.global[*v] < 0)
        continue;
      ic->rMask = bitVectSetBit(ic->rMask, a.global[*v]);

      if(!(IC_RESULT(ic) && IS_SYMOP(IC_RESULT(ic)) && OP_SYMBOL_CONST(IC_RESULT(ic))->key == I[*v].v))
        if(G[i].dying.find(*v) == G[i].dying.end())
          ic->rSurv = bitVectSetBit(ic->rSurv, a.global[*v]);
    }
}

template <class G_t, class I_t>
static void assign_operand_for_cost(operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  if(!o || !IS_SYMOP(o))
    return;
  symbol *sym = OP_SYMBOL(o);
  operand_map_t::const_iterator oi, oi_end;
  for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key); oi != oi_end; ++oi)
    {
      var_t v = oi->second;
      if(a.global[v] >= 0)
        { 
          sym->regs[I[v].byte] = stm8_regs + a.global[v];   
          sym->nRegs = I[v].size;
        }
      else
        {
          sym->regs[I[v].byte] = 0;
          sym->nRegs = I[v].size;
        }
    }
}

template <class G_t, class I_t>
static void assign_operands_for_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
  
  if(ic->op == IFX)
    assign_operand_for_cost(IC_COND(ic), a, i, G, I);
  else if(ic->op == JUMPTABLE)
    assign_operand_for_cost(IC_JTCOND(ic), a, i, G, I);
  else
    {
      assign_operand_for_cost(IC_LEFT(ic), a, i, G, I);
      assign_operand_for_cost(IC_RIGHT(ic), a, i, G, I);
      assign_operand_for_cost(IC_RESULT(ic), a, i, G, I);
    }
    
  if(ic->op == SEND && ic->builtinSEND)
    assign_operands_for_cost(a, (unsigned short)*(adjacent_vertices(i, G).first), G, I);
}

template <class G_t, class I_t>
static bool operand_sane(const operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
#if 0
  int v, byteregs[8];   // Todo: Change this when sdcc supports variables larger than 8 bytes.
  unsigned short int size;

  if(!o || !IS_SYMOP(o))
    return(true);
 
  operand_map_t::const_iterator oi, oi_end;
  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key);
  
  if(oi == oi_end)
    return(true);
  
  // Ensure: Fully in registers or fully in mem.
  if(a.local.find(oi->second) != a.local.end())
    {
      while(++oi != oi_end)
        if(a.local.find(oi->second) == a.local.end())
          return(false);
    }
  else
    {
       while(++oi != oi_end)
        if(a.local.find(oi->second) != a.local.end())
          return(false);
    }

  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key);
  v = oi->second;
  byteregs[I[v].byte] = a.global[v];
  size = 1;
  while(++oi != oi_end)
    {
      v = oi->second;
      byteregs[I[v].byte] = a.global[v];
      size++;
    }

  if (byteregs[0] == -1)
    return(true);

  // Ensure: 8 bit only in A, 16 bit only in X or Y.
  if (size == 1)
    return(byteregs[0] == A_IDX);
  if (size == 2)
    return(byteregs[0] == XL_IDX && byteregs[1] == XH_IDX || byteregs[0] == YL_IDX && byteregs[1] == YH_IDX);
  if (size > 2)
    return(false);
#endif

  return(true);
}

template <class G_t, class I_t>
static bool inst_sane(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  return(operand_sane(IC_RESULT(ic), a, i, G, I) && operand_sane(IC_LEFT(ic), a, i, G, I) && operand_sane(IC_RIGHT(ic), a, i, G, I));
}

// Cost function.
template <class G_t, class I_t>
static float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;
  float c;

  wassert(TARGET_IS_STM8);
  wassert(ic);

  if(!inst_sane(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

#if 0
  std::cout << "Calculating at cost at ic " << ic->key << ", op " << ic->op << " for: ";
  print_assignment(a);
  std::cout << "\n";
  std::cout.flush();
#endif

  if(ic->generated)
    {
#if 0
  std::cout << "Skipping, already generated.\n";
#endif
      return(0.0f);
    }

  if(!Ainst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  if(!Yinst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  switch(ic->op)
    {
    // Register assignment doesn't matter for these:
    case FUNCTION:
    case ENDFUNCTION:
    case LABEL:
    case GOTO:
    case INLINEASM:
#if 0
  std::cout << "Skipping, indepent from assignment.\n";
#endif
      return(0.0f);
    case '!':
    case '~':
    case UNARYMINUS:
    case '+':
    case '-':
    case '^':
    case '|':
    case BITWISEAND:
    case IPUSH:
    //case IPOP:
    case CALL:
    case PCALL:
    case RETURN:
    case '*':
    case '/':
    case '%':
    case '>':
    case '<':
    case LE_OP:
    case GE_OP:
    case EQ_OP:
    case NE_OP:
    case AND_OP:
    case OR_OP:
    case GETABIT:
    case GETBYTE:
    case GETWORD:
    case LEFT_OP:
    case RIGHT_OP:
    case GET_VALUE_AT_ADDRESS:
    case SET_VALUE_AT_ADDRESS:
    case '=':
    case IFX:
    case ADDRESS_OF:
    case JUMPTABLE:
    case CAST:
    /*case RECEIVE:
    case SEND:*/
    case DUMMY_READ_VOLATILE:
    /*case CRITICAL:
    case ENDCRITICAL:*/
    case SWAP:
      assign_operands_for_cost(a, i, G, I);
      set_surviving_regs(a, i, G, I);
      c = drySTM8iCode(ic);
      ic->generated = false;
#if 0
      std::cout << "Got cost " << c << "\n";
#endif
      return(c);
    default:
      return(0.0f);
    }
}

// For early removal of assignments that cannot be extended to valid assignments. This is just a dummy for now.
template <class G_t, class I_t>
static bool assignment_hopeless(const assignment &a, unsigned short int i, const G_t &G, const I_t &I, const var_t lastvar)
{
  return(false);
}

// Increase chance of finding good compatible assignments at join nodes.
template <class T_t>
static void get_best_local_assignment_biased(assignment &a, typename boost::graph_traits<T_t>::vertex_descriptor t, const T_t &T)
{
  a = *T[t].assignments.begin();

  std::set<var_t>::const_iterator vi, vi_end;
  varset_t newlocal;
  std::set_union(T[t].alive.begin(), T[t].alive.end(), a.local.begin(), a.local.end(), std::inserter(newlocal, newlocal.end()));
  a.local = newlocal;
}

// Suggest to honor register keyword and to not reverse bytes and prefer use of a. Prefer x over y.
template <class G_t, class I_t>
static float rough_cost_estimate(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const i_assignment_t &ia = a.i_assignment;
  float c = 0.0f;

  if(ia.registers[REG_A][1] < 0)
    c += 0.05f;

  varset_t::const_iterator v, v_end;
  for(v = a.local.begin(), v_end = a.local.end(); v != v_end; ++v)
    {
      const symbol *const sym = (symbol *)(hTabItemWithKey(liveRanges, I[*v].v));
      if(a.global[*v] < 0 && !sym->remat) // Try to put non-rematerializeable variables into registers.
        c += 0.1f;
      if(a.global[*v] < 0 && IS_REGISTER(sym->type)) // Try to honour register keyword.
        c += 4.0f;
      if((I[*v].byte % 2) ? // Try not to reverse bytes.
        (a.global[*v] == REG_XL || a.global[*v] == REG_YL) :
        (a.global[*v] == REG_XH || a.global[*v] == REG_YH))
        c += 0.1f;
    }

  return(c);
}

// Code for another ic is generated when generating this one. Mark the other as generated.
static void extra_ic_generated(iCode *ic)
{
  if(ic->op == '>' || ic->op == '<' || ic->op == LE_OP || ic->op == GE_OP || ic->op == EQ_OP || ic->op == NE_OP ||
    ic->op == BITWISEAND && (IS_OP_LITERAL (IC_LEFT (ic)) || IS_OP_LITERAL (IC_RIGHT (ic))) || ic->op == GETABIT)
    {
      iCode *ifx;

      // Bitwise and code generation can only do the jump if one operand is a literal with at most one nonzero byte.
      if (ic->op == BITWISEAND && getSize(operandType(IC_RESULT(ic))) > 1)
        {
          int nonzero = 0;
          operand *const litop = IS_OP_LITERAL (IC_LEFT (ic)) ? IC_LEFT (ic) : IC_RIGHT (ic);

          for(unsigned int i = 0; i < getSize(operandType(IC_LEFT (ic))) && i < getSize(operandType(IC_RIGHT (ic))) && i < getSize(operandType(IC_RESULT(ic))); i++)
            if(byteOfVal (OP_VALUE (litop), i))
              nonzero++;

          if(nonzero > 1)
            return;
        }
      if (ic->op == GETABIT)
        {
          unsigned bit = byteOfVal (OP_VALUE (IC_RIGHT (ic)), 0);

          if (bit % 8 != 7)
            return;
        }

      if (ifx = ifxForOp (IC_RESULT (ic), ic))
        {
          OP_SYMBOL (IC_RESULT (ic))->for_newralloc = false;
          OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
          ifx->generated = true;
        }
    }
}

template <class T_t, class G_t, class I_t, class SI_t>
static bool tree_dec_ralloc(T_t &T, G_t &G, const I_t &I, SI_t &SI)
{
  bool assignment_optimal;

  con2_t I2(boost::num_vertices(I));
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
    {
      I2[i].v = I[i].v;
      I2[i].byte = I[i].byte;
      I2[i].size = I[i].size;
      I2[i].name = I[i].name;
    }
  typename boost::graph_traits<I_t>::edge_iterator e, e_end;
  for(boost::tie(e, e_end) = boost::edges(I); e != e_end; ++e)
    add_edge(boost::source(*e, I), boost::target(*e, I), I2);

  assignment ac;
  assignment_optimal = true;
  tree_dec_ralloc_nodes(T, find_root(T), G, I2, ac, &assignment_optimal);

  const assignment &winner = *(T[find_root(T)].assignments.begin());

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Winner: ";
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
    {
      std::cout << "(" << i << ", " << int(winner.global[i]) << ") ";
    }
  std::cout << "\n";
  std::cout << "Cost: " << winner.s << "\n";
  std::cout.flush();
#endif

  // Todo: Make this an assertion
  if(winner.global.size() != boost::num_vertices(I))
    {
      std::cerr << "ERROR: No Assignments at root\n";
      exit(-1);
    }

  for(unsigned int v = 0; v < boost::num_vertices(I); v++)
    {
      symbol *sym = (symbol *)(hTabItemWithKey(liveRanges, I[v].v));
      bool spilt = false;

      if(winner.global[v] >= 0)
        sym->regs[I[v].byte] = stm8_regs + winner.global[v];   
      else
        {
          sym->regs[I[v].byte] = 0;
          spilt = true;
        }

      if(spilt)
        stm8SpillThis(sym, false);

      sym->nRegs = I[v].size;
    }

  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    set_surviving_regs(winner, i, G, I);

  set_spilt(G, I, SI);

  return(!assignment_optimal);
}

iCode *stm8_ralloc2_cc(ebbIndex *ebbi)
{
  eBBlock **const ebbs = ebbi->bbOrder;
  const int count = ebbi->count;

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Processing " << currFunc->name << " from " << dstFileName << "\n"; std::cout.flush();
#endif

  cfg_t control_flow_graph;

  con_t conflict_graph;

  iCode *ic = create_cfg(control_flow_graph, conflict_graph, ebbi);

  if(options.dump_graphs)
    dump_cfg(control_flow_graph);

  if(options.dump_graphs)
    dump_con(conflict_graph);

  tree_dec_t tree_decomposition;

  get_nice_tree_decomposition(tree_decomposition, control_flow_graph);

  alive_tree_dec(tree_decomposition, control_flow_graph);

  good_re_root(tree_decomposition);
  nicify(tree_decomposition);
  alive_tree_dec(tree_decomposition, control_flow_graph);

  if(options.dump_graphs)
    dump_tree_decomposition(tree_decomposition);

  guessCounts (ic, ebbi);

  scon_t stack_conflict_graph;

  stm8_assignment_optimal = !tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph, stack_conflict_graph);

  stm8RegFix (ebbs, count);

  chaitin_salloc(stack_conflict_graph);

  if(options.dump_graphs)
    dump_scon(stack_conflict_graph);

  return(ic);
}

