// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2011
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

//#define DEBUG_RALLOC_DEC // Uncomment to get debug messages while doing register allocation on the tree decomposition.
//#define DEBUG_RALLOC_DEC_ASS // Uncomment to get debug messages about assignments while doing register allocation on the tree decomposition (much more verbose than the one above).

#define TD_SALLOC
#define CH_SALLOC

#include "SDCCralloc.hpp"

extern "C"
{
  #include "ralloc.h"
  #include "gen.h"
  unsigned char dryhc08iCode (iCode *ic);
  bool hc08_assignment_optimal;
};

#define REG_A 0
#define REG_X 1
#define REG_H 2

template <class I_t>
static void add_operand_conflicts_in_node(const cfg_node &n, I_t &I)
{
  const iCode *ic = n.ic;
  
  const operand *result = IC_RESULT(ic);
  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
	
  if(!result || !IS_SYMOP(result))
    return;
    
  // Todo: Identify more operations that code generation can always handle and exclude them (as done for the z80-like ports).
  if (ic->op == '=')
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

  operand_map_t::const_iterator oi, oi_end;
  for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key); oi != oi_end; ++oi)
    if(oi->second == ia.registers[r][1] || oi->second == ia.registers[r][0])
      return(true);

  return(false);
}

// Check that the operand is either fully in registers or fully in memory.
template <class G_t, class I_t>
static bool operand_sane(const operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  if(!o || !IS_SYMOP(o))
    return(true);
 
  operand_map_t::const_iterator oi, oi2, oi_end;
  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key);
  
  if(oi == oi_end)
    return(true);

  // Go to the second byte. If the operand is only a single byte, it cannot be
  // an unsupported register combination or split between register and memory.
  oi2 = oi;
  oi2++;
  if (oi2 == oi_end)
    return(true);
  
  // Register combinations code generation cannot handle yet (AH, XH, HA).
  if(a.local.find(oi->second) != a.local.end() && a.local.find(oi2->second) != a.local.end())
    {
      const reg_t l = a.global[oi->second];
      const reg_t h = a.global[oi2->second];
      if(l == REG_A && h == REG_H || l == REG_H)
        return(false);
    }
  
  // In registers.
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
 
  return(true);
}

template <class G_t, class I_t>
static bool inst_sane(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  return(operand_sane(IC_RESULT(ic), a, i, G, I) && operand_sane(IC_LEFT(ic), a, i, G, I) && operand_sane(IC_RIGHT(ic), a, i, G, I));
}

template <class G_t, class I_t>
static bool operand_is_ax(const operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{  
  if(!o || !IS_SYMOP(o))
    return(false);
 
  operand_map_t::const_iterator oi, oi2, oi_end;
  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key);
  
  if(oi == oi_end)
    return(false);

  oi2 = oi;
  oi2++;
  if (oi2 == oi_end)
    return(false);
  
  // Register combinations code generation cannot handle yet (AX, AH, XH, HA).
  if(a.local.find(oi->second) != a.local.end() && a.local.find(oi2->second) != a.local.end())
    {
      const reg_t l = a.global[oi->second];
      const reg_t h = a.global[oi2->second];
      if(l == REG_X && h == REG_A)
        return(true);
    }

  return(false);
}

template <class G_t, class I_t>
static bool XAinst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  // Instructions that can handle anything.
  if(ic->op == '!' ||
    ic->op == '~' ||
    ic->op == UNARYMINUS ||
    ic->op == CALL ||
    ic->op == PCALL ||
    ic->op == FUNCTION ||
    ic->op == ENDFUNCTION ||
    ic->op == RETURN ||
    ic->op == LABEL ||
    ic->op == GOTO ||
    ic->op == IFX ||
    ic->op == '+' ||
    ic->op == '-' ||
    ic->op == '*' ||
    ic->op == '/' ||
    ic->op == '%' ||
    ic->op == '<' || ic->op == '>' || ic->op == LE_OP || ic->op == GE_OP ||
    ic->op == NE_OP || ic->op == EQ_OP ||
    ic->op == AND_OP ||
    ic->op == OR_OP ||
    ic->op == '^' ||
    ic->op == '|' ||
    ic->op == BITWISEAND ||
    ic->op == GETABIT ||
    ic->op == GETBYTE ||
    ic->op == GETWORD ||
    ic->op == LEFT_OP ||
    ic->op == RIGHT_OP ||
    ic->op == '=' ||  /* both regular assignment and POINTER_SET safe */
    ic->op == GET_VALUE_AT_ADDRESS ||
    ic->op == ADDRESS_OF ||
    ic->op == CAST ||
    ic->op == DUMMY_READ_VOLATILE ||
    ic->op == SWAP)
    return(true);

  if(ic->op == IFX && ic->generated)
    return(true);

  const i_assignment_t &ia = a.i_assignment;

  bool unused_A = (ia.registers[REG_A][1] < 0);
  bool unused_H = (ia.registers[REG_H][1] < 0);
  bool unused_X = (ia.registers[REG_X][1] < 0);

  if(unused_X && unused_A && unused_H)
    return(true);

#if 0
  std::cout << "XAinst_ok: at (" << i << ", " << ic->key << ")\nX = (" << ia.registers[REG_X][0] << ", " << ia.registers[REG_X][1] << "), A = (" << ia.registers[REG_A][0] << ", " << ia.registers[REG_A][1] << ")inst " << i << ", " << ic->key << "\n";
#endif

  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
  const operand *result = IC_RESULT(ic);

  bool result_in_A = operand_in_reg(result, REG_A, ia, i, G) && !(ic->op == '=' && POINTER_SET(ic));
  bool result_in_H = operand_in_reg(result, REG_H, ia, i, G) && !(ic->op == '=' && POINTER_SET(ic));
  bool result_in_X = operand_in_reg(result, REG_X, ia, i, G) && !(ic->op == '=' && POINTER_SET(ic));
  bool left_in_A = operand_in_reg(result, REG_A, ia, i, G);
  bool left_in_X = operand_in_reg(result, REG_X, ia, i, G);

  const std::set<var_t> &dying = G[i].dying;

  bool dying_A = result_in_A || dying.find(ia.registers[REG_A][1]) != dying.end() || dying.find(ia.registers[REG_A][0]) != dying.end();
  bool dying_H = result_in_H || dying.find(ia.registers[REG_H][1]) != dying.end() || dying.find(ia.registers[REG_H][0]) != dying.end();
  bool dying_X = result_in_X || dying.find(ia.registers[REG_X][1]) != dying.end() || dying.find(ia.registers[REG_X][0]) != dying.end();

  bool result_only_XA = (result_in_X || unused_X || dying_X) && (result_in_A || unused_A || dying_A);

  if(ic->op == JUMPTABLE && (unused_A || dying_A))
    return(true);

  if(ic->op == IPUSH && (unused_A || dying_A || left_in_A || operand_in_reg(left, REG_H, ia, i, G) || left_in_X))
    return(true);

  if(ic->op == RECEIVE && (!ic->next || !(ic->next->op == RECEIVE) || !result_in_X || getSize(operandType(result)) >= 2))
    return(true);

  if(ic->op == SEND && ic->next && ic->next->op == SEND && ic->next->next && ic->next->next->op == SEND)
    return(true);

  if(ic->op == SEND && ic->next && ic->next->op == SEND && (unused_X || dying_X))
    return(true);

  if(ic->op == SEND && (unused_X || dying_X) && (unused_A || dying_A))
    return(true);

  if(ic->op == SEND && ic->next && (ic->next->op == CALL || ic->next->op == PCALL)) // Might mess up A and X, but these would have been saved before if surviving, and will not be needed again before the call.
    return(true);

  if((ic->op == CRITICAL || ic->op == ENDCRITICAL) && (unused_A || dying_A))
    return(true);

  return(false);
}

template <class G_t, class I_t>
static bool AXinst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;

  const i_assignment_t &ia = a.i_assignment;

  if(ic->op == '!' ||
    ic->op == '~' ||
    ic->op == IPUSH ||
    ic->op == CALL ||
    ic->op == FUNCTION ||
    ic->op == ENDFUNCTION ||
    ic->op == RETURN ||
    ic->op == LABEL ||
    ic->op == GOTO ||
    ic->op == '+' ||
    ic->op == '-' ||
    ic->op == NE_OP || ic->op == EQ_OP ||
    ic->op == '^' ||
    ic->op == '|' ||
    ic->op == BITWISEAND ||
    ic->op == GETABIT ||
    ic->op == GETBYTE ||
    ic->op == GETWORD ||
    /*ic->op == LEFT_OP ||
    ic->op == RIGHT_OP ||*/
    ic->op == GET_VALUE_AT_ADDRESS ||
    ic->op == '=' ||
    ic->op == ADDRESS_OF ||
    ic->op == RECEIVE ||
    ic->op == SEND ||
    ic->op == DUMMY_READ_VOLATILE ||
    ic->op == CRITICAL ||
    ic->op == ENDCRITICAL ||
    ic->op == SWAP)
    return(true);

  bool unused_A = (ia.registers[REG_A][1] < 0);
  bool unused_X = (ia.registers[REG_X][1] < 0);

  if (unused_A || unused_X)
    return(true);

  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
  const operand *result = IC_RESULT(ic);

  bool result_in_A = operand_in_reg(result, REG_A, ia, i, G) && !(ic->op == '=' && POINTER_SET(ic));
  bool result_in_X = operand_in_reg(result, REG_X, ia, i, G) && !(ic->op == '=' && POINTER_SET(ic));
  bool left_in_A = operand_in_reg(result, REG_A, ia, i, G);
  bool left_in_X = operand_in_reg(result, REG_X, ia, i, G);
  bool right_in_A = operand_in_reg(result, REG_A, ia, i, G);
  bool right_in_X = operand_in_reg(result, REG_X, ia, i, G);

  bool result_is_ax = operand_is_ax (result, a, i, G, I);
  bool left_is_ax = operand_is_ax (left, a, i, G, I);
  bool right_is_ax = operand_is_ax (right, a, i, G, I);

  if (!result_is_ax && !left_is_ax && !right_is_ax)
    return(true);

  return(false);
}

template <class G_t, class I_t>
static void set_surviving_regs(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;
  
  ic->rMask = newBitVect(port->num_regs);
  ic->rSurv = newBitVect(port->num_regs);
  
  std::set<var_t>::const_iterator v, v_end;
  for (v = G[i].alive.begin(), v_end = G[i].alive.end(); v != v_end; ++v)
    {
      if(a.global[*v] < 0)
        continue;
      ic->rMask = bitVectSetBit(ic->rMask, a.global[*v]);
      if(G[i].dying.find(*v) == G[i].dying.end())
        if(!((IC_RESULT(ic) && !POINTER_SET(ic)) && IS_SYMOP(IC_RESULT(ic)) && OP_SYMBOL_CONST(IC_RESULT(ic))->key == I[*v].v))
          ic->rSurv = bitVectSetBit(ic->rSurv, a.global[*v]);
    }
}

template<class G_t>
static void unset_surviving_regs(unsigned short int i, const G_t &G)
{
  iCode *ic = G[i].ic;
  
  freeBitVect(ic->rSurv);
  freeBitVect(ic->rMask);
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
          sym->regs[I[v].byte] = regshc08 + a.global[v];   
          sym->isspilt = false;
          sym->nRegs = I[v].size;
          sym->accuse = 0;
        }
      else
        {
          for(int i = 0; i < I[v].size; i++)
            sym->regs[i] = 0;
          sym->accuse = 0;
          sym->nRegs = I[v].size;
          sym->isspilt = true;
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
    {
      assign_operands_for_cost(a, *(adjacent_vertices(i, G).first), G, I);
    }
}

// Cost function.
template <class G_t, class I_t>
static float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;
  float c;

  wassert (TARGET_IS_HC08 || TARGET_IS_S08);

  if(!inst_sane(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  if(!XAinst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

  if(!AXinst_ok(a, i, G, I))
    return(std::numeric_limits<float>::infinity());

#if 0
  std::cout << "Calculating at cost at ic " << ic->key << " for: ";
  for(unsigned int i = 0; i < boost::num_vertices(I); i++)
  {
  	std::cout << "(" << i << ", " << int(a.global[i]) << ") ";
  }
  std::cout << "\n";
  std::cout.flush();
#endif

  if(ic->generated)
    return(0.0f);

  switch(ic->op)
    {
    // Register assignment doesn't matter for these:
    case FUNCTION:
    case ENDFUNCTION:
    case LABEL:
    case GOTO:
    case INLINEASM:
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
    case '=':
    case IFX:
    case ADDRESS_OF:
    case JUMPTABLE:
    case CAST:
    case RECEIVE:
    case SEND:
    case DUMMY_READ_VOLATILE:
    case CRITICAL:
    case ENDCRITICAL:
    case SWAP:
      assign_operands_for_cost(a, i, G, I);
      set_surviving_regs(a, i, G, I);
      c = dryhc08iCode(ic);
      unset_surviving_regs(i, G);
      return(c);
    default:
      return(0.0f);
    }
}

// For early removal of assignments that cannot be extended to valid assignments. This is just a dummy for now, it probably isn't really needed for hc08 due to the low number of registers.
template <class G_t, class I_t>
static bool assignment_hopeless(const assignment &a, unsigned short int i, const G_t &G, const I_t &I, const var_t lastvar)
{
  return(false);
}

// Increase chance of finding good compatible assignments at join nodes. This is just a dummy for now, it probably isn't really needed for hc08 due to the low number of registers.
template <class T_t>
static void get_best_local_assignment_biased(assignment &a, typename boost::graph_traits<T_t>::vertex_descriptor t, const T_t &T)
{
  a = *T[t].assignments.begin();

  std::set<var_t>::const_iterator vi, vi_end;
  for(vi = T[t].alive.begin(), vi_end = T[t].alive.end(); vi != vi_end; ++vi)
    a.local.insert(*vi);
}

// This is just a dummy for now, it probably isn't really needed for hc08 due to the low number of registers.
template <class G_t, class I_t>
static float rough_cost_estimate(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  return(0.0f);
}

// Code for another ic is generated when generating this one. Mark the other as generated.
static void extra_ic_generated(iCode *ic)
{
  if(ic->op == '>' || ic->op == '<' || ic->op == LE_OP || ic->op == GE_OP || ic->op == EQ_OP || ic->op == NE_OP || ic->op == '^' || ic->op == '|' || ic->op == BITWISEAND)
    {
      iCode *ifx;
      if (ifx = ifxForOp (IC_RESULT (ic), ic))
        {
          OP_SYMBOL (IC_RESULT (ic))->for_newralloc = false;
          OP_SYMBOL (IC_RESULT (ic))->regType = REG_CND;
          ifx->generated = true;
        }
    }
  if(ic->op == '-' && IS_VALOP (IC_RIGHT (ic)) && operandLitValue (IC_RIGHT (ic)) == 1 && getSize(operandType(IC_RESULT (ic))) == 1 && !isOperandInFarSpace (IC_RESULT (ic)) && isOperandEqual (IC_RESULT (ic), IC_LEFT (ic)))
    {
      iCode *ifx;
      if (ifx = ifxForOp (IC_RESULT (ic), ic))
        {
          OP_SYMBOL (IC_RESULT (ic))->for_newralloc = false;
          OP_SYMBOL (IC_RESULT (ic))->regType == REG_CND;
          ifx->generated = true;
        }
    }
  if(ic->op == GET_VALUE_AT_ADDRESS)
    {
      iCode *inc;
      if (inc = hasInchc08 (IC_LEFT (ic), ic,  getSize (operandType (IC_RIGHT (ic)))))
         inc->generated = true;
    }
}

template <class T_t, class G_t, class I_t>
static bool tree_dec_ralloc(T_t &T, G_t &G, const I_t &I)
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
      if(winner.global[v] >= 0)
        { 
          sym->regs[I[v].byte] = regshc08 + winner.global[v];   
          sym->isspilt = false;
          sym->nRegs = I[v].size;
          sym->accuse = 0;
        }
      else
        {
          for(int i = 0; i < I[v].size; i++)
            sym->regs[i] = 0;
          sym->accuse = 0;
          sym->nRegs = I[v].size;
          wassert (sym->nRegs);
          //spillThis(sym); Leave it to regFix, which can do some spillocation compaction. Todo: Use Thorup instead.
          sym->isspilt = false;
        }
    }

  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    set_surviving_regs(winner, i, G, I);	// Never freed. Memory leak?

  return(!assignment_optimal);
}

iCode *hc08_ralloc2_cc(ebbIndex *ebbi)
{
  iCode *ic;

#ifdef DEBUG_RALLOC_DEC
  std::cout << "Processing " << currFunc->name << " from " << dstFileName << "\n"; std::cout.flush();
#endif

  cfg_t control_flow_graph;

  con_t conflict_graph;

  ic = create_cfg(control_flow_graph, conflict_graph, ebbi);

  if(options.dump_graphs)
    dump_cfg(control_flow_graph);

  if(options.dump_graphs)
    dump_con(conflict_graph);

  tree_dec_t tree_decomposition;

  thorup_tree_decomposition(tree_decomposition, control_flow_graph);

  nicify(tree_decomposition);

  alive_tree_dec(tree_decomposition, control_flow_graph);

  good_re_root(tree_decomposition);
  nicify(tree_decomposition);
  alive_tree_dec(tree_decomposition, control_flow_graph);

  if(options.dump_graphs)
    dump_tree_decomposition(tree_decomposition);

  hc08_assignment_optimal = !tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph);

  return(ic);
}

