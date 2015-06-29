/*-------------------------------------------------------------------------

   pcoderegs.c - post code generation register optimizations

   Written By -  Scott Dattalo scott@dattalo.com

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

/*
pcoderegs.c

  The purpose of the code in this file is to optimize the register usage.

*/

#include "main.h"
#include "pcoderegs.h"
#include "pcodeflow.h"
#include "ralloc.h"


static int total_registers_saved=0;
static int register_optimization=1;

/*-----------------------------------------------------------------*
* void pCodeRegMapLiveRangesInFlow(pCodeFlow *pcfl)
*-----------------------------------------------------------------*/
static void pCodeRegMapLiveRangesInFlow(pCodeFlow *pcfl)
{
	
	pCode *pc=NULL;
	
	reg_info *reg;
	
	if(!pcfl)
		return;
	
	
	pc = findNextInstruction(pcfl->pc.next);
	
	while(pc && !isPCFL(pc)) {
		while (pc && !isPCI(pc) && !isPCFL(pc))
		{
			pc = pc->next;
		} // while
		if (!pc || isPCFL(pc)) continue;
		assert( isPCI(pc) );

		reg = getRegFromInstruction(pc);
		#if 0
		pc->print(stderr, pc);
		fprintf( stderr, "--> reg %p (%s,%u), inCond/outCond: %x/%x\n",
			reg, reg ? reg->name : "(null)", reg ? reg->rIdx : -1,
			PCI(pc)->inCond, PCI(pc)->outCond );
		#endif
		if(reg) {
		/*
		fprintf(stderr, "flow seq %d, inst seq %d  %s  ",PCODE(pcfl)->seq,pc->seq,reg->name);
		fprintf(stderr, "addr = 0x%03x, type = %d  rIdx=0x%03x\n",
		reg->address,reg->type,reg->rIdx);
			*/
			
			addSetIfnotP(& (PCFL(pcfl)->registers), reg);
			
			if(PCC_REGISTER & PCI(pc)->inCond)
				addSetIfnotP(& (reg->reglives.usedpFlows), pcfl);
			
			if(PCC_REGISTER & PCI(pc)->outCond)
				addSetIfnotP(& (reg->reglives.assignedpFlows), pcfl);
			
			addSetIfnotP(& (reg->reglives.usedpCodes), pc);
			reg->wasUsed = 1;
		}
		
		
		//pc = findNextInstruction(pc->next);
		pc = pc->next;
		
	}

}

/*-----------------------------------------------------------------*
* void pCodeRegMapLiveRanges(pBlock *pb) 
*-----------------------------------------------------------------*/
void pCodeRegMapLiveRanges(pBlock *pb)
{
	pCode *pcflow;
	
	for( pcflow = findNextpCode(pb->pcHead, PC_FLOW); 
	pcflow != NULL;
	pcflow = findNextpCode(pcflow->next, PC_FLOW) ) {
		
		if(!isPCFL(pcflow)) {
			fprintf(stderr, "pCodeRegMapLiveRanges - pcflow is not a flow object ");
			continue;
		}
		pCodeRegMapLiveRangesInFlow(PCFL(pcflow));
	}
	
#if 0
	for( pcflow = findNextpCode(pb->pcHead, PC_FLOW); 
	pcflow != NULL;
	pcflow = findNextpCode(pcflow->next, PC_FLOW) ) {
		
		regs *r = setFirstItem(PCFL(pcflow)->registers);
		fprintf(stderr,"flow seq %d\n", pcflow->seq);
		
		while (r) {
			fprintf(stderr, "  %s\n",r->name);
			r = setNextItem(PCFL(pcflow)->registers);
			
		}
		
	}
#endif
	
}


/*-----------------------------------------------------------------*
*
*-----------------------------------------------------------------*/
static void Remove1pcode(pCode *pc, reg_info *reg, int debug_code)
{

	pCode *pcn=NULL;
	
	if(!reg || !pc)
		return;

	deleteSetItem (&(reg->reglives.usedpCodes),pc);
	
	if(PCI(pc)->label) {
		pcn = findNextInstruction(pc->next);
		
		if(pcn)
			PCI(pcn)->label = pBranchAppend(PCI(pcn)->label,PCI(pc)->label);
	}
	
	if(PCI(pc)->cline) {
		if(!pcn)
			pcn = findNextInstruction(pc->next);
		
		if(pcn) {
			if(PCI(pcn)->cline) {
				//fprintf(stderr, "source line has been optimized completely out\n");
				//pc->print(stderr,pc);
			} else {
				PCI(pcn)->cline = PCI(pc)->cline;
			}
		}
	}
	
	
	if(1) {
                /*
                 * Debug stuff. Comment out the instruction we're about to delete.
                 */
		
		char buff1[256];
		size_t size = 256;
		
		char *pbuff;
		pbuff = &buff1[0];
		
		SNPRINTF(pbuff, size, ";%d", debug_code);
		size -= strlen(pbuff);
		pbuff += strlen(pbuff);
		pCode2str(pbuff, size, pc);
		pCodeInsertBefore(pc, newpCodeCharP(buff1));
		//fprintf(stderr,"removing instruction:\n%s\n",buff1);
	}
	
	pc->destruct(pc);
	
}

/*-----------------------------------------------------------------*
* void RemoveRegsFromSet(set *regset)
*
*-----------------------------------------------------------------*/
static void RemoveRegsFromSet(set *regset)
{
	reg_info *reg;
	int used;
	
	while(regset) {
		reg = regset->item;
		regset = regset->next;
		
		used = elementsInSet(reg->reglives.usedpCodes);
		
		if(used <= 1) {
			
			//fprintf(stderr," reg %s isfree=%d, wasused=%d\n",reg->name,reg->isFree,reg->wasUsed);
			if(used == 0) {
				//fprintf(stderr," getting rid of reg %s\n",reg->name);
				reg->isFree = 1;
				reg->wasUsed = 0;
			} else {
				pCode *pc;
				
				
				pc = setFirstItem(reg->reglives.usedpCodes);
				
				if(reg->type == REG_SFR || reg->type == REG_STK || reg->isPublic || reg->isExtern) {
					//fprintf(stderr, "not removing SFR reg %s even though used only once\n",reg->name);
					continue;
				}
				
				
				if(isPCI(pc)) {
					if(PCI(pc)->label) {
						pCode *pcn = findNextInstruction(pc->next);
						
						if(pcn && PCI(pcn)->label) {
							//fprintf(stderr,"can't delete instruction with label...\n");
							//pc->print(stderr,pc);
							continue;
						} 
						/* Move the label to the next instruction */
						
						PCI(pcn)->label = PCI(pc)->label;
						
					}
					
					if(isPCI_SKIP(pc)) {
						reg_info *r = getRegFromInstruction(pc);
						fprintf(stderr, "WARNING, a skip instruction is being optimized out\n");
						pc->print(stderr,pc);
						fprintf(stderr,"reg %s, type =%d\n",r->name, r->type);
					}
					//fprintf(stderr," removing reg %s because it is used only once\n",reg->name);
					Remove1pcode(pc, reg, 1);
					/*
					unlinkpCode(pc);
					deleteSetItem (&(reg->reglives.usedpCodes),pc);
					*/
					reg->isFree = 1;
					reg->wasUsed = 0;
					total_registers_saved++;  // debugging stats.
				}
			}
		}
		
	}
}

static void pic14_ReMapLiveRanges(void)
{
	pBlock *pb;
	if (!the_pFile) return;
	RegsUnMapLiveRanges();
	for (pb = the_pFile->pbHead; pb; pb = pb->next)
	{
	#if 0
		pCode *pc = findNextpCode(pb->pcHead, PC_FLOW);
		if (pc) {
			pc->print( stderr, pc );
		} else {
			fprintf( stderr, "unnamed pBlock\n");
		}
		pc = findNextInstruction(pb->pcHead);
		while (pc) {
		  pc->print( stderr, pc );
		  pc = findNextInstruction(pc->next);;
		}
	#endif	
		pCodeRegMapLiveRanges(pb);
	} // for
}

/*-----------------------------------------------------------------*
* void RemoveUnusedRegisters(void)
*
*-----------------------------------------------------------------*/
void RemoveUnusedRegisters(void)
{
	/* First, get rid of registers that are used only one time */
	pic14_ReMapLiveRanges();
	
	//RemoveRegsFromSet(dynInternalRegs);
	RemoveRegsFromSet(dynAllocRegs);
	RemoveRegsFromSet(dynStackRegs);
	/*
	don't do DirectRegs yet - there's a problem with arrays
	RemoveRegsFromSet(dynDirectRegs);
	*/
	RemoveRegsFromSet(dynDirectBitRegs);
	
	if(total_registers_saved) DFPRINTF((stderr, " *** Saved %d registers ***\n", total_registers_saved));
}


/*-----------------------------------------------------------------*
*
*-----------------------------------------------------------------*/
static void Remove2pcodes(pCode *pcflow, pCode *pc1, pCode *pc2, reg_info *reg, int can_free)
{
	static int debug_code=99;
	if(!reg)
		return;
#if 0
	fprintf (stderr, "%s:%d(%s): %d (reg:%s)\n", __FILE__, __LINE__, __FUNCTION__, debug_code, reg ? reg->name : "???");
	printpCode (stderr, pc1);
	printpCode (stderr, pc2);
#endif
	
	//fprintf(stderr,"%s\n",__FUNCTION__);
	if(pc1)
		Remove1pcode(pc1, reg, debug_code++);
	
	if(pc2) {
		Remove1pcode(pc2, reg, debug_code++);
		deleteSetItem (&(PCFL(pcflow)->registers), reg);
		
		if(can_free) {
			reg->isFree = 1;
			reg->wasUsed = 0;
		}
		
	}
	
	pCodeRegMapLiveRangesInFlow(PCFL(pcflow));
}

/*-----------------------------------------------------------------*
*
*-----------------------------------------------------------------*/
static int regUsedinRange(pCode *pc1, pCode *pc2, reg_info *reg)
{
	int i=0;
	reg_info *testreg;
	
	do {
		testreg = getRegFromInstruction(pc1);
		if(testreg && (testreg->rIdx == reg->rIdx)) {
			return 1;
		}
		if (i++ > 1000) {
			fprintf(stderr, "warning, regUsedinRange searched through too many pcodes\n");
			return 0;
		}
		
		pc1 = findNextInstruction(pc1->next);
		
	} while (pc1 && (pc1 != pc2)) ;
	
	return 0;
}

static int regIsSpecial (reg_info *reg, int mayBeGlobal)
{
  if (!reg) return 0;

  if (reg->type == REG_SFR || reg->type == REG_STK || (!mayBeGlobal && (reg->isPublic || reg->isExtern))) return 1;

  return 0;
}

/*-----------------------------------------------------------------*
* void pCodeOptime2pCodes(pCode *pc1, pCode *pc2) 
*
* ADHOC pattern checking 
* Now look for specific sequences that are easy to optimize.
* Many of these sequences are characteristic of the compiler
* (i.e. it'd probably be a waste of time to apply these adhoc
* checks to hand written assembly.)
* 
*
*-----------------------------------------------------------------*/
static int pCodeOptime2pCodes(pCode *pc1, pCode *pc2, pCode *pcfl_used, reg_info *reg, int can_free, int optimize_level)
{
	pCode *pct1, *pct2;
	reg_info  *reg1, *reg2;
	
	int t = total_registers_saved;

	if (!isPCI(pc1) || !isPCI(pc2)) return 0;
	if (PCI(pc1)->pcflow != PCI(pc2)->pcflow) return 0;
	
	if (pc2->seq < pc1->seq) {
		pct1 = pc2;
		pc2 = pc1;
		pc1 = pct1;
	}

	/* disable this optimization for now -- it's buggy */
	if (pic14_options.disable_df) return 0;
	
	//fprintf(stderr,"pCodeOptime2pCodes\n");
	//pc1->print(stderr,pc1);
	//pc2->print(stderr,pc2);

	if((PCI(pc1)->op == POC_CLRF) && (PCI(pc2)->op == POC_MOVFW) ){
	    /*
	     * CLRF sets Z
	     * MOVFW affects Z
	     * MOVWF does not touch Z
	     * MOVLW does not touch Z
	     */
		pCode *newpc;
		/*
		clrf  reg    ; pc1
		stuff...
		movf  reg,w  ; pc2
		
		  can be replaced with (only if following instructions are not going to use W and reg is not used again later)
		  
			stuff...
			movlw 0 or clrf  reg
		*/
		DFPRINTF((stderr, "   optimising CLRF reg ... MOVF reg,W to ... MOVLW 0\n"));
		pct2 = findNextInstruction(pc2->next);
		if (pCodeSearchCondition(pct2, PCC_Z, 0) == -1) {
			/* Z is definitely overwritten before use */
			newpc = newpCode(POC_MOVLW, newpCodeOpLit(0));
			
			pCodeInsertAfter(pc2, newpc);
			PCI(newpc)->pcflow = PCFL(pcfl_used);
			newpc->seq = pc2->seq;
			
			//fprintf (stderr, "%s:%d(%s): Remove2pcodes (CLRF reg, ..., MOVF reg,W)\n", __FILE__, __LINE__, __FUNCTION__);
			//Remove2pcodes(pcfl_used, pc2, NULL, reg, 0);
			pc2->destruct(pc2);
			//total_registers_saved++;  // debugging stats.
		}
	} else if((PCI(pc1)->op == POC_CLRF) && (PCI(pc2)->op == POC_IORFW) ){
		DFPRINTF((stderr, "   optimising CLRF/IORFW\n"));
		
		pct2 = findNextInstruction(pc2->next);
		
		/* We must ensure that Z is destroyed before being read---IORLW must be performed unless this is proven. */
		if (pCodeSearchCondition(pct2, PCC_Z, 0) != -1) {
			pct2 = newpCode(POC_IORLW, newpCodeOpLit(0));
			pct2->seq = pc2->seq;
			PCI(pct2)->pcflow = PCFL(pcfl_used);
			pCodeInsertAfter(pc1,pct2);
		}
		//fprintf (stderr, "%s:%d(%s): Remove2pcodes (CLRF/IORFW)\n", __FILE__, __LINE__, __FUNCTION__);
		Remove2pcodes(pcfl_used, pc1, pc2, reg, can_free);
		total_registers_saved++;  // debugging stats.
		
	}  else if(PCI(pc1)->op == POC_MOVWF) {
		// Optimising MOVWF reg ...
		
		pct2 = findNextInstruction(pc2->next);
		
		if(PCI(pc2)->op == POC_MOVFW) {
			// Optimising MOVWF reg ... MOVF reg,W
			
			if(PCI(pct2)->op == POC_MOVWF) {
			/*
			Change:
			
			  movwf   reg    ; pc1
			  stuff...
			  movf    reg,w  ; pc2
			  movwf   reg2   ; pct2
			  
				To: ( as long as 'stuff' does not use reg2 or if following instructions do not use W or reg is not used later)
				
				  movwf   reg2
				  stuff...
				  
				*/
				reg2 = getRegFromInstruction(pct2);
				/* Check reg2 is not used for something else before it is loaded with reg */
				if (reg2 && !regIsSpecial (reg2, 1) && !regUsedinRange(pc1,pc2,reg2)) {
					pCode *pct3 = findNextInstruction(pct2->next);
					/* Check following instructions are not relying on the use of W or the Z flag condiction */
					/* XXX: We must ensure that this value is destroyed before use---otherwise it might be used in
					 *      subsequent flows (checking for < 1 is insufficient). */
					if ((pCodeSearchCondition(pct3,PCC_Z,0) == -1) && (pCodeSearchCondition(pct3,PCC_W,0) == -1)) {
						DFPRINTF((stderr, "   optimising MOVF reg ... MOVF reg,W MOVWF reg2 to MOVWF reg2 ...\n"));
						pct2->seq = pc1->seq;
						unlinkpCode(pct2);
						pCodeInsertBefore(pc1,pct2);
						if(regUsedinRange(pct2,0,reg))
						{
							//fprintf (stderr, "%s:%d(%s): Remove2pcodes IF (MOVWF reg, ..., MOVW reg,W  MOVWF reg2)\n", __FILE__, __LINE__, __FUNCTION__);
							Remove2pcodes(pcfl_used, pc2, NULL, reg, can_free);
						} else {
							//fprintf (stderr, "%s:%d(%s): Remove2pcodes ELSE (MOVWF reg, ..., MOVW reg,W  MOVWF reg2)\n", __FILE__, __LINE__, __FUNCTION__);
							Remove2pcodes(pcfl_used, pc1, pc2, reg, can_free);
						}
						total_registers_saved++;  // debugging stats.
						return 1;
					}
				}
			}
		}
		
		pct1 = findPrevInstruction(pc1->prev);
		if(pct1 && (PCI(pct1)->pcflow == PCI(pc1)->pcflow)) {
			
			if ( (PCI(pct1)->op == POC_MOVFW) &&
				(PCI(pc2)->op == POC_MOVFW)) {
				
				reg1 = getRegFromInstruction(pct1);
				if(reg1 && !regIsSpecial (reg1, 0) && !regUsedinRange(pc1,pc2,reg1)) {
					DFPRINTF((stderr, "   optimising MOVF reg1,W MOVWF reg ... MOVF reg,W\n"));
					/*
					Change:
					
						movf   reg1,w
						movwf  reg

						stuff...
						movf   reg,w
						
					To:

						stuff...

						movf   reg1,w

					Or, if we're not deleting the register then the "To" is:

						stuff...

						movf   reg1,w
						movwf  reg
					*/
					pct2 = newpCode(PCI(pc2)->op, PCI(pct1)->pcop);
					pCodeInsertAfter(pc2, pct2);
					PCI(pct2)->pcflow = PCFL(pcfl_used);
					pct2->seq = pc2->seq;
					
					if(can_free) {
						//fprintf (stderr, "%s:%d(%s): Remove2pcodes CANFREE (MOVF reg1,W; MOVWF reg2; MOVF reg2,W)\n", __FILE__, __LINE__, __FUNCTION__);
						Remove2pcodes(pcfl_used, pc1, pc2, reg, can_free);
					} else {
					/* If we're not freeing the register then that means (probably)
						* the register is needed somewhere else.*/
						unlinkpCode(pc1);
						pCodeInsertAfter(pct2, pc1);
						
						//fprintf (stderr, "%s:%d(%s): Remove2pcodes ELSE (MOVF reg1,W; MOVWF reg2; MOVF reg2,W)\n", __FILE__, __LINE__, __FUNCTION__);
						Remove2pcodes(pcfl_used, pc2, NULL, reg, can_free);
					}
					
					//fprintf (stderr, "%s:%d(%s): Remove2pcodes ALWAYS (MOVF reg1,W; MOVWF reg2; MOVF reg2,W)\n", __FILE__, __LINE__, __FUNCTION__);
					Remove2pcodes(pcfl_used, pct1, NULL, reg1, 0);
					total_registers_saved++;  // debugging stats.
					
				}
			}
		}
  }

  return (total_registers_saved != t);
}

/*-----------------------------------------------------------------*
* void pCodeRegOptimeRegUsage(pBlock *pb) 
*-----------------------------------------------------------------*/
static void OptimizeRegUsage(set *fregs, int optimize_multi_uses, int optimize_level)
{
	reg_info *reg;
	int used;
	pCode *pc1=NULL, *pc2=NULL;
	
	
	while(fregs) {
		pCode *pcfl_used, *pcfl_assigned;
		
		/* Step through the set by directly accessing the 'next' pointer.
		* We could also step through by using the set API, but the 
		* the (debug) calls to print instructions affect the state
		* of the set pointers */
		
		reg = fregs->item;
		fregs = fregs->next;
		/*
		if (strcmp(reg->name,"_SubState")==0)
		fprintf(stderr,"Reg: %s\n",reg->name);
		*/
		
		/* Catch inconsistently set-up live ranges
		 * (see tracker items #1469504 + #1474602)
		 * FIXME: Actually we should rather fix the
		 * computation of the liveranges instead...
		 */
		if (!reg || !reg->reglives.usedpFlows
			|| !reg->reglives.assignedpFlows)
		{
		  //fprintf( stderr, "skipping reg w/o liveranges: %s\n", reg ? reg->name : "(unknown)");
		  continue;
		}

		if(reg->type == REG_SFR || reg->type == REG_STK || reg->isPublic || reg->isExtern|| reg->isFixed) {
			//fprintf(stderr,"skipping SFR: %s\n",reg->name);
			continue;
		}

		pcfl_used = setFirstItem(reg->reglives.usedpFlows);
		pcfl_assigned = setFirstItem(reg->reglives.assignedpFlows);
		
		used = elementsInSet(reg->reglives.usedpCodes);
		if(used == 2) { 
			/*
			In this section, all registers that are used in only in two 
			instructions are examined. If possible, they're optimized out.
			*/
			
			/*
			fprintf (stderr, "OptimizeRegUsage: %s  addr=0x%03x rIdx=0x%03x type=%d used=%d\n",
			reg->name,
			reg->address,
			reg->rIdx, reg->type, used);
			*/
			pc1 = setFirstItem(reg->reglives.usedpCodes);
			pc2 = setNextItem(reg->reglives.usedpCodes);
			
			if(pcfl_used && pcfl_assigned) {
				/* 
				expected case - the register has been assigned a value and is
				subsequently used 
				*/
				
				//fprintf(stderr," used only twice\n");
				if(pcfl_used->seq == pcfl_assigned->seq) {
					
					//fprintf(stderr, "  and used in same flow\n");
					
					pCodeOptime2pCodes(pc1, pc2, pcfl_used, reg, 1,optimize_level);
					
				} else {
					// fprintf(stderr, "  and used in different flows\n");
					
				}
				
			} else if(pcfl_used) {
				
				/* register has been used twice without ever being assigned */
				fprintf(stderr,"WARNING %s: reg %s used without being assigned\n",__FUNCTION__,reg->name);
				
			} else {
				//fprintf(stderr,"WARNING %s.1: reg %s assigned without being used\n",__FUNCTION__,reg->name);
				Remove2pcodes(pcfl_assigned, pc1, pc2, reg, 1);
				total_registers_saved++;  // debugging stats.
			}
		} else {
			
			/* register has been used either once, or more than twice */
			
			if(used && !pcfl_used && pcfl_assigned) {
				pCode *pc;
				
				//fprintf(stderr,"WARNING %s.2: reg %s assigned without being used\n",__FUNCTION__,reg->name);
				
				pc = setFirstItem(reg->reglives.usedpCodes);
				while(pc) {
					
					pcfl_assigned = PCODE(PCI(pc)->pcflow);
					Remove1pcode(pc, reg,2);
					
					deleteSetItem (&(PCFL(pcfl_assigned)->registers), reg);
					/*
					deleteSetItem (&(reg->reglives.usedpCodes),pc);
					pc->destruct(pc);
					*/
					pc = setNextItem(reg->reglives.usedpCodes);
				}
				
				
				reg->isFree = 1;
				reg->wasUsed = 0;
				
				total_registers_saved++;  // debugging stats.
			} else if( (used > 2) && optimize_multi_uses) {
				
				set *rset1=NULL;
				set *rset2=NULL;
				int searching=1;
				
				pCodeFlow *pcfl1=NULL, *pcfl2=NULL;
				
				/* examine the number of times this register is used */
				
				
				rset1 = reg->reglives.usedpCodes;
				while(rset1 && searching) {
					
					pc1 = rset1->item;
					rset2 = rset1->next;
					
					if(pc1 && isPCI(pc1) &&  ( (pcfl1 = PCI(pc1)->pcflow) != NULL) ) {
						
						if(rset2) {
							
							pc2 = rset2->item;
							if(pc2 && isPCI(pc2)  &&  ( (pcfl2 = PCI(pc2)->pcflow) != NULL) )  {
								if(pcfl2 == pcfl1) {
									
									if(pCodeOptime2pCodes(pc1, pc2, pcfl_used, reg, 0,optimize_level))
										searching = 0;
								}
							}
							
							//rset2 = rset2->next;
							
						}
					}
					rset1 = rset2;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------*
* void pCodeRegOptimeRegUsage(pBlock *pb) 
*-----------------------------------------------------------------*/
void pCodeRegOptimizeRegUsage(int level)
{
	
	int passes;
	int saved = 0;
	int t = total_registers_saved;

#if 0
	/* This is currently broken (need rewrite to correctly
	 * handle arbitrary pCodeOps instead of registers only). */
	if (!pic14_options.disable_df)
		optimizeDataflow ();
#endif

	if(!register_optimization)
		return;
#define OPT_PASSES 4
	passes = OPT_PASSES;
	
	do {
		saved = total_registers_saved;
		
		/* Identify registers used in one flow sequence */
		OptimizeRegUsage(dynAllocRegs,level, (OPT_PASSES-passes));
		OptimizeRegUsage(dynStackRegs,level, (OPT_PASSES-passes));
		OptimizeRegUsage(dynDirectRegs,0, (OPT_PASSES-passes));
		
		if(total_registers_saved != saved)
			DFPRINTF((stderr, " *** pass %d, Saved %d registers, total saved %d ***\n", 
			(1+OPT_PASSES-passes),total_registers_saved-saved,total_registers_saved));
		
		passes--;
		
	} while( passes && ((total_registers_saved != saved) || (passes==OPT_PASSES-1)) );
	
	if(total_registers_saved == t) 
		DFPRINTF((stderr, "No registers saved on this pass\n"));
}


/*-----------------------------------------------------------------*
* void RegsUnMapLiveRanges(set *regset)
*
*-----------------------------------------------------------------*/
static void RegsSetUnMapLiveRanges(set *regset)
{
	reg_info *reg;
	
	while(regset) {
		reg = regset->item;
		regset = regset->next;
		
		
		deleteSet(&reg->reglives.usedpCodes);
		deleteSet(&reg->reglives.usedpFlows);
		deleteSet(&reg->reglives.assignedpFlows);
		
	}
	
}

void  RegsUnMapLiveRanges(void)
{
	
	RegsSetUnMapLiveRanges(dynAllocRegs);
	RegsSetUnMapLiveRanges(dynStackRegs);
	RegsSetUnMapLiveRanges(dynDirectRegs);
	RegsSetUnMapLiveRanges(dynProcessorRegs);
	RegsSetUnMapLiveRanges(dynDirectBitRegs);
	RegsSetUnMapLiveRanges(dynInternalRegs);
	
}
