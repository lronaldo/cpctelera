%{
#include "cmdlexcl.h"
#include "memcl.h"
#include "globals.h"
#include "stypes.h"

static void yyerror (const char *msg);
%}
%expect 6

%token PTOK_PLUS PTOK_MINUS PTOK_ASTERIX PTOK_SLASH PTOK_EQUAL
%token PTOK_LEFT_PAREN PTOK_RIGHT_PAREN
%token PTOK_LEFT_BRACKET PTOK_RIGHT_BRACKET
%token PTOK_DOT PTOK_AMPERSAND

%token <memory_object> PTOK_MEMORY_OBJECT
%token <memory> PTOK_MEMORY
%token <number> PTOK_NUMBER
%token <bit> PTOK_BIT

%right PTOK_EQUAL
%left PTOK_MINUS PTOK_PLUS
%left PTOK_ASTERIX PTOK_SLASH
%nonassoc UNARYMINUS PTOK_AMPERSAND
%nonassoc PTOK_LEFT_PAREN PTOK_RIGHT_PAREN

%type <number> ucsim_grammar assignment expression address_of_expression
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
	expression { application->dd_printf("%d\n", $1); }
	;

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

expression:
	  assignment { $$= $1; }
	| expression PTOK_PLUS expression { $$= $1 + $3; }
	| expression PTOK_MINUS expression { $$= $1 - $3; }
	| expression PTOK_ASTERIX expression { $$= $1 * $3; }
	| expression PTOK_SLASH expression
	  {
	    if ($3 == 0)
	      yyerror((char *)"Divide by zero");
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

address_of_expression:
	  PTOK_AMPERSAND memory { $$= $2.address; }
	| PTOK_AMPERSAND bit
	{
	  $$= $2.bit_address;
	  if ($$ < 0)
	    {
	      yyerror((char *)"Bit has no address.");
	      $$= 0;
	    }
	}
	;

memory:
	  PTOK_MEMORY
	| PTOK_MEMORY_OBJECT PTOK_LEFT_BRACKET expression PTOK_RIGHT_BRACKET
	  {
	    $$.memory= $1;
	    $$.address= $3;
	  }

bit:
	  PTOK_BIT
	| memory PTOK_DOT expression
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
  application->dd_printf ("Parser error: %s\n", msg);
}
