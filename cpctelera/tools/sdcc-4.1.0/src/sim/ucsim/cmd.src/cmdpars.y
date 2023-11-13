%{
#include <stdlib.h>
#include <unistd.h>
  
#include "cmdlexcl.h"
  //#include "memcl.h"
#include "globals.h"
  //#include "stypes.h"

static void yyerror (const char *msg);
%}
/*%expect 6*/

%token PTOK_PLUS PTOK_MINUS PTOK_ASTERIX PTOK_SLASH PTOK_EQUAL
%token PTOK_LEFT_PAREN PTOK_RIGHT_PAREN
%token PTOK_LEFT_BRACKET PTOK_RIGHT_BRACKET
%token PTOK_DOT PTOK_AMPERSAND PTOK_PIPE PTOK_CIRCUM
%token PTOK_PERCENT PTOK_TILDE PTOK_QUESTION PTOK_COLON PTOK_EXCLAMATION
%token PTOK_LESS PTOK_GREATHER PTOK_COMMA

%token PTOK_AND_OP PTOK_OR_OP
%token PTOK_INC_OP PTOK_DEC_OP
%token PTOK_EQ_OP PTOK_NE_OP
%token PTOK_GE_OP PTOK_LE_OP
%token PTOK_LEFT_OP PTOK_RIGHT_OP

%token PTOK_MUL_ASSIGN
%token PTOK_DIV_ASSIGN
%token PTOK_MOD_ASSIGN
%token PTOK_ADD_ASSIGN
%token PTOK_SUB_ASSIGN
%token PTOK_LEFT_ASSIGN
%token PTOK_RIGHT_ASSIGN
%token PTOK_AND_ASSIGN
%token PTOK_XOR_ASSIGN
%token PTOK_OR_ASSIGN

%token PTOK_INT

%token <memory_object> PTOK_MEMORY_OBJECT
%token <memory> PTOK_MEMORY
%token <number> PTOK_NUMBER
%token <bit> PTOK_BIT

%type <number> ucsim_grammar expr
%type <number> primary_expr postfix_expr
%type <number> unary_expr cast_expr
%type <number> multiplicative_expr additive_expr shift_expr
%type <number> relational_expr equality_expr
%type <number> and_expr exclusive_or_expr inclusive_or_expr
%type <number> logical_and_expr logical_or_expr
%type <number> conditional_expr assignment_expr

%type <number> type_name assignment_operator

%type <memory> memory
%type <bit> bit

%union {
  long number;
  class cl_memory *memory_object;
  struct {
    class cl_memory *memory;
    long address;
  } memory;
  struct {
    class cl_memory *memory;
    long mem_address, bit_address;
    long mask;
  } bit;
}

%%

ucsim_grammar:
expr { application->/*dd_printf("%d\n", $1)*/expr_result=$1; }
	;
/*
assignment:
	  memory PTOK_EQUAL expression
	  {
	    $1.memory->write($1.address, $3);
	    $$= $3;
	  }
	| bit PTOK_EQUAL expression
	  {
	    if ($3)
	      {
		$1.memory->write($1.mem_address,
				 $1.memory->read($1.mem_address) | $1.mask);
		$$= 1;
	      }
	    else
	      {
		$1.memory->write($1.mem_address,
				 $1.memory->read($1.mem_address) & ~($1.mask));
		$$= 0;
	      }
	  }
	;
*/
/*
expression:
	  assignment { $$= $1; }
	| expression PTOK_PLUS expression { $$= $1 + $3; }
	| expression PTOK_MINUS expression { $$= $1 - $3; }
	| expression PTOK_ASTERIX expression { $$= $1 * $3; }
	| expression PTOK_SLASH expression
	  {
	    if ($3 == 0)
	      yyerror("Divide by zero");
	    else
	      $$= $1 / $3;
	  }
	| PTOK_MINUS expression %prec UNARYMINUS { $$= -$2; }
	| address_of_expression { $$= $1; }
	| PTOK_LEFT_PAREN expression PTOK_RIGHT_PAREN { $$= $2; }
	| PTOK_NUMBER { $$= $1; }
	| memory { $$= $1.memory->read($1.address); }
	| bit { $$= ($1.memory->read($1.mem_address) & $1.mask)?1:0; }
	;
*/

/*
address_of_expression:
	  PTOK_AMPERSAND memory { $$= $2.address; }
	| PTOK_AMPERSAND bit
	{
	  $$= $2.bit_address;
	  if ($$ < 0)
	    {
	      yyerror("Bit has no address.");
	      $$= 0;
	    }
	}
	;
*/


primary_expr
/*   : identifier      */
: memory { $$= $1.memory->read($1.address); }
| bit { $$= ($1.memory->read($1.mem_address) & $1.mask)?1:0; }
| PTOK_NUMBER { $$= $1; }
     /*   | string_literal_val*/
| PTOK_LEFT_PAREN expr PTOK_RIGHT_PAREN { $$= $2; }
     /*   | generic_selection*/
   ;

postfix_expr
: primary_expr { $$= $1; }
/*   | postfix_expr PTOK_LEFT_BRACKET expr PTOK_RIGHT_BRACKET          */
/*   | postfix_expr PTOK_LEFT_PAREN PTOK_RIGHT_PAREN               */
/*   | postfix_expr PTOK_LEFT_PAREN argument_expr_list PTOK_RIGHT_PAREN*/
/*   | postfix_expr PTOK_DOT  identifier*/
/*   | postfix_expr PTOK_AMPERSAND  identifier*/
/*| postfix_expr PTOK_INC_OP*/
| memory PTOK_INC_OP
	{
	  $$= $1.memory->read($1.address);
	  $1.memory->write($1.address, $$+1);
	}
/*| postfix_expr PTOK_DEC_OP*/
| memory PTOK_DEC_OP
	{
	  $$= $1.memory->read($1.address);
	  $1.memory->write($1.address, $$-1);
	}
;
/*
argument_expr_list
   : assignment_expr
   | assignment_expr PTOK_COLON argument_expr_list 
   ;
*/

unary_expr
: postfix_expr { $$= $1; }
/*| PTOK_INC_OP unary_expr      */
| PTOK_INC_OP memory
	{
	  $$= $2.memory->read($2.address);
	  $2.memory->write($2.address, $$+1);
	  $$= $2.memory->read($2.address);
	}
/*| PTOK_DEC_OP unary_expr      */
| PTOK_DEC_OP memory
	{
	  $$= $2.memory->read($2.address);
	  $2.memory->write($2.address, $$-1);
	  $$= $2.memory->read($2.address);
	}
/*| unary_operator cast_expr	*/
/*| PTOK_AMPERSAND unary_expr*/
|PTOK_AMPERSAND memory { $$= $2.address; }
|PTOK_AMPERSAND bit
	{
	  $$= $2.bit_address;
	  if ($$ < 0)
	    {
	      yyerror("Bit has no address.");
	      $$= 0;
	    }
	}
| PTOK_MINUS unary_expr { $$= -$2; }
| PTOK_PLUS unary_expr { $$= +$2; }
| PTOK_TILDE unary_expr { $$= ~$2; }
| PTOK_EXCLAMATION unary_expr { $$= ($2)?0:1; }
  /*   | SIZEOF unary_expr        */
  /*   | SIZEOF PTOK_LEFT_PAREN type_name PTOK_RIGHT_PAREN */
  /*   | ALIGNOF PTOK_LEFT_PAREN type_name PTOK_RIGHT_PAREN*/
  /*   | TYPEOF unary_expr        */
  /*   | OFFSETOF PTOK_LEFT_PAREN type_name PTOK_COMMA offsetof_member_designator PTOK_RIGHT_PAREN */
;

/*
unary_operator
   : '&'    
   | '*'    
   | '+'    
   | '-'    
   | '~'    
   | '!'    
   ;
*/

cast_expr
: unary_expr { $$= $1; }
| PTOK_LEFT_PAREN type_name PTOK_RIGHT_PAREN /*cast_expr*/ memory
	{
	  $$= $4.memory->read($4.address);
	  if ($2 == PTOK_INT)
	    {
	      // If the highest bit for the memory width is set
	      // sign extend by setting all the bits above that.
	      long smask= 1U << ($4.memory->width - 1);
	      if ($$ & smask)
	        $$ |= ~(smask - 1);
	    }
	}
;

type_name
: PTOK_INT { $$= PTOK_INT; }
;

multiplicative_expr
: cast_expr { $$= $1; }
| multiplicative_expr PTOK_ASTERIX cast_expr { $$= $1 * $3; }
| multiplicative_expr PTOK_SLASH cast_expr { $$= $1 / $3; }
| multiplicative_expr PTOK_PERCENT cast_expr { $$= $1 % $3; }
;

additive_expr
: multiplicative_expr { $$= $1; }
| additive_expr PTOK_PLUS multiplicative_expr { $$= $1 + $3; }
| additive_expr PTOK_MINUS multiplicative_expr { $$= $1 - $3; }
;

shift_expr
: additive_expr { $$= $1; }
| shift_expr PTOK_LEFT_OP additive_expr { $$= $1 << $3; }
| shift_expr PTOK_RIGHT_OP additive_expr { $$= $1 >> $3; }
;

relational_expr
: shift_expr { $$= $1; }
| relational_expr PTOK_LESS shift_expr { $$= ($1 < $3)?1:0; }
| relational_expr PTOK_GREATHER shift_expr { $$= ($1 > $3)?1:0; }
| relational_expr PTOK_LE_OP shift_expr { $$= ($1 <= $3)?1:0; }
| relational_expr PTOK_GE_OP shift_expr { $$= ($1 >= $3)?1:0; }
;

equality_expr
: relational_expr { $$= $1; }
| equality_expr PTOK_EQ_OP relational_expr { $$= ($1==$3)?1:0; }
| equality_expr PTOK_NE_OP relational_expr { $$= ($1!=$3)?1:0; }
;

and_expr
: equality_expr { $$= $1; }
| and_expr PTOK_AMPERSAND equality_expr { $$= $1 & $3; }
;

exclusive_or_expr
: and_expr { $$= $1; }
| exclusive_or_expr PTOK_CIRCUM and_expr { $$= $1 ^ $3; }
;

inclusive_or_expr
: exclusive_or_expr { $$= $1; }
| inclusive_or_expr PTOK_PIPE exclusive_or_expr { $$= $1 | $3; }
;

logical_and_expr
: inclusive_or_expr { $$= $1; }
| logical_and_expr PTOK_AND_OP inclusive_or_expr { $$= ($1 && $3)?1:0; }
;

logical_or_expr
: logical_and_expr { $$= $1; }
| logical_or_expr PTOK_OR_OP logical_and_expr { $$= ($1 || $3)?1:0; }
;

conditional_expr
: logical_or_expr { $$= $1; }
| logical_or_expr PTOK_QUESTION expr PTOK_COLON conditional_expr { $$= ($1)?($3):($5); }
;

assignment_expr
: conditional_expr { $$= $1; }
/*| cast_expr assignment_operator assignment_expr*/
/*| cast_expr PTOK_EQUAL assignment_expr*/
| memory assignment_operator assignment_expr
	{
	  t_mem org= $1.memory->read($1.address);
	  $$= $3;
	  switch ($2)
	    {
	    case PTOK_EQUAL:
	      $1.memory->write($1.address, $3);
	      break;
	    case PTOK_MUL_ASSIGN:
	      $1.memory->write($1.address, org *= $3);
	      break;
	    case PTOK_DIV_ASSIGN:
	      $1.memory->write($1.address, org /= $3);
	      break;
	    case PTOK_MOD_ASSIGN:
	      $1.memory->write($1.address, org %= $3);
	      break;
	    case PTOK_ADD_ASSIGN:
	      $1.memory->write($1.address, org += $3);
	      break;
	    case PTOK_SUB_ASSIGN:
	      $1.memory->write($1.address, org -= $3);
	      break;
	    case PTOK_LEFT_ASSIGN:
	      $1.memory->write($1.address, org <<= $3);
	      break;
	    case PTOK_RIGHT_ASSIGN:
	      $1.memory->write($1.address, org >>= $3);
	      break;
	    case PTOK_AND_ASSIGN:
	      $1.memory->write($1.address, org &= $3);
	      break;
	    case PTOK_XOR_ASSIGN:
	      $1.memory->write($1.address, org ^= $3);
	      break;
	    case PTOK_OR_ASSIGN:
	      $1.memory->write($1.address, org |= $3);
	      break;
	    }
	  $$= $1.memory->read($1.address);
	}
| bit PTOK_EQUAL assignment_expr
	{
	  if ($3)
	    {
	      $1.memory->write($1.mem_address,
			       $1.memory->read($1.mem_address) | $1.mask);
	      $$= 1;
	    }
	  else
	    {
	      $1.memory->write($1.mem_address,
			       $1.memory->read($1.mem_address) & ~($1.mask));
	      $$= 0;
	    }
	}
;

assignment_operator
: PTOK_EQUAL { $$= PTOK_EQUAL; }
| PTOK_MUL_ASSIGN { $$= PTOK_MUL_ASSIGN; }
| PTOK_DIV_ASSIGN { $$= PTOK_DIV_ASSIGN; }
| PTOK_MOD_ASSIGN { $$= PTOK_MOD_ASSIGN; }
| PTOK_ADD_ASSIGN { $$= PTOK_ADD_ASSIGN; }
| PTOK_SUB_ASSIGN { $$= PTOK_SUB_ASSIGN; }
| PTOK_LEFT_ASSIGN { $$= PTOK_LEFT_ASSIGN; }
| PTOK_RIGHT_ASSIGN { $$= PTOK_RIGHT_ASSIGN; }
| PTOK_AND_ASSIGN { $$= PTOK_AND_ASSIGN; }
| PTOK_XOR_ASSIGN { $$= PTOK_XOR_ASSIGN; }
| PTOK_OR_ASSIGN { $$= PTOK_OR_ASSIGN; }
;

expr
: assignment_expr { $$= $1; }
| expr PTOK_COMMA assignment_expr { $$= $3; }
;

memory:
	  PTOK_MEMORY
	| PTOK_MEMORY_OBJECT PTOK_LEFT_BRACKET expr PTOK_RIGHT_BRACKET
	  {
	    $$.memory= $1;
	    $$.address= $3;
	  }

bit:
	  PTOK_BIT
	| memory PTOK_DOT expr
	  {
	    $$.memory= $1.memory;
	    $$.mem_address= $1.address;
	    $$.mask= 1 << $3;
	    $$.bit_address= -1;
	    class cl_uc *uc= application->get_uc();
	    if (uc)
	      $$.bit_address= uc->bit_address($1.memory, $1.address, $3);
	  }
	;

%%

static void
yyerror (const char *msg)
{
  application->dd_cprintf ("error", "Parser error: %s\n", msg);
}
