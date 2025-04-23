%{
#include <stdio.h>
//#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "cmdlexcl.h"
#include "cmdpars.hh"
#include "cmdutil.h"

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
">"			return(PTOK_GREATER);
"!"			return(PTOK_EXCLAMATION);
","			return(PTOK_COMMA);

($|0[xX])[0-9a-fA-F]+ {
  yylval.number = 0;
  int i;
  if (yytext[0]=='$')
    i=1;
  else
    i=2;
  for (; yytext[i]; i++)
  {
  char c= yytext[i];
  int val= 0;
  if (c>='0' && c<='9') val= c-'0';
  else if (c>='a' && c<='f') val= c-'a'+10;
  else if (c>='A' && c<='F') val= c-'A'+10;
  yylval.number = (yylval.number << 4) | (val&0xf);
  }
  return PTOK_NUMBER;
}

0b[01]+ {
  yylval.number = 0;
  for (int i= 2; yytext[i]; i++)
    yylval.number = (yylval.number << 1) | (yytext[i] == '0' ? 0 : 1);
  return PTOK_NUMBER;
}

0[0-7]* {
  yylval.number = 0;
  for (int i= 1; yytext[i]; i++)
  {
  char c= yytext[i];
  int val= c-'0';
  yylval.number = (yylval.number << 3) | (val&0x7);
  }
  return PTOK_NUMBER;
}

[1-9][0-9]* {
  yylval.number = 0;
  for (int i= 0; yytext[i]; i++)
  {
  char c= yytext[i];
  int val= c-'0';
  yylval.number = (yylval.number * 10) + (val&0xf);
  }
  return PTOK_NUMBER;
}

\.?[a-zA-Z_][0-9a-zA-Z_]*($[0-9]+)?	return(uc_yy_check_id(yytext));

'[^']+' {
  char *s, *p;
  int l= strlen(&yytext[1]);
  yylval.number=0;
  if (l>0)
  {
    s= strdup(&yytext[1]);
    s[strlen(s)-1]= 0;
    p= proc_escape(s, &l);
    yylval.number= p[0];
    free(s);
    free(p);
  }
  return PTOK_NUMBER;
}

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
  //YY_FLUSH_BUFFER;
  if ((YY_CURRENT_BUFFER != 0) && (YY_CURRENT_BUFFER!=NULL))
    yy_flush_buffer(YY_CURRENT_BUFFER);
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
      bool found;
      if ((found= uc->symbol2address(yytext, &mem, &addr)))
	  {
	      yylval.memory.memory= mem;
	      yylval.memory.address= addr;
	      return PTOK_MEMORY;
	  }
      if (found)
	{
	  t_addr memaddr;
	  int bitnr_low, bitnr_high;
	  yylval.bit.memory= uc->bit2mem(addr, &memaddr, &bitnr_high, &bitnr_low);
	  yylval.bit.mem_address = memaddr;
	  yylval.bit.bit_address = addr;
	  yylval.bit.bitnr_high = bitnr_high;
	  yylval.bit.bitnr_low = bitnr_low;
	  return PTOK_BITS;
	}
      {
	t_index i;
	found= uc->vars->by_name.search(token, i);
	if (found)
	  {
	    class cl_cvar *v= (class cl_cvar *)uc->vars->by_name.at(i);
	    yylval.cell= v->get_cell();
	    return PTOK_CELL;
          }
       }
    }

  return 0;
}
