%{
#include <stdio.h>
//#include <stdint.h>
#include "cmdlexcl.h"
#include "cmdpars.hh"

static int uc_yy_check_id(char *token);
static int uc_yy_input(char *buf, int max_size);
#define YY_INPUT(buf,result,max_size) result = uc_yy_input(buf, max_size) 
%}

%option nounput

%%

[ \t]*			;

"int"			return(PTOK_INT);
"*="			return(PTOK_MUL_ASSIGN);
"/="			return(PTOK_DIV_ASSIGN);
"%="			return(PTOK_MOD_ASSIGN);
"+="			return(PTOK_ADD_ASSIGN);
"-="			return(PTOK_SUB_ASSIGN);
"<<="			return(PTOK_LEFT_ASSIGN);
">>="			return(PTOK_RIGHT_ASSIGN);
"&="			return(PTOK_AND_ASSIGN);
"^="			return(PTOK_XOR_ASSIGN);
"|="			return(PTOK_OR_ASSIGN);
"&&"			return(PTOK_AND_OP);
"||"			return(PTOK_OR_OP);
"++"			return(PTOK_INC_OP);
"--"			return(PTOK_DEC_OP);
"=="			return(PTOK_EQ_OP);
"!="			return(PTOK_NE_OP);
">="			return(PTOK_GE_OP);
"<="			return(PTOK_LE_OP);
">>"			return(PTOK_RIGHT_OP);
"<<"			return(PTOK_LEFT_OP);
"+"			return(PTOK_PLUS);
"-"			return(PTOK_MINUS);
"*"			return(PTOK_ASTERIX);
"/"			return(PTOK_SLASH);
"("			return(PTOK_LEFT_PAREN);
")"			return(PTOK_RIGHT_PAREN);
"["			return(PTOK_LEFT_BRACKET);
"]"			return(PTOK_RIGHT_BRACKET);
"="			return(PTOK_EQUAL);
"."			return(PTOK_DOT);
"&"			return(PTOK_AMPERSAND);
"|"			return(PTOK_PIPE);
"^"			return(PTOK_CIRCUM);
"%"			return(PTOK_PERCENT);
"~"			return(PTOK_TILDE);
"?"			return(PTOK_QUESTION);
":"			return(PTOK_COLON);
"<"			return(PTOK_LESS);
">"			return(PTOK_GREATHER);
"!"			return(PTOK_EXCLAMATION);
","			return(PTOK_COMMA);

([0-9]+)|(0x[0-9a-fA-F]+) {
  //printf("\nlexer found a nr: %s\n",yytext);
  yylval.number = strtol(yytext, 0, 0);
  return PTOK_NUMBER;
}

[a-zA-Z_][0-9a-zA-Z_]*	return(uc_yy_check_id(yytext));

.			return(yytext[0]);

%%

int
yywrap()
{
  return 1;
}

#include "globals.h"
static char *string_to_parse = NULL;

void
uc_yy_set_string_to_parse(const char *str)
{
  string_to_parse = strdup(str);
  YY_FLUSH_BUFFER;
}

static const char *string_ptr = NULL;

void
uc_yy_free_string_to_parse()
{
  free(string_to_parse);
  string_ptr=NULL;
}

static int
uc_yy_input(char *buf, int max_size)
{
  //printf("\nuc_yy_input called for max=%d\n",max_size);
  if (NULL == string_ptr)
    {
      string_ptr = string_to_parse;
      //printf("\nstring_ptr is NULL, start over with %s\n",string_to_parse);
    }
  else
  {
    //printf("\ncontinue with %s\n",string_ptr);
  }

  if (NULL != string_ptr)
    {
      int lrem = strlen(string_ptr);
      int n = max_size;
      if (lrem < max_size)
        n = lrem;
      strncpy(buf, string_ptr, n);
      string_ptr += n;
      //printf("\n%d chars copied, left=%s\n",n,string_ptr);
      return n;
    }
  else
    return 0;
}

static int
uc_yy_check_id(char *token)
{
  class cl_uc *uc= application->get_uc();
  //printf("checking id=\"%s\"\n",token);

  if (uc)
    {
      class cl_memory *mem = uc->memory(token);
      if (mem)
	{
	  yylval.memory_object = mem;
	  return PTOK_MEMORY_OBJECT;
	}
      
      t_addr addr;
      class cl_address_space *as;
      bool found;
      if ((found= uc->symbol2address(yytext, &as, &addr)))
	  {
	      yylval.memory.memory= as;
	      yylval.memory.address= addr;
	      return PTOK_MEMORY;
	  }
      /*
      else if ((found= uc->symbol2address(yytext, uc->sfr_tbl(), &addr)))
	{
	  //yylval.number= addr; return PTOK_NUMBER;
	  yylval.memory.memory = uc->address_space(MEM_SFR_ID);
	  yylval.memory.address = addr;
	  return PTOK_MEMORY;
	}
      */
      //found= uc->symbol2address(yytext, uc->bit_tbl(), &addr);
      if (found)
	{
	  t_addr memaddr;
	  t_mem mask;
	  yylval.bit.memory= uc->bit2mem(addr, &memaddr, &mask);
	  yylval.bit.mem_address = memaddr;
	  yylval.bit.bit_address = addr;
	  yylval.bit.mask = mask;
	  return PTOK_BIT;
	}
    }

  return 0;
}
