// Philipp Klaus Krause, philipp@informatik.uni-frankfurt.de, pkk@spth.de, 2010 - 2018
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

// #define DEBUG_RALLOC_DEC // Uncomment to get debug messages while doing register allocation on the tree decomposition.
// #define DEBUG_RALLOC_DEC_ASS // Uncomment to get debug messages about assignments while doing register allocation on the tree decomposition (much more verbose than the one above).

#include "SDCCralloc.hpp"
#include "SDCCsalloc.hpp"

extern "C"
{
  #include "ralloc.h"
  #include "gen.h"
  float dryPdkiCode (iCode *ic);
  bool pdk_assignment_optimal;
}

#define REG_A 0
#define REG_P 1

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
  if (!(ic->op == '-' || ic->op == UNARYMINUS && !IS_FLOAT (operandType (left)) || ic->op == '~' ||
    ic->op == '^' || ic->op == '|' || ic->op == BITWISEAND))
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

// Return true, iff the operand is placed in a reg.
template <class G_t>
static bool operand_byte_in_reg(const operand *o, int offset, reg_t r, const assignment &a, unsigned short int i, const G_t &G)
{
  if(!o || !IS_SYMOP(o))
    return(false);

  operand_map_t::const_iterator oi, oi2, oi3, oi_end;

  for(boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key); offset && oi != oi_end; offset--, oi++);

  if(oi == oi_end)
    return(false);

  return(a.global[oi->second] == r);
}

template <class G_t, class I_t>
static bool Ainst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
  const i_assignment_t &ia = a.i_assignment;

  if(ia.registers[REG_A][1] < 0)
    return(true);       // Register a not in use.

  if(ic->op == GOTO || ic->op == LABEL)
    return(true);

  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
  const operand *result = IC_RESULT(ic);

  bool result_in_A = operand_in_reg(result, REG_A, ia, i, G);
  bool left_in_A = operand_in_reg(left, REG_A, ia, i, G);
  bool right_in_A = operand_in_reg(right, REG_A, ia, i, G);

  const cfg_dying_t &dying = G[i].dying;

  bool dying_A = result_in_A || dying.find(ia.registers[REG_A][1]) != dying.end() || dying.find(ia.registers[REG_A][0]) != dying.end();

  bool result_dir = IS_TRUE_SYMOP (result) || IS_ITEMP (result) && !(options.stackAuto || reentrant) && !result_in_A;
  bool left_dir = IS_TRUE_SYMOP (left) || IS_ITEMP (left) && !(options.stackAuto || reentrant) && !left_in_A;
  bool right_dir = IS_TRUE_SYMOP (right) || IS_ITEMP (right) && !(options.stackAuto || reentrant) && !right_in_A;

  if (ic->op == '=' || ic->op == DUMMY_READ_VOLATILE || ic->op == CAST || ic->op == GET_VALUE_AT_ADDRESS || ic->op == SET_VALUE_AT_ADDRESS || ic->op == '~' || ic->op == '|' || ic->op == '^' || ic->op == BITWISEAND && !ifxForOp (result, ic))
    return(true);

  if(result && IS_ITEMP(result) && OP_SYMBOL_CONST(result)->remat && !operand_in_reg(result, REG_A, ia, i, G) && !operand_in_reg(result, REG_P, ia, i, G))
    return(true);

  if ((ic->op == EQ_OP || ic->op == NE_OP) && dying_A &&
    (left_in_A && (right_dir || IS_OP_LITERAL(right) || IS_ITEMP(right) && OP_SYMBOL_CONST(right)->remat) ||
    right_in_A && (left_dir || IS_OP_LITERAL(left) || IS_ITEMP(left) && OP_SYMBOL_CONST(left)->remat)))
    return (true);

  if ((ic->op == EQ_OP || ic->op == NE_OP || (ic->op == '>' || ic->op == '<') && SPEC_USIGN(getSpec(operandType(left)))) && // Non-destructive comparison.
    (left_in_A && getSize(operandType(left)) == 1 && (IS_OP_LITERAL(right) || right_dir) || right_in_A && getSize(operandType(right)) == 1 && (IS_OP_LITERAL(left) || left_dir)))
    return (true);

  if (ic->op == IFX && (dying_A || left_in_A))
    return (true);

  if (ic->op == '<' && IS_OP_LITERAL(right) && !ullFromVal(OP_VALUE_CONST (right)) &&
    (operand_byte_in_reg(left, getSize(operandType(left)) - 1, REG_A, a, i, G) || operand_byte_in_reg(left, getSize(operandType(left)) - 1, REG_P, a, i, G)))
    return (true);

  if (ic->op == SET_VALUE_AT_ADDRESS && getSize(operandType(right)) == 1 && left_dir && right_in_A)
    return (true);
  if (ic->op == SET_VALUE_AT_ADDRESS && IS_ITEMP(left) && OP_SYMBOL_CONST(left)->remat && !operand_in_reg(left, REG_A, ia, i, G))
    return (true);

  if ((ic->op == '+' || ic->op == '-' || ic->op == UNARYMINUS) && (!left_in_A && !right_in_A || getSize(operandType(result)) == 1))
    return (true);

  if ((ic->op == CALL || ic->op == PCALL) && !left_in_A)
    return(true);

  if (ic->op == RETURN ||
    (ic->op == GET_VALUE_AT_ADDRESS && getSize(operandType(result)) <= 2 || ic->op == SET_VALUE_AT_ADDRESS && getSize(operandType(right)) == 1) && dying_A)
    return(true);

  if (ic->op == GET_VALUE_AT_ADDRESS && !left_in_A)
    return(true);

  if (ic->op == CAST &&
    (getSize(operandType(result)) == 1 || getSize(operandType(result)) == 2 && SPEC_USIGN (getSpec(operandType(right))) && operand_byte_in_reg(result, 1, REG_P, a, i, G)) &&
    right_in_A && (result_dir || dying_A))
    return (true);

  if ((ic->op == CAST && (getSize(operandType(result)) <= getSize(operandType(right)) || SPEC_USIGN(getSpec(operandType(right))))) &&
    getSize(operandType(result)) <= 2 &&
    (result_dir && dying_A || result_in_A && right_dir || result_in_A && right_in_A))
    return (true);

  if ((ic->op == LEFT_OP || ic->op == RIGHT_OP))
    return(IS_OP_LITERAL(right) || right_in_A && !result_in_A);

  if (ic->op == '^' &&
    (operand_byte_in_reg(result, 0, REG_A, a, i, G) && (operand_byte_in_reg(left, 0, REG_A, a, i, G) || operand_byte_in_reg(right, 0, REG_A, a, i, G)) ||
    operand_byte_in_reg(result, 1, REG_A, a, i, G) && (operand_byte_in_reg(left, 1, REG_A, a, i, G) || operand_byte_in_reg(right, 1, REG_A, a, i, G))))
    return (true);

  // For most operations only allow lower byte in a for now (upper byte for result).
  if (left_in_A && !operand_byte_in_reg(left, 0, REG_A, a, i, G) || right_in_A && !operand_byte_in_reg(right, 0, REG_A, a, i, G) ||
    ic->op != '+' && ic->op != '-' && ic->op != UNARYMINUS && result_in_A && !operand_byte_in_reg(result, getSize(operandType(result)) - 1, REG_A, a, i, G))
    return(false);

  if(dying_A)
    return(true);

  return(false);
}

template <class G_t, class I_t>
static bool Pinst_ok(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  const iCode *ic = G[i].ic;
  const i_assignment_t &ia = a.i_assignment;

  if(ia.registers[REG_P][1] < 0)
    return(true);       // Pseudoregister p not in use.

  const operand *left = IC_LEFT(ic);
  const operand *right = IC_RIGHT(ic);
  const operand *result = IC_RESULT(ic);

  bool left_in_P = operand_in_reg(left, REG_P, ia, i, G);
  bool right_in_P = operand_in_reg(right, REG_P, ia, i, G);
  bool result_in_P = operand_in_reg(result, REG_P, ia, i, G);

  bool left_in_A = operand_in_reg(left, REG_A, ia, i, G);
  bool right_in_A = operand_in_reg(right, REG_A, ia, i, G);
  bool result_in_A = operand_in_reg(result, REG_A, ia, i, G);

  const cfg_dying_t &dying = G[i].dying;

  bool dying_P = result_in_P || dying.find(ia.registers[REG_P][1]) != dying.end() || dying.find(ia.registers[REG_P][0]) != dying.end();

  bool left_stack = (IS_ITEMP (left) || IS_PARM (left)) && (options.stackAuto || reentrant) && !left_in_A && !left_in_P;
  bool right_stack = (IS_ITEMP (right) || IS_PARM (right)) && (options.stackAuto || reentrant) && !right_in_A && !right_in_P;
  bool result_stack = (IS_ITEMP (result) || IS_PARM (result)) && (options.stackAuto || reentrant) && !result_in_A && !result_in_P;

  if(result && IS_ITEMP(result) && OP_SYMBOL_CONST(result)->remat)
    return(true);

  if(ic->op == IPUSH && left_stack)
    return(false);

  // Arithmetic uses p internally for literal operands with multiple nonzero bytes.
  if ((ic->op == '+' || ic->op == '-' || ic->op == '!' || ic->op == '<' || ic->op == '>') && (IS_OP_LITERAL(left) || IS_OP_LITERAL(right)))
    {
      const operand *const litop = IS_OP_LITERAL(left) ? left : right;
      if ((ullFromVal(OP_VALUE_CONST (litop)) & 0x000000ffull) && (ullFromVal(OP_VALUE_CONST(litop)) & 0x0000ff00ull) && (ullFromVal(OP_VALUE_CONST (litop)) & 0x00ff0000ull) && (ullFromVal(OP_VALUE_CONST (litop)) & 0xff000000ull))
        return(false);
    }
  if (ic->op == PCALL)
    return(false);

  if (ic->op == CALL && !dying_P)
    return(false);

  if (ic->op == GET_VALUE_AT_ADDRESS && !dying_P && !(left_in_P && getSize(operandType(result)) == 1))
    return(false);

  if ((ic->op == '^' || ic->op == '|' || ic->op == BITWISEAND || ic->op == EQ_OP || ic->op == NE_OP) &&
    (left_stack || right_stack || result_stack && !dying_P))
    return(false);

  return(true);
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
          sym->regs[I[v].byte] = pdk_regs + a.global[v];   
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

// Check that the operand is either fully in registers or fully in memory. Todo: Relax this once code generation can handle partially spilt variables!
template <class G_t, class I_t>
static bool operand_sane(const operand *o, const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  if(!o || !IS_SYMOP(o))
    return(true);
 
  operand_map_t::const_iterator oi, oi_end;
  boost::tie(oi, oi_end) = G[i].operands.equal_range(OP_SYMBOL_CONST(o)->key);
  
  if(oi == oi_end)
    return(true);
  
  // In registers.
  if(std::binary_search(a.local.begin(), a.local.end(), oi->second))
    {
      while(++oi != oi_end)
        if(!std::binary_search(a.local.begin(), a.local.end(), oi->second))
          return(false);
    }
  else
    {
       while(++oi != oi_end)
        if(std::binary_search(a.local.begin(), a.local.end(), oi->second))
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

// Cost function.
template <class G_t, class I_t>
static float instruction_cost(const assignment &a, unsigned short int i, const G_t &G, const I_t &I)
{
  iCode *ic = G[i].ic;
  float c;

  wassert(TARGET_PDK_LIKE);
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

  if(!Pinst_ok(a, i, G, I))
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
    //case GETABIT:
    //case GETBYTE:
    //case GETWORD:
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
      c = dryPdkiCode(ic);

      if (IC_RESULT (ic) && IS_ITEMP (IC_RESULT(ic)) && !OP_SYMBOL_CONST(IC_RESULT(ic))->remat && // Nudge towards saving RAM space. TODO: Do this in a better way, so it works for all backends!
        !operand_in_reg(IC_RESULT(ic), REG_A, a.i_assignment, i, G) && !operand_in_reg(IC_RESULT(ic), REG_P, a.i_assignment, i, G)) 
        c += 0.0001;

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

// Suggest to honor register keyword.
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
    }

  return(c);
}

// Code for another ic is generated when generating this one. Mark the other as generated.
static void extra_ic_generated(iCode *ic)
{
  iCode *ifx;

  // - can only jump on nonzero result for decrement of register / direct variable.
  if(ic->op == '-' && ic->next && ic->next->op == IFX && IC_COND (ic->next)->key == IC_RESULT(ic)->key)
    {
      ifx = ic->next;

      if ((!IS_ITEMP(IC_LEFT (ic)) || options.stackAuto || reentrant) && !isOperandGlobal (IC_LEFT (ic)))
        return;

      if (!IS_OP_LITERAL(IC_RIGHT(ic)))
        return;

      if (ullFromVal(OP_VALUE(IC_RIGHT(ic))) != 1)
        return;

      if (!isOperandEqual (IC_RESULT(ic), IC_LEFT(ic)))
        return;

      ifx->generated = true;
      return;
    }

  if(ic->op != EQ_OP && ic->op != NE_OP && ic->op != '<' && ic->op != '>' && ic->op != BITWISEAND)
    return;

  ifx = ifxForOp(IC_RESULT(ic), ic);

  if(!ifx)
    return;

  // Bitwise and code generation can only do the jump if there is at most one nonzero byte.
  if(ic->op == BITWISEAND)
    {
      int nonzero = 0;
      operand *const litop = IS_OP_LITERAL(IC_LEFT(ic)) ? IC_LEFT(ic) : IC_RIGHT(ic);

      if (!IS_OP_LITERAL(litop))
        return;

      for(unsigned int i = 0; i < getSize(operandType(IC_LEFT (ic))) && i < getSize(operandType(IC_RIGHT(ic))) && i < getSize(operandType(IC_RESULT(ic))); i++)
        if(byteOfVal(OP_VALUE(litop), i))
          nonzero++;

      if(nonzero > 1 && IC_FALSE (ifx))
        return;
    }

cnd:
  OP_SYMBOL(IC_RESULT(ic))->for_newralloc = false;
  OP_SYMBOL(IC_RESULT(ic))->regType = REG_CND;
  ifx->generated = true;
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
        sym->regs[I[v].byte] = pdk_regs + winner.global[v];   
      else
        {
          sym->regs[I[v].byte] = 0;
          spilt = true;
        }

      if(spilt)
        pdkSpillThis(sym);

      sym->nRegs = I[v].size;
    }

  for(unsigned int i = 0; i < boost::num_vertices(G); i++)
    set_surviving_regs(winner, i, G, I);

  set_spilt(G, I, SI);

  return(!assignment_optimal);
}

iCode *pdk_ralloc2_cc(ebbIndex *ebbi)
{
  eBBlock **const ebbs = ebbi->bbOrder;
  const int count = ebbi->count;
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

  get_nice_tree_decomposition(tree_decomposition, control_flow_graph);

  alive_tree_dec(tree_decomposition, control_flow_graph);

  good_re_root(tree_decomposition);
  nicify(tree_decomposition);
  alive_tree_dec(tree_decomposition, control_flow_graph);

  if(options.dump_graphs)
    dump_tree_decomposition(tree_decomposition);

  guessCounts (ic, ebbi);

  scon_t spilt_conflict_graph;

  pdk_assignment_optimal = !tree_dec_ralloc(tree_decomposition, control_flow_graph, conflict_graph, spilt_conflict_graph);

  pdkRegFix (ebbs, count);

  if (reentrant)
    {
      chaitin_salloc(spilt_conflict_graph);

      if(options.dump_graphs)
        dump_scon(spilt_conflict_graph);
    }
  else
    doOverlays (ebbs, count);

  return(ic);
}

