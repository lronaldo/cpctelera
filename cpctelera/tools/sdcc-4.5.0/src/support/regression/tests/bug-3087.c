/* bug-3087.c
   A bug in z80 register allocation resultingin broken code for gbz80.
 */

#include <testfwk.h>

#ifndef PORT_HOST
#pragma disable_warning 84
#endif

void Proc_2 (int *Int_Par_Ref)
{
	Int_Par_Ref;
}

void Proc_7 (int Int_1_Par_Val, int Int_2_Par_Val, int *Int_Par_Ref)
{
	Int_1_Par_Val;
	Int_2_Par_Val;
	*Int_Par_Ref = 1;
}


int f(void)
{
  int       Int_1_Loc;
  int       Int_2_Loc;
  int       Int_3_Loc;
  int       Run_Index;
  int       Number_Of_Runs;

  Number_Of_Runs = 2;

  for (Run_Index = 1; Run_Index <= Number_Of_Runs; ++Run_Index)
  {

    Int_1_Loc = 2;
    Int_2_Loc = 3;

    while (Int_1_Loc < Int_2_Loc)
    {
      Proc_7 (Int_1_Loc, Int_2_Loc, &Int_3_Loc);
      Int_1_Loc += 1;
    }

    Int_2_Loc = Int_2_Loc * Int_1_Loc;
    Int_1_Loc = Int_2_Loc / Int_3_Loc;
    Int_2_Loc = 7 * (Int_2_Loc - Int_3_Loc) - Int_1_Loc; // Operand overwritten on second subtraction.
    Proc_2 (&Int_1_Loc);
  }

  return Int_2_Loc;
}

void
testBug(void)
{
  ASSERT(f() == 47);
}

