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

// #define DEBUG_LOSPRE // Uncomment to get debug messages while doing lospre.
// #define DEBUG_LOSPRE_ASS // Uncomment to get debug messages on considered assignmentd while doing lospre.

#include "SDCClospre.hpp"

// A quick-and-dirty function to get the CFG from sdcc (a simplified version of the function from SDCCralloc.hpp).
void
create_cfg_lospre (cfg_lospre_t &cfg, iCode *start_ic, ebbIndex *ebbi)
{
  iCode *ic;

  std::map<int, unsigned int> key_to_index;
  {
    int i;

    for (ic = start_ic, i = 0; ic; ic = ic->next, i++)
      {
        boost::add_vertex(cfg);
        key_to_index[ic->key] = i;
        cfg[i].ic = ic;
      }
  }

  // Get control flow graph from sdcc.
  for (ic = start_ic; ic; ic = ic->next)
    {
      if((ic->op == '>' || ic->op == '<' || ic->op == LE_OP || ic->op == GE_OP || ic->op == EQ_OP || ic->op == NE_OP || ic->op == '^' || ic->op == '|' || ic->op == BITWISEAND) && ifxForOp (IC_RESULT (ic), ic))
        boost::add_edge(key_to_index[ic->key], key_to_index[ic->next->key], 4.0f, cfg); // Try not to separate op from ifx.
      else if (ic->op != GOTO && ic->op != RETURN && ic->op != JUMPTABLE && ic->next)
        boost::add_edge(key_to_index[ic->key], key_to_index[ic->next->key], 3.0f, cfg);

      if (ic->op == GOTO)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, ic->label)->sch->key], 6.0f, cfg);
      else if (ic->op == RETURN)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, returnLabel)->sch->key], 6.0f, cfg);
      else if (ic->op == IFX)
        boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, IC_TRUE(ic) ? IC_TRUE(ic) : IC_FALSE(ic))->sch->key], 6.0f, cfg);
      else if (ic->op == JUMPTABLE)
        for (symbol *lbl = (symbol *)(setFirstItem (IC_JTLABELS (ic))); lbl; lbl = (symbol *)(setNextItem (IC_JTLABELS (ic))))
          boost::add_edge(key_to_index[ic->key], key_to_index[eBBWithEntryLabel(ebbi, lbl)->sch->key], 6.0f, cfg);
    }
}

static bool
candidate_expression (const iCode *const ic, int lkey)
{
  wassert(ic);

  if (
    ic->op != '!' &&
    ic->op != '~' &&
    ic->op != UNARYMINUS &&
    ic->op != '+' &&
    ic->op != '-' &&
    ic->op != '*' &&
    ic->op != '/' &&
    ic->op != '%' &&
    ic->op != '>' &&
    ic->op != '<' &&
    ic->op != LE_OP &&
    ic->op != GE_OP &&
    ic->op != NE_OP &&
    ic->op != EQ_OP &&
    ic->op != AND_OP &&
    ic->op != OR_OP &&
    ic->op != '^' &&
    ic->op != '|' &&
    ic->op != BITWISEAND &&
    ic->op != RRC &&
    ic->op != RLC &&
    ic->op != GETABIT &&
    ic->op != GETHBIT &&
    ic->op != LEFT_OP &&
    ic->op != RIGHT_OP &&
    !(ic->op == '=' && !POINTER_SET(ic) && !(IS_ITEMP(IC_RIGHT(ic)) /*&& IC_RIGHT(ic)->key > lkey*/)) &&
    ic->op != GET_VALUE_AT_ADDRESS &&
    ic->op != CAST /*&&
    ic->op != ADDRESS_OF Apparently typically not worth the cost in code size*/)
    return (false);

  const operand *const left = IC_LEFT (ic);
  const operand *const right = IC_RIGHT (ic);
  const operand *const result = IC_RESULT (ic);

  // Todo: Allow literal right operand once backends can rematerialize literals!
  if(ic->op == '=' && IS_OP_LITERAL (right))
    return (false);

  // Todo: Allow more operands!
  if (ic->op != CAST && left && !(IS_SYMOP (left) || IS_OP_LITERAL (left)) ||
    right && !(IS_SYMOP (right) || IS_OP_LITERAL (right)) ||
    result && !(IS_SYMOP (result) || IS_OP_LITERAL (result)))
    return (false);

  return (true);
}

static bool
same_expression (const iCode *const lic, const iCode *const ric)
{
  wassert(lic);
  wassert(ric);

  if (lic->op != ric->op)
    return (false);

  const operand *lleft = IC_LEFT (lic);
  const operand *lright = IC_RIGHT (lic);
  const operand *lresult = IC_RESULT (lic);
  const operand *rleft = IC_LEFT (ric);
  const operand *rright = IC_RIGHT (ric);
  const operand *rresult = IC_RESULT (ric);

  if ((isOperandEqual (lleft, rleft) && isOperandEqual (lright, rright) ||
    IS_COMMUTATIVE (lic) && isOperandEqual (lleft, rright) && isOperandEqual (lright, rleft)) &&
    (lresult && rresult && compareTypeInexact (operandType (lresult), operandType (rresult)) > 0) &&
    IS_FLOAT (operandType (lresult)) == IS_FLOAT (operandType (rresult)))
    return (true);

  return (false);
}

static void
get_candidate_set(std::set<int> *c, const iCode *const sic, int lkey)
{
  // TODO: For loop invariant code motion allow expression that only occurs once, too - will be needed when optimizing for speed.
  for (const iCode *ic = sic; ic; ic = ic->next)
    {
      if (!candidate_expression (ic, lkey))
        continue;
      for (const iCode *pic = sic; pic != ic; pic = pic->next)
        if (candidate_expression (pic, lkey) && same_expression (ic, pic) && c->find (pic->key) == c->end ())
          {
            // Found expression that occurs at least twice.
            c->insert (pic->key);
            break;
          }
    }
}

static bool
invalidates_expression(const iCode *const eic, const iCode *const iic)
{
  const operand *const eleft = IC_LEFT (eic);
  const operand *const eright = IC_RIGHT (eic);
  const bool uses_global = (eic->op == GET_VALUE_AT_ADDRESS || isOperandGlobal (eleft) || isOperandGlobal (eright) || IS_SYMOP (eleft) && OP_SYMBOL_CONST (eleft)->addrtaken || IS_SYMOP (eright) && OP_SYMBOL_CONST (eright)->addrtaken);
  const bool uses_volatile = POINTER_GET (eic) && IS_VOLATILE (operandType (eleft)->next) || IS_OP_VOLATILE (eleft) || IS_OP_VOLATILE (eright);

  const operand *const left = IC_LEFT (iic);
  const operand *const right = IC_RIGHT (iic);
  const operand *const result = IC_RESULT (iic);

  if (iic->op == FUNCTION || iic->op == ENDFUNCTION || iic->op == RECEIVE)
    return(true);
  if (eic->op == ADDRESS_OF) // ADDRESS_OF does not really read its operand.
    return(false);
  if (eic->op == GET_VALUE_AT_ADDRESS && (isOperandGlobal (IC_RESULT (iic)) || IS_SYMOP (IC_RESULT (iic)) && OP_SYMBOL_CONST (IC_RESULT (iic))->addrtaken))
    return(true);
  if (IC_RESULT (iic) && !IS_OP_LITERAL (result) && !POINTER_SET(iic) &&
    (eleft && isOperandEqual (eleft, result) || eright && isOperandEqual (eright, result)))
    return(true);
  if ((uses_global || uses_volatile) && (iic->op == CALL || iic->op == PCALL))
    return(true);
  if (uses_volatile && (POINTER_GET (iic) && IS_VOLATILE (operandType (left)->next)) || IS_OP_VOLATILE (left) || IS_OP_VOLATILE (right))
    return(true);
  if (uses_global && POINTER_SET (iic)) // TODO: More accuracy here!
    return(true);

  return(false);
}

static bool
setup_cfg_for_expression (cfg_lospre_t *const cfg, const iCode *const eic)
{
  typedef boost::graph_traits<cfg_lospre_t>::vertex_descriptor vertex_t;
  bool safety_required = false;

  // In redundancy elimination, safety means not doing a computation on any path were it was not done before.
  // This is important, if the compuation can have side-effects, which depends on the target architecure.
  // E.g. On some systems division requires safety, since division by zero might result in an interrupt.
  // When there are memory-mapped devices or there is memory management, reading from a pointer requires
  // safety, since reading from an unknown location could result in making the device do something or in a SIGSEGV. 
  // On the other hand, addition is something that typically does not require safety, since adding two undefined
  // operands gives just another undefined (the C standard allows trap representations, which, could result
  // in addition requiring safety though; AFAIK none of the targets currently supported by SDCC have trap representations).
  // Philipp, 2012-07-06.
  //
  // For now we just always require safety for "dangerous" operations.
  //
  // TODO: Replace the current one  by a more exact mechanism, that takes into account information from
  // (not yet implemented) generalized constant propagation, pointer analysis, etc.

  // Function calls can have any side effects.
  if (eic->op == CALL || eic->op == PCALL)
    safety_required = true;

  // volatile requires safety.
  if (POINTER_GET (eic) && IS_VOLATILE (operandType (IC_LEFT (eic))->next) || IS_OP_VOLATILE (IC_LEFT (eic)) || IS_OP_VOLATILE (IC_RIGHT (eic)))
    safety_required = true;

  // Reading from an invalid address might be dangerous, since there could be memory-mapped I/O.
  if (eic->op == GET_VALUE_AT_ADDRESS && !optimize.allow_unsafe_read)
    safety_required = true;

  // The division routines for z80-like ports and the hc08/s08's and stm8's hardware division just give an undefined result
  // for division by zero, but there are no harmful side effects. I don't know about the other ports.
  if ((eic->op == '/' || eic->op == '%') && !TARGET_Z80_LIKE && !TARGET_HC08_LIKE && !TARGET_IS_STM8)
    safety_required = true;

  // TODO: Relax this! There are cases where allowing unsafe optimizations will improve speed.
  // This probably needs implementation of profile-guided optimization though.
  if (optimize.codeSpeed)
    safety_required = true;

#ifdef DEBUG_LOSPRE
  std::cout << "Invalidation set I: ";
#endif
  for (vertex_t i = 0; i < boost::num_vertices (*cfg); i++)
    {
       const iCode *const ic = (*cfg)[i].ic;

       (*cfg)[i].uses = same_expression (eic, ic);
       (*cfg)[i].invalidates = invalidates_expression (eic, ic);

       (*cfg)[i].forward = std::pair<int, int>(-1, -1);

#ifdef DEBUG_LOSPRE
       if ((*cfg)[i].invalidates)
         std::cout << i << ", ";
#endif
    }
#ifdef DEBUG_LOSPRE
  std::cout << "\n";
#endif

  return (safety_required);
}

// Dump cfg, with numbered nodes.
void dump_cfg_lospre (const cfg_lospre_t &cfg)
{
  if (!currFunc)
    return;

  std::ofstream dump_file((std::string(dstFileName) + ".dumplosprecfg" + currFunc->rname + ".dot").c_str());

  std::string *name = new std::string[num_vertices(cfg)];
  for (unsigned int i = 0; i < boost::num_vertices(cfg); i++)
    {
      const char *iLine = printILine (cfg[i].ic);
      std::ostringstream os;
      os << i << ", " << cfg[i].ic->key << " : " << iLine;
      dbuf_free (iLine);
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, cfg, boost::make_label_writer(name), boost::default_writer(), cfg_titlewriter(currFunc->rname, "lospre"));
  delete[] name;
}

// Dump tree decomposition.
static void dump_dec_lospre(const tree_dec_t &tree_dec)
{
  wassert (currFunc);

  std::ofstream dump_file((std::string(dstFileName) + ".dumplospredec" + currFunc->rname + ".dot").c_str());

  unsigned int w = 0;

  std::string *name = new std::string[num_vertices(tree_dec)];
  for (unsigned int i = 0; i < boost::num_vertices(tree_dec); i++)
    {
      if (tree_dec[i].bag.size() > w)
        w = tree_dec[i].bag.size();
      std::ostringstream os;
      typename decltype(tree_dec[0].bag)::const_iterator v1;
       os << i << " | ";
      for (v1 = tree_dec[i].bag.begin(); v1 != tree_dec[i].bag.end(); ++v1)
        os << *v1 << " ";
      name[i] = os.str();
    }
  boost::write_graphviz(dump_file, tree_dec, boost::make_label_writer(name));
  delete[] name;
}

void
lospre (iCode *sic, ebbIndex *ebbi)
{
  cfg_lospre_t control_flow_graph;
  tree_dec_t tree_decomposition;

  wassert (sic);

#ifdef DEBUG_LOSPRE
  if (currFunc)
    std::cout << "lospre for " << currFunc->rname << "()\n";
#endif

  create_cfg_lospre (control_flow_graph, sic, ebbi);

  if(options.dump_graphs)
    dump_cfg_lospre(control_flow_graph);

  get_nice_tree_decomposition (tree_decomposition, control_flow_graph);

  if(options.dump_graphs)
    dump_dec_lospre(tree_decomposition);

  int lkey = operandKey;

  for (bool change = true; change;)
    {
      change = false;

      std::set<int> candidate_set;
      get_candidate_set (&candidate_set, sic, lkey);

      std::set<int>::iterator ci, ci_end;
      for (ci = candidate_set.begin(), ci_end = candidate_set.end(); ci != ci_end; ++ci)
        {
          const iCode *ic;
          for (ic = sic; ic && ic->key != *ci; ic = ic->next);

          if (!ic || !candidate_expression (ic, lkey))
            continue;

          bool safety = setup_cfg_for_expression (&control_flow_graph, ic);

          if (safety && tree_dec_safety (tree_decomposition, control_flow_graph, ic) < 0)
            continue;

          change |= (tree_dec_lospre (tree_decomposition, control_flow_graph, ic) > 0);
        }
    }
}

