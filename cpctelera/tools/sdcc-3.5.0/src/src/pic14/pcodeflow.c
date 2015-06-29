/*-------------------------------------------------------------------------

   pcodeflow.c - post code generation flow analysis

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
  pcodeflow.c

  The purpose of the code in this file is to analyze the flow of the
  pCode.

*/

#include "pcodeflow.h"

#if 0
static void dbg_dumpFlow(pBlock *pb)
{
	pCode *pcflow;
	
	for( pcflow = findNextpCode(pb->pcHead, PC_FLOW); 
	pcflow != NULL;
	pcflow = findNextpCode(pcflow->next, PC_FLOW) ) {
		
		if(!isPCFL(pcflow))
			fprintf(stderr, "LinkFlow - pcflow is not a flow object ");
		
		fprintf(stderr, "Flow: 0x%x",pcflow->seq);
		if(PCFL(pcflow) && PCFL(pcflow)->ancestor)
			fprintf(stderr,", ancestor 0x%x\n",
			PCFL(pcflow)->ancestor->pc.seq);
		else {
			pCodeFlowLink *from = (PCFL(pcflow)->from) ? (PCFLINK(setFirstItem(PCFL(pcflow)->from))) : NULL;
			fprintf(stderr," no ancestor");
			while(from) {
				fprintf(stderr," (0x%x)",from->pcflow->pc.seq);
				from = setNextItem(PCFL(pcflow)->from);
			}
			fprintf(stderr,"\n");
		}
	}
	
}
#endif

#if 0
/*-----------------------------------------------------------------*
* void BuildFlowSegment(set *segment, pCodeFlow *pcflow)
*-----------------------------------------------------------------*/
static void BuildFlowSegment(pCodeFlow *pcflow)
{
	static int recursion=0;
	pCodeFlow *pcflow_other;
	set *flowset;
	
	if(!pcflow)
		return;
	
	if(recursion++ > 200) {
		fprintf(stderr, " exceeded recursion\n");
		return;
	}
	
	fprintf(stderr,"examining 0x%x\n",pcflow->pc.seq);
	
	if(pcflow->from) {
		
		
		flowset = pcflow->from;
		
		if(flowset && flowset->next == NULL) {
			
		/* 
		There is a flow before this one. In fact, there's
		exactly one flow before this one (because ->next is
		NULL). That means all children of this node pass
		through both this node and the node immediately 
		before this one; i.e. they have the same ancestor.
			*/
			
			if( (NULL == (pcflow_other = PCFLINK(flowset->item)->pcflow)) ||
				(NULL == pcflow_other)) {
				fprintf(stderr,"2 error in flow link\n");
				exit(1);
			}
			pcflow->ancestor = pcflow_other->ancestor ;
			
			fprintf(stderr,"Assigning ancestor 0x%x from flow 0x%x\n",
				pcflow->ancestor->pc.seq, pcflow_other->pc.seq);
			
		} else {
			
			if(flowset == NULL) {
				
			/* There are no flows before this one.
			* If this is not the first flow object in the pBlock then 
				* there's an error */
				
				if(!pcflow->ancestor) {
					fprintf(stderr,"error in flow link\n");
					exit(1);
					
				} 
				
			} else {
				
			/* Flow passes to this flow from multiple flows. Let's
			look at just one of these. If the one we look at has
			an ancestor, then that's our ancestor too. If the one
			we look at doesn't have an ancestor, then that means
			we haven't traversed that branch of the call tree 
				yet - but we will */
				
				pcflow_other = PCFLINK(flowset->item)->pcflow;
				if(pcflow_other) {
					fprintf(stderr, "coming from 0x%x\n",pcflow_other->pc.seq);
					if( pcflow_other->ancestor)
						pcflow->ancestor = pcflow_other->ancestor;
				}
			}
			
			
		}
		
	} else {
		/* there are no nodes before this one */
		if(!pcflow->ancestor)
			fprintf(stderr,"3 Error in flow link\n");
	}
	
	/* Now let's recursively expand the call tree */
	
	if(pcflow->ancestor && pcflow->to) {
		flowset = pcflow->to;
		while(flowset) {
			BuildFlowSegment(PCFLINK(flowset->item)->pcflow);
			flowset = flowset->next;
		}
	}
	
}
#endif

void BuildFlowTree(pBlock *pb)
{
	pCodeFlow *first_pcflow, *pcflow;
	
	
	//  fprintf(stderr,"BuildFlowTree \n");
	
	first_pcflow = PCFL(findNextpCode(pb->pcHead, PC_FLOW));
	if(!first_pcflow)
		return;
	
		/* The very first node is like Adam, it's its own ancestor (i.e. 
		* there are no other flows in this pBlock prior to the first one).
	*/
	
	first_pcflow->ancestor = first_pcflow;
	
	/* For each flow that has only one predecessor, it's easy to 
	identify the ancestor */
	pcflow = PCFL(findNextpCode(first_pcflow->pc.next, PC_FLOW));
	
	while(pcflow) {
		if(elementsInSet(pcflow->from) == 1) {
			pCodeFlowLink *from = PCFLINK(setFirstItem(pcflow->from));
			
			pcflow->ancestor = from->pcflow;
			/*
			fprintf(stderr,"Assigning ancestor 0x%x to flow 0x%x\n",
			pcflow->ancestor->pc.seq, pcflow->pc.seq);
			*/
		}
		
		pcflow = PCFL(findNextpCode(pcflow->pc.next, PC_FLOW));
		
	}
	
	pcflow = PCFL(findNextpCode(first_pcflow->pc.next, PC_FLOW));
	
	while(pcflow) {
		if(elementsInSet(pcflow->from) > 1) {
			pCodeFlow *min_pcflow;
			pCodeFlowLink *from = PCFLINK(setFirstItem(pcflow->from));
			
			min_pcflow = from->pcflow;
			
			while( (from = setNextItem(pcflow->from)) != NULL) {
				if(from->pcflow->pc.seq < min_pcflow->pc.seq)
					min_pcflow = from->pcflow;
			}
			
			pcflow->ancestor = min_pcflow;
			/*
			fprintf(stderr,"Assigning ancestor 0x%x to flow 0x%x from multiple\n",
			pcflow->ancestor->pc.seq, pcflow->pc.seq);
			*/
			
		}
		
		pcflow = PCFL(findNextpCode(pcflow->pc.next, PC_FLOW));
		
	}
	
	//  BuildFlowSegment(pcflow);
	
	//dbg_dumpFlow(pb);
	
}
