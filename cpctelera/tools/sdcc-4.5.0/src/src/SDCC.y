/*-----------------------------------------------------------------------

  SDCC.y - parser definition file for sdcc

  Written By : Sandeep Dutta . sandeep.dutta@usa.net (1997)
  Revised by : Philipp Klaus Krause krauspeh@informatik.uni-freiburg.de (2022)
  Using inspiration from the grammar by Jutta Degener as extended by Jens Gustedt (under Expat license)

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

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/
%{
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "SDCCglobl.h"
#include "SDCCattr.h"
#include "SDCCsymt.h"
#include "SDCChasht.h"
#include "SDCCval.h"
#include "SDCCmem.h"
#include "SDCCast.h"
#include "port.h"
#include "newalloc.h"
#include "SDCCerr.h"
#include "SDCCutil.h"
#include "SDCCbtree.h"
#include "SDCCopt.h"
#include "dbuf_string.h"

extern int yyerror (char *);
extern FILE     *yyin;
long NestLevel = 0;     /* current NestLevel       */
int stackPtr  = 1;      /* stack pointer           */
int xstackPtr = 0;      /* xstack pointer          */
int reentrant = 0;
int blockNo   = 0;      /* sequential block number  */
int currBlockno=0;
int inCriticalFunction = 0;
int inCriticalBlock = 0;
int seqPointNo= 1;      /* sequence point number */
int ignoreTypedefType=0;
extern int yylex();
int yyparse(void);
extern int noLineno;
char lbuff[1024];       /* local buffer */
char function_name[256] = {0};

/* break & continue stacks */
STACK_DCL(continueStack  ,symbol *,MAX_NEST_LEVEL)
STACK_DCL(breakStack  ,symbol *,MAX_NEST_LEVEL)
STACK_DCL(forStack  ,symbol *,MAX_NEST_LEVEL)
STACK_DCL(swStk   ,ast   *,MAX_NEST_LEVEL)
STACK_DCL(blockNum,int,MAX_NEST_LEVEL*3)

value *cenum = NULL;        /* current enumeration  type chain*/
bool uselessDecl = true;

#define YYDEBUG 1

%}
%expect 3

%union {
    attribute  *attr;       /* attribute                              */
    symbol     *sym;        /* symbol table pointer                   */
    structdef  *sdef;       /* structure definition                   */
    char       yychar[SDCC_NAME_MAX+1];
    sym_link   *lnk;        /* declarator  or specifier               */
    int        yyint;       /* integer value returned                 */
    value      *val;        /* for integer constant                   */
    initList   *ilist;      /* initial list                           */
    designation*dsgn;       /* designator                             */
    const char *yystr;      /* pointer to dynamicaly allocated string */
    ast        *asts;       /* expression tree                        */
}

%token <yychar> IDENTIFIER TYPE_NAME ADDRSPACE_NAME
%token <val> CONSTANT
%token SIZEOF OFFSETOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP
%token ATTR_START TOK_SEP
%token <yyint> MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <yyint> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <yyint> XOR_ASSIGN OR_ASSIGN
%token TYPEDEF EXTERN STATIC AUTO REGISTER CONSTEXPR CODE EEPROM INTERRUPT SFR SFR16 SFR32 ADDRESSMOD
%token AT SBIT REENTRANT USING  XDATA DATA IDATA PDATA ELLIPSIS CRITICAL
%token NONBANKED BANKED SHADOWREGS SD_WPARAM
%token SD_BOOL SD_CHAR SD_SHORT SD_INT SD_LONG SIGNED UNSIGNED SD_FLOAT DOUBLE FIXED16X16 SD_CONST VOLATILE SD_VOID BIT
%token COMPLEX IMAGINARY
%token STRUCT UNION ENUM RANGE SD_FAR
%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token NAKED JAVANATIVE OVERLAY TRAP
%token <yystr> STRING_LITERAL INLINEASM FUNC
%token IFX ADDRESS_OF GET_VALUE_AT_ADDRESS SET_VALUE_AT_ADDRESS SPIL UNSPIL GETABIT GETBYTE GETWORD
%token BITWISEAND UNARYMINUS IPUSH IPUSH_VALUE_AT_ADDRESS IPOP PCALL ENDFUNCTION JUMPTABLE
%token ROT
%token CAST CALL PARAM NULLOP BLOCK LABEL RECEIVE SEND ARRAYINIT
%token DUMMY_READ_VOLATILE ENDCRITICAL INLINE RESTRICT SMALLC RAISONANCE IAR COSMIC SDCCCALL PRESERVES_REGS Z88DK_FASTCALL Z88DK_CALLEE Z88DK_SHORTCALL Z88DK_PARAMS_OFFSET

/* C11 */
%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

/* C23 problem: too many legacy FALSE and TRUE still in SDCC, workaround: prefix by TOKEN_*/
%token TOKEN_FALSE TOKEN_TRUE NULLPTR TYPEOF TYPEOF_UNQUAL SD_BITINT
%token DECIMAL32 DECIMAL64 DECIMAL128

/* SDCC extensions */
%token ASM

/* For internal use (in further stages of SDCC) */
%token GENERIC_ASSOC_LIST GENERIC_ASSOCIATION

/* If an enum type specifier is another enum (invalid but syntactically possible), make sure that the next '{' is part of the inner enum.
 * If an enum specifier is in a struct, the next ':' belongs to its enum type specifier.
 * Reason from C23: "If an enum type specifier is present, then the longest possible sequence of tokens that can be
 *                   interpreted as a specifier qualifier list is interpreted as part of the enum type specifier." */
%right ENUM '{' ':'

%type <yyint> Interrupt_storage
%type <attr> attribute_specifier_sequence attribute_specifier_sequence_opt attribute_specifier attribute_list attribute attribute_opt
%type <sym> identifier attribute_token declarator declarator2 direct_declarator array_declarator enumerator_list enumerator
%type <sym> member_declarator function_declarator
%type <sym> member_declarator_list member_declaration member_declaration_list
%type <sym> declaration init_declarator_list init_declarator
%type <sym> declaration_list identifier_list
%type <sym> kr_declaration kr_declaration_list
%type <sym> declaration_after_statement
%type <sym> declarator2_function_attributes while do for critical
%type <sym> addressmod
%type <lnk> pointer specifier_qualifier_list type_specifier_list_ type_specifier_qualifier type_specifier typeof_specifier type_qualifier_list type_qualifier_list_opt type_qualifier type_name
%type <lnk> type_specifier_list_without_struct_or_union type_specifier_qualifier_without_struct_or_union type_specifier_without_struct_or_union
%type <lnk> storage_class_specifier struct_or_union_specifier function_specifier alignment_specifier
%type <lnk> declaration_specifiers declaration_specifiers_ sfr_reg_bit sfr_attributes
%type <lnk> function_attribute function_attributes enum_specifier enum_comma_opt enum_type_specifier simple_typed_enum_decl
%type <lnk> abstract_declarator direct_abstract_declarator direct_abstract_declarator_opt array_abstract_declarator function_abstract_declarator
%type <lnk> unqualified_pointer
%type <val> parameter_type_list parameter_list parameter_declaration opt_assign_expr
%type <sdef> stag opt_stag
%type <asts> primary_expression
%type <asts> postfix_expression unary_expression offsetof_member_designator cast_expression multiplicative_expression
%type <asts> additive_expression shift_expression relational_expression equality_expression
%type <asts> and_expression exclusive_or_expression inclusive_or_expression logical_or_expr
%type <asts> logical_and_expr conditional_expr assignment_expr constant_expr constant_range_expr
%type <asts> expression argument_expr_list function_definition expression_opt predefined_constant
%type <asts> statement_list statement labeled_statement unlabeled_statement compound_statement
%type <asts> primary_block secondary_block
%type <asts> expression_statement selection_statement iteration_statement
%type <asts> jump_statement else_statement function_body string_literal_val
%type <asts> critical_statement asm_statement label
%type <asts> generic_selection generic_assoc_list generic_association generic_controlling_operand
%type <asts> implicit_block statements_and_implicit block_item_list
%type <dsgn> designator designator_list designation designation_opt
%type <ilist> initializer initializer_list
%type <yyint> unary_operator assignment_operator struct_or_union
%type <yystr> asm_string_literal

%start file

%%

   /* C23 A.2.1 Expressions */

primary_expression
   : identifier      { $$ = newAst_VALUE (symbolVal ($1)); }
   | CONSTANT        { $$ = newAst_VALUE ($1); }
   | string_literal_val
   | '(' expression ')'    { $$ = $2; }
   | generic_selection
   | predefined_constant
   ;

predefined_constant
   : TOKEN_FALSE { $$ = newAst_VALUE (constBoolVal (false, true)); }
   | TOKEN_TRUE  { $$ = newAst_VALUE (constBoolVal (true, true)); }
   | NULLPTR     { $$ = newAst_VALUE (constNullptrVal ()); }
   ;

generic_selection
   : GENERIC '(' generic_controlling_operand ',' generic_assoc_list ')' { $$ = newNode (GENERIC, $3, $5); }
   ;

generic_controlling_operand
   : assignment_expr
   | type_name { $$ = newAst_LINK ($1); }
   ;

generic_assoc_list
   : generic_association { $$ = newNode  (GENERIC_ASSOC_LIST, NULL, $1); }
   | generic_assoc_list ',' generic_association { $$ = newNode  (GENERIC_ASSOC_LIST, $1, $3); }
   ;

generic_association
   : type_name ':' assignment_expr { $$ = newNode  (GENERIC_ASSOCIATION, newAst_LINK($1), $3); }
   | DEFAULT ':' assignment_expr { $$ = newNode  (GENERIC_ASSOCIATION,NULL,$3); }
   ;

postfix_expression
   : primary_expression
   | postfix_expression '[' expression ']'          { $$ = newNode  ('[', $1, $3); }
   | postfix_expression '(' ')'               { $$ = newNode  (CALL,$1,NULL);
                                          $$->left->funcName = 1;}
   | postfix_expression '(' argument_expr_list ')'
          {
            $$ = newNode  (CALL,$1,$3); $$->left->funcName = 1;
          }
   | postfix_expression '.' { ignoreTypedefType = 1; } identifier
                      {
                        ignoreTypedefType = 0;
                        $4 = newSymbol($4->name,NestLevel);
                        $4->implicit = 1;
                        $$ = newNode(PTR_OP,newNode('&',$1,NULL),newAst_VALUE(symbolVal($4)));
                      }
   | postfix_expression PTR_OP { ignoreTypedefType = 1; } identifier
                      {
                        ignoreTypedefType = 0;
                        $4 = newSymbol($4->name,NestLevel);
                        $4->implicit = 1;
                        $$ = newNode(PTR_OP,$1,newAst_VALUE(symbolVal($4)));
                      }
   | postfix_expression INC_OP
                      { $$ = newNode(INC_OP,$1,NULL);}
   | postfix_expression DEC_OP
                      { $$ = newNode(DEC_OP,$1,NULL); }
   | '(' type_name ')' '{' initializer_list '}'
                      {
                        /* if (!options.std_c99) */
                          werror(E_COMPOUND_LITERALS_C99);

                        /* TODO: implement compound literals (C99) */
                        $$ = newAst_VALUE (valueFromLit (0));
                      }
   | '(' type_name ')' '{' initializer_list ',' '}'
     {
       // if (!options.std_c99)
         werror(E_COMPOUND_LITERALS_C99);

       // TODO: implement compound literals (C99)
       $$ = newAst_VALUE (valueFromLit (0));
     }
   | '(' type_name ')' '{' '}'
     {
       if (!options.std_c23)
         werror(W_EMPTY_INIT_C23);
       // if (!options.std_c99)
         werror(E_COMPOUND_LITERALS_C99);

       // TODO: implement compound literals (C99)
       $$ = newAst_VALUE (valueFromLit (0));
     }
   ;

argument_expr_list
   : assignment_expr
   | assignment_expr ',' argument_expr_list { $$ = newNode(PARAM,$1,$3); }
   ;

unary_expression
   : postfix_expression
   | INC_OP unary_expression        { $$ = newNode (INC_OP, NULL, $2); }
   | DEC_OP unary_expression        { $$ = newNode (DEC_OP, NULL, $2); }
   | unary_operator cast_expression
       {
         if ($1 == '&' && IS_AST_OP ($2) && $2->opval.op == '*' && $2->right == NULL)
           $$ = $2->left;
         else if ($1 == '*' && IS_AST_OP ($2) && $2->opval.op == '&' && $2->right == NULL)
           $$ = $2->left;
         else
           $$ = newNode ($1, $2, NULL);
       }
   | SIZEOF unary_expression        { $$ = newNode (SIZEOF, NULL, $2); }
   | SIZEOF '(' type_name ')'       { $$ = newAst_VALUE (sizeofOp ($3)); }
   | ALIGNOF '(' type_name ')'      { $$ = newAst_VALUE (alignofOp ($3)); }
   | OFFSETOF '(' type_name ',' offsetof_member_designator ')' { $$ = offsetofOp($3, $5); }
   | ROT '(' unary_expression ',' unary_expression ')'         { $$ = newNode (ROT, $3, $5); }
   ;

unary_operator
   : '&'    { $$ = '&';}
   | '*'    { $$ = '*';}
   | '+'    { $$ = '+';}
   | '-'    { $$ = '-';}
   | '~'    { $$ = '~';}
   | '!'    { $$ = '!';}
   ;

cast_expression
   : unary_expression
   | '(' type_name ')' cast_expression { $$ = newNode(CAST,newAst_LINK($2),$4); }
   ;

multiplicative_expression
   : cast_expression
   | multiplicative_expression '*' cast_expression { $$ = newNode('*',$1,$3);}
   | multiplicative_expression '/' cast_expression { $$ = newNode('/',$1,$3);}
   | multiplicative_expression '%' cast_expression { $$ = newNode('%',$1,$3);}
   ;

additive_expression
   : multiplicative_expression
   | additive_expression '+' multiplicative_expression { $$=newNode('+',$1,$3);}
   | additive_expression '-' multiplicative_expression { $$=newNode('-',$1,$3);}
   ;

shift_expression
   : additive_expression
   | shift_expression LEFT_OP additive_expression  { $$ = newNode(LEFT_OP,$1,$3); }
   | shift_expression RIGHT_OP additive_expression { $$ = newNode(RIGHT_OP,$1,$3); }
   ;

relational_expression
   : shift_expression
   | relational_expression '<' shift_expression   { $$ = newNode('<',  $1,$3);}
   | relational_expression '>' shift_expression   { $$ = newNode('>',  $1,$3);}
   | relational_expression LE_OP shift_expression { $$ = newNode(LE_OP,$1,$3);}
   | relational_expression GE_OP shift_expression { $$ = newNode(GE_OP,$1,$3);}
   ;

equality_expression
   : relational_expression
   | equality_expression EQ_OP relational_expression { $$ = newNode(EQ_OP,$1,$3);}
   | equality_expression NE_OP relational_expression { $$ = newNode(NE_OP,$1,$3);}
   ;

and_expression
   : equality_expression
   | and_expression '&' equality_expression  { $$ = newNode('&',$1,$3);}
   ;

exclusive_or_expression
   : and_expression
   | exclusive_or_expression '^' and_expression { $$ = newNode('^',$1,$3);}
   ;

inclusive_or_expression
   : exclusive_or_expression
   | inclusive_or_expression '|' exclusive_or_expression { $$ = newNode('|',$1,$3);}
   ;

logical_and_expr
   : inclusive_or_expression
   | logical_and_expr AND_OP { seqPointNo++;} inclusive_or_expression
                                 { $$ = newNode(AND_OP,$1,$4);}
   ;

logical_or_expr
   : logical_and_expr
   | logical_or_expr OR_OP { seqPointNo++;} logical_and_expr
                                 { $$ = newNode(OR_OP,$1,$4); }
   ;

conditional_expr
   : logical_or_expr
   | logical_or_expr '?' { seqPointNo++;} expression ':' conditional_expr
                     {
                        $$ = newNode(':',$4,$6);
                        $$ = newNode('?',$1,$$);
                     }
   ;

assignment_expr
   : conditional_expr
   | unary_expression assignment_operator assignment_expr
                     {

                             switch ($2) {
                             case '=':
                                     $$ = newNode($2,$1,$3);
                                     break;
                             case MUL_ASSIGN:
                                     $$ = createRMW($1, '*', $3);
                                     break;
                             case DIV_ASSIGN:
                                     $$ = createRMW($1, '/', $3);
                                     break;
                             case MOD_ASSIGN:
                                     $$ = createRMW($1, '%', $3);
                                     break;
                             case ADD_ASSIGN:
                                     $$ = createRMW($1, '+', $3);
                                     break;
                             case SUB_ASSIGN:
                                     $$ = createRMW($1, '-', $3);
                                     break;
                             case LEFT_ASSIGN:
                                     $$ = createRMW($1, LEFT_OP, $3);
                                     break;
                             case RIGHT_ASSIGN:
                                     $$ = createRMW($1, RIGHT_OP, $3);
                                     break;
                             case AND_ASSIGN:
                                     $$ = createRMW($1, '&', $3);
                                     break;
                             case XOR_ASSIGN:
                                     $$ = createRMW($1, '^', $3);
                                     break;
                             case OR_ASSIGN:
                                     $$ = createRMW($1, '|', $3);
                                     break;
                             default :
                                     $$ = NULL;
                             }

                     }
;

assignment_operator
   : '='             { $$ = '=';}
   | MUL_ASSIGN
   | DIV_ASSIGN
   | MOD_ASSIGN
   | ADD_ASSIGN
   | SUB_ASSIGN
   | LEFT_ASSIGN
   | RIGHT_ASSIGN
   | AND_ASSIGN
   | XOR_ASSIGN
   | OR_ASSIGN
   ;

expression
   : assignment_expr
   | expression ',' { seqPointNo++;} assignment_expr { $$ = newNode(',',$1,$4);}
   ;

expression_opt
   :                 { $$ = NULL; seqPointNo++; }
   | expression      { $$ = $1; seqPointNo++; }
   ;

constant_expr
   : conditional_expr
   ;

constant_range_expr
   : constant_expr ELLIPSIS constant_expr { $$ = newNode(ELLIPSIS,$1,$3); }
   ;

   /* C23 A.2.2 Declarations */

declaration
   : simple_typed_enum_decl
      {
         uselessDecl = true;
         $$ = NULL;
      }
   | declaration_specifiers ';'
      {
         /* Special case: if incomplete struct/union declared without name, */
         /* make sure an incomplete type for it exists in the current scope */
         if (IS_STRUCT($1))
           {
             structdef *sdef = SPEC_STRUCT($1);
             structdef *osdef;
             osdef = findSymWithBlock (StructTab, sdef->tagsym, currBlockno, NestLevel);
             if (osdef && osdef->block != currBlockno)
               {
                 sdef = newStruct(osdef->tagsym->name);
                 sdef->level = NestLevel;
                 sdef->block = currBlockno;
                 sdef->tagsym = newSymbol (osdef->tagsym->name, NestLevel);
                 addSym (StructTab, sdef, sdef->tag, sdef->level, currBlockno, 0);
                 uselessDecl = false;
               }
           }
         if (uselessDecl)
           werror(W_USELESS_DECL);
         uselessDecl = true;
         $$ = NULL;
      }
   | declaration_specifiers init_declarator_list ';'
      {
         /* add the specifier list to the id */
         symbol *sym , *sym1;

         bool autocandidate = options.std_c23 && IS_SPEC ($1) && SPEC_SCLS($1) == S_AUTO;

         for (sym1 = sym = reverseSyms($2);sym != NULL;sym = sym->next) {
             sym_link *lnk = copyLinkChain($1);
             sym_link *l0 = NULL, *l1 = NULL, *l2 = NULL;
             /* check illegal declaration */
             for (l0 = sym->type; l0 != NULL; l0 = l0->next)
               if (IS_PTR (l0))
                 break;
             /* check if creating instances of structs with flexible arrays */
             for (l1 = lnk; l1 != NULL; l1 = l1->next)
               if (IS_STRUCT (l1) && SPEC_STRUCT (l1)->b_flexArrayMember)
                 break;
             if (!options.std_c99 && l0 == NULL && l1 != NULL && SPEC_EXTR($1) != 1)
               werror (W_FLEXARRAY_INSTRUCT, sym->name);
             /* check if creating instances of function type */
             for (l1 = lnk; l1 != NULL; l1 = l1->next)
               if (IS_FUNC (l1))
                 break;
             for (l2 = lnk; l2 != NULL; l2 = l2->next)
               if (IS_PTR (l2))
                 break;
             if (l0 == NULL && l2 == NULL && l1 != NULL)
               werrorfl(sym->fileDef, sym->lineDef, E_TYPE_IS_FUNCTION, sym->name);
             if (autocandidate && !sym->type && sym->ival && sym->ival->type == INIT_NODE) // C23 auto type inference
               {
                 sym->type = sym->etype = typeofOp (sym->ival->init.node);
                 SPEC_SCLS (lnk) = 0;
               }
             /* do the pointer stuff */
             pointerTypes(sym->type,lnk);
             addDecl (sym,0,lnk);
         }

         uselessDecl = true;
         $$ = sym1;
      }
   | static_assert_declaration
      {
         $$ = NULL;
      }
   | attribute_declaration
      {
         $$ = NULL;
      }
   ;

declaration_specifiers : declaration_specifiers_ { $$ = finalizeSpec($1); };

declaration_specifiers_
   : storage_class_specifier                         { $$ = $1; }
   | storage_class_specifier declaration_specifiers_ {
     /* if the decl $2 is not a specifier */
     /* find the spec and replace it      */
     $$ = mergeDeclSpec($1, $2, "storage_class_specifier declaration_specifiers - skipped");
   }
   | type_specifier_qualifier                       { $$ = $1; }
   | type_specifier_qualifier declaration_specifiers_ {
     /* if the decl $2 is not a specifier */
     /* find the spec and replace it      */
     $$ = mergeDeclSpec($1, $2, "type_specifier_qualifier declaration_specifiers - skipped");
   }
   | function_specifier                             { $$ = $1; }
   | function_specifier declaration_specifiers_     {
     /* if the decl $2 is not a specifier */
     /* find the spec and replace it      */
     $$ = mergeDeclSpec($1, $2, "function_specifier declaration_specifiers - skipped");
   }
   ;

init_declarator_list
   : init_declarator
   | init_declarator_list ',' init_declarator      { $3->next = $1; $$ = $3;}
   ;

init_declarator
   : declarator                  { $1->ival = NULL; }
   | declarator '=' initializer  { $1->ival = $3; seqPointNo++; }
   ;

attribute_declaration
   : attribute_specifier_sequence ';'
   ;

storage_class_specifier
   : TYPEDEF   {
                  $$ = newLink (SPECIFIER);
                  SPEC_TYPEDEF($$) = 1;
               }
   | EXTERN    {
                  $$ = newLink(SPECIFIER);
                  SPEC_EXTR($$) = 1;
               }
   | STATIC    {
                  $$ = newLink (SPECIFIER);
                  SPEC_STAT($$) = 1;
               }
   | THREAD_LOCAL
               {
                  $$ = 0;
                  werror(E_THREAD_LOCAL);
               }
   | AUTO      {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_AUTO;
               }
   | REGISTER  {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_REGISTER;
               }
   | CONSTEXPR {
                  $$ = newLink (SPECIFIER);
                  werror (E_CONSTEXPR);
               }
   ;

/* NOTE:
 * Structs and unions have been factored out to avoid parsing conflicts with
 * enum-type-specifier, which semantically cannot be a struct or union, anyway.
 */

type_specifier
   : type_specifier_without_struct_or_union { $$ = $1; }
   | struct_or_union_specifier  {
                                   uselessDecl = false;
                                   $$ = $1;
                                   ignoreTypedefType = 1;
                                }
   ;

type_specifier_without_struct_or_union
   : SD_VOID   {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_VOID;
                  ignoreTypedefType = 1;
               }
   | SD_CHAR   {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_CHAR;
                  ignoreTypedefType = 1;
               }
   | SD_SHORT  {
                  $$=newLink(SPECIFIER);
                  SPEC_SHORT($$) = 1;
                  ignoreTypedefType = 1;
               }
   | SD_INT    {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_INT;
                  ignoreTypedefType = 1;
               }
   | SD_LONG   {
                  $$=newLink(SPECIFIER);
                  SPEC_LONG($$) = 1;
                  ignoreTypedefType = 1;
               }
   | SD_FLOAT  {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_FLOAT;
                  ignoreTypedefType = 1;
               }
   | DOUBLE    {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_DOUBLE;
                  ignoreTypedefType = 1;
               }
   | SIGNED    {
                  $$=newLink(SPECIFIER);
                  $$->select.s.b_signed = 1;
                  ignoreTypedefType = 1;
               }
   | UNSIGNED  {
                  $$=newLink(SPECIFIER);
                  SPEC_USIGN($$) = 1;
                  ignoreTypedefType = 1;
               }
   | SD_BITINT '(' constant_expr ')'  {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_BITINT;
                  SPEC_BITINTWIDTH($$) = (unsigned int) ulFromVal(constExprValue($3, true));
                  ignoreTypedefType = 1;
               }
   | SD_BOOL   {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_BOOL;
                  ignoreTypedefType = 1;
               }
   | COMPLEX   {
                  $$=newLink(SPECIFIER);
                  werror (E_COMPLEX_UNSUPPORTED);
               }
   | IMAGINARY {
                  $$=newLink(SPECIFIER);
                  werror (E_COMPLEX_UNSUPPORTED);
               }
   | DECIMAL32 {
                  $$=newLink(SPECIFIER);
                  werror (E_DECIMAL_FLOAT_UNSUPPORTED);
               }
   | DECIMAL64 {
                  $$=newLink(SPECIFIER);
                  werror (E_DECIMAL_FLOAT_UNSUPPORTED);
               }
   | DECIMAL128 {
                  $$=newLink(SPECIFIER);
                  werror (E_DECIMAL_FLOAT_UNSUPPORTED);
               }
   | enum_specifier     {
                           cenum = NULL;
                           uselessDecl = false;
                           ignoreTypedefType = 1;
                           $$ = $1;
                        }       
   | TYPE_NAME
         {
            symbol *sym;
            sym_link *p;
            sym = findSym(TypedefTab,NULL,$1);
            $$ = p = copyLinkChain(sym ? sym->type : NULL);
            SPEC_TYPEDEF(getSpec(p)) = 0;
            ignoreTypedefType = 1;
         }
   | typeof_specifier
     {
       $$ = $1;
     }          
   | FIXED16X16 {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_FIXED16X16;
                  ignoreTypedefType = 1;
               }
   | BIT       {
                  $$=newLink(SPECIFIER);
                  SPEC_NOUN($$) = V_BIT;
                  SPEC_SCLS($$) = S_BIT;
                  SPEC_BLEN($$) = 1;
                  SPEC_BSTR($$) = 0;
                  ignoreTypedefType = 1;
               }
   | AT CONSTANT {
                  $$=newLink(SPECIFIER);
                  /* add this to the storage class specifier  */
                  SPEC_ABSA($$) = 1;   /* set the absolute addr flag */
                  /* now get the abs addr from value */
                  SPEC_ADDR($$) = (unsigned int) ulFromVal(constExprValue(newAst_VALUE ($2), true));
               }
   | AT '(' constant_expr ')' {
                  $$=newLink(SPECIFIER);
                  /* add this to the storage class specifier  */
                  SPEC_ABSA($$) = 1;   /* set the absolute addr flag */
                  /* now get the abs addr from value */
                  SPEC_ADDR($$) = (unsigned int) ulFromVal(constExprValue($3, true));
               }


   | sfr_reg_bit;

typeof_specifier
   : TYPEOF '(' expression ')'
     {
       $$ = typeofOp ($3);
       wassert ($$);
     }
   | TYPEOF '(' type_name ')'
     {
       checkTypeSanity ($3, "(typeof)");
       $$ = $3;
     }
   | TYPEOF_UNQUAL '(' expression ')'
     {
       $$ = typeofOp ($3);
       wassert ($$);
       wassert (IS_SPEC ($$));
       SPEC_CONST ($$) = 0;
       SPEC_RESTRICT ($$) = 0;
       SPEC_VOLATILE ($$) = 0;
       SPEC_ATOMIC ($$) = 0;
       SPEC_ADDRSPACE ($$) = 0;
     }
   | TYPEOF_UNQUAL '(' type_name ')'
     {
       checkTypeSanity ($3, "(typeof_unqual)");
       $$ = $3;
       wassert (IS_SPEC ($$));
       SPEC_CONST ($$) = 0;
       SPEC_RESTRICT ($$) = 0;
       SPEC_VOLATILE ($$) = 0;
       SPEC_ATOMIC ($$) = 0;
       SPEC_ADDRSPACE ($$) = 0;
     }

struct_or_union_specifier
   : struct_or_union attribute_specifier_sequence_opt opt_stag
        {
          structdef *sdef;

          if (!$3->tagsym)
            {
              /* no tag given, so new struct def for current scope */
              addSym (StructTab, $3, $3->tag, $3->level, currBlockno, 0);
            }
          else
            {
              sdef = findSymWithBlock (StructTab, $3->tagsym, currBlockno, NestLevel);
              if (sdef)
                {
                  /* Error if a complete type already defined in this scope */
                  if (sdef->block == currBlockno)
                    {
                      if (sdef->fields)
                        $3->redefinition = true;
                      else // We are completing an incomplete type
                        $3 = sdef;
                    }
                  else
                    {
                      /* There is an existing struct def in an outer scope. */
                      /* Create new struct def for current scope */
                      addSym (StructTab, $3, $3->tag, $3->level, currBlockno, 0);
                    }
                }
              else
               {
                 /* There is no existing struct def at all. */
                 /* Create new struct def for current scope */
                 addSym (StructTab, $3, $3->tag, $3->level, currBlockno, 0);
               }
            }

          if (!$3->type)
            {
              $3->type = $1;
            }
          else
            {
              if ($3->type != $1)
                  werror(E_BAD_TAG, $3->tag, $1==STRUCT ? "struct" : "union");
            }
        }
   '{' member_declaration_list '}'
        {
          structdef *sdef;
          symbol *sym, *dsym;

          // check for errors in structure members
          for (sym=$6; sym; sym=sym->next)
            {
              if (IS_ABSOLUTE(sym->etype))
                {
                  werrorfl(sym->fileDef, sym->lineDef, E_NOT_ALLOWED, "'at'");
                  SPEC_ABSA(sym->etype) = 0;
                }
              if (IS_SPEC(sym->etype) && SPEC_SCLS(sym->etype))
                {
                  werrorfl(sym->fileDef, sym->lineDef, E_NOT_ALLOWED, "storage class");
                  printTypeChainRaw (sym->type, NULL);
                  SPEC_SCLS(sym->etype) = 0;
                }
              for (dsym=sym->next; dsym; dsym=dsym->next)
                {
                  if (*dsym->name && strcmp(sym->name, dsym->name)==0)
                    {
                      werrorfl(sym->fileDef, sym->lineDef, E_DUPLICATE_MEMBER,
                               $1==STRUCT ? "struct" : "union", sym->name);
                      werrorfl(dsym->fileDef, dsym->lineDef, E_PREVIOUS_DEF);
                    }
                }
            }

          /* Create a structdef   */
          $3->fields = reverseSyms($6);        /* link the fields */
          $3->size = compStructSize($1, $3);   /* update size of  */
          promoteAnonStructs ($1, $3);

          if ($3->redefinition) // Since C23, multiple definitions for struct / union are allowed, if they are compatible and have the same tags. The current standard draft N3047 allows redeclarations of unions to have a different order of the members. We don't. The rule in N3047 is now considered a mistake by many, and will hopefully be changed to the SDCC behaviour via a national body comment for the final version of the standard.
            {
              sdef = findSymWithBlock (StructTab, $3->tagsym, currBlockno, NestLevel);
              bool compatible = options.std_c23 && sdef->tagsym && $3->tagsym && !strcmp (sdef->tagsym->name, $3->tagsym->name);
              for (symbol *fieldsym1 = sdef->fields, *fieldsym2 = $3->fields; compatible; fieldsym1 = fieldsym1->next, fieldsym2 = fieldsym2->next)
                {
                  if (!fieldsym1 && !fieldsym2)
                    break;
                  if (!fieldsym1 || !fieldsym2)
                    compatible = false;
                  else if (strcmp (fieldsym1->name, fieldsym2->name))
                    compatible = false;
                  else if (compareType (fieldsym1->type, fieldsym2->type, true) <= 0)
                    compatible = false;
               }
              if (!compatible)
                {
                  werror(E_STRUCT_REDEF_INCOMPATIBLE, $3->tag);
                  werrorfl(sdef->tagsym->fileDef, sdef->tagsym->lineDef, E_PREVIOUS_DEF);
                }
            }
          else
            sdef = $3;

          /* Create the specifier */
          $$ = newLink (SPECIFIER);
          SPEC_NOUN($$) = V_STRUCT;
          SPEC_STRUCT($$)= sdef;
        }
   | struct_or_union attribute_specifier_sequence_opt stag
        {
          structdef *sdef;

          sdef = findSymWithBlock (StructTab, $3->tagsym, currBlockno, NestLevel);

          if (sdef)
            $3 = sdef;
          else
            {
              /* new struct def for current scope */
              addSym (StructTab, $3, $3->tag, $3->level, currBlockno, 0);
            }
          $$ = newLink(SPECIFIER);
          SPEC_NOUN($$) = V_STRUCT;
          SPEC_STRUCT($$) = $3;

          if (!$3->type)
            {
              $3->type = $1;
            }
          else
            {
              if ($3->type != $1)
                  werror(E_BAD_TAG, $3->tag, $1==STRUCT ? "struct" : "union");
            }
        }
   ;

struct_or_union
   : STRUCT          { $$ = STRUCT; ignoreTypedefType = 1; }
   | UNION           { $$ = UNION; ignoreTypedefType = 1; }
   ;

member_declaration_list
   : member_declaration
   | member_declaration_list member_declaration
        {
          symbol *sym = $2;

          /* go to the end of the chain */
          while (sym->next) sym = sym->next;
          sym->next = $1;

           $$ = $2;
        }
   ;

member_declaration
   : attribute_specifier_sequence_opt specifier_qualifier_list member_declarator_list ';'
        {
          /* add this type to all the symbols */
          symbol *sym;
          for ( sym = $3; sym != NULL; sym = sym->next )
            {
              sym_link *btype = copyLinkChain($2);

              pointerTypes(sym->type, btype);
              if (!sym->type)
                {
                  sym->type = btype;
                  sym->etype = getSpec(sym->type);
                }
              else
                  addDecl (sym, 0, btype);
              /* make sure the type is complete and sane */
              checkTypeSanity(sym->etype, sym->name);
            }
          ignoreTypedefType = 0;
          $$ = $3;
        }
   ;

type_specifier_qualifier
   : type_specifier      { $$ = $1; }
   | type_qualifier      { $$ = $1; }
   | alignment_specifier { $$ = $1; }
   ;

type_specifier_qualifier_without_struct_or_union
   : type_specifier_without_struct_or_union        { $$ = $1; }
   | struct_or_union     { fatal (1, E_ENUM_UNDERLYING_TYPE); }
   | type_qualifier      { $$ = $1; }
   | alignment_specifier { $$ = $1; }
   ;

member_declarator_list
   : member_declarator
   | member_declarator_list ',' member_declarator
        {
          $3->next  = $1;
          $$ = $3;
        }
   ;

member_declarator
   : declarator
   | ':' constant_expr
        {
          unsigned int bitsize;
          $$ = newSymbol (genSymName(NestLevel), NestLevel);
          bitsize = (unsigned int) ulFromVal(constExprValue($2, true));
          if (!bitsize)
              bitsize = BITVAR_PAD;
          $$->bitVar = bitsize;
          $$->bitUnnamed = 1;
        }
   | declarator ':' constant_expr
        {
          unsigned int bitsize;
          bitsize = (unsigned int) ulFromVal(constExprValue($3, true));

          if (!bitsize)
            {
              $$ = newSymbol (genSymName(NestLevel), NestLevel);
              $$->bitVar = BITVAR_PAD;
              werror(W_BITFLD_NAMED);
            }
          else
              $1->bitVar = bitsize;
        }
   | { $$ = newSymbol ("", NestLevel); }
   ;

enum_specifier
    : ENUM '{' enumerator_list enum_comma_opt '}'
        {
          $$ = newEnumType ($3, NULL);
          SPEC_SCLS(getSpec($$)) = 0;
        }
     | ENUM enum_type_specifier '{' enumerator_list enum_comma_opt '}'
        {
          $$ = newEnumType ($4, $2);
          SPEC_SCLS(getSpec($$)) = 0;
        }
     | ENUM identifier '{' enumerator_list enum_comma_opt '}'
        {
          symbol *csym;
          sym_link *enumtype;

          enumtype = newEnumType ($4, NULL);
          SPEC_SCLS(getSpec(enumtype)) = 0;
          $2->type = enumtype;

          csym = findSymWithLevel(enumTab, $2);
          if ((csym && csym->level == $2->level))
            {
              if (!options.std_c23 || compareType (csym->type, $2->type, true) <= 0)
                {
                  werrorfl($2->fileDef, $2->lineDef, E_DUPLICATE_TYPEDEF, csym->name);
                  werrorfl(csym->fileDef, csym->lineDef, E_PREVIOUS_DEF);
                }
            }

          /* add this to the enumerator table */
          if (!csym)
              addSym (enumTab, $2, $2->name, $2->level, $2->block, 0);
          $$ = copyLinkChain(enumtype);
        }
     | ENUM identifier enum_type_specifier '{' enumerator_list enum_comma_opt '}'
        {
          symbol *csym;
          sym_link *enumtype;

          enumtype = newEnumType ($5, $3);
          SPEC_SCLS(getSpec(enumtype)) = 0;
          $2->type = enumtype;

          csym = findSymWithLevel(enumTab, $2);
          if ((csym && csym->level == $2->level))
            {
              if (!options.std_c23 || compareType (csym->type, $2->type, true) <= 0)
                {
                  werrorfl($2->fileDef, $2->lineDef, E_DUPLICATE_TYPEDEF, csym->name);
                  werrorfl(csym->fileDef, csym->lineDef, E_PREVIOUS_DEF);
                }
            }

          /* add this to the enumerator table */
          if (!csym)
              addSym (enumTab, $2, $2->name, $2->level, $2->block, 0);
          $$ = copyLinkChain(enumtype);
        }
   | ENUM identifier
        {
          symbol *csym;

          /* check the enumerator table */
          if ((csym = findSymWithLevel(enumTab, $2)))
              $$ = copyLinkChain(csym->type);
          else
            {
              $$ = newLink(SPECIFIER);
              SPEC_NOUN($$) = V_INT;
            }
        }
   ;

/* C23:
 * An enum specifier of the form "enum identifier enum-type-specifier"
 * may not appear except in a declaration of the form "enum identifier enum-type-specifier ;"
 */
simple_typed_enum_decl
   : ENUM identifier enum_type_specifier ';'
        {
          symbol *csym;

          /* let newEnumType check the enum-type-specifier and discard the returned type */
          newEnumType (NULL, $3);

          /* check the enumerator table */
          if ((csym = findSymWithLevel(enumTab, $2)))
              $$ = copyLinkChain(csym->type);
          else
            {
              $$ = newLink(SPECIFIER);
              SPEC_NOUN($$) = V_INT;
            }
        }
   ;

enum_comma_opt
   : 
     {
       $$ = NULL;
     }
   | ','
     {
       if (!options.std_c99)
         werror (E_ENUM_COMMA_C99);
       $$ = NULL;
     }

enumerator_list
   : enumerator
   | enumerator_list ',' enumerator
        {
          $3->next = $1;
          $$ = $3;
        }
   ;

enumerator
   : identifier attribute_specifier_sequence_opt opt_assign_expr
        {
          symbol *sym;

          $1->type = copyLinkChain ($3->type);
          $1->etype = getSpec ($1->type);
          SPEC_ENUM ($1->etype) = 1;
          $$ = $1;

          // check if the symbol at the same level already exists
          if ((sym = findSymWithLevel (SymbolTab, $1)) && sym->level == $1->level)
            {
              // C23 allows redefinitions of enumeration constants with the same value as part of a redeclaration of the same enumerated type.
              if (!options.std_c23 || ullFromVal (valFromType (sym->type)) != ullFromVal (valFromType ($1->type)))
                {
                  werrorfl ($1->fileDef, $1->lineDef, E_DUPLICATE_MEMBER, "enum", $1->name);
                  werrorfl (sym->fileDef, sym->lineDef, E_PREVIOUS_DEF);
                }
            }
          else
            {
              // do this now, so we can use it for the next enums in the list
              addSymChain (&$1);
            }
        }
   ;

enum_type_specifier
   : ':' type_specifier_list_without_struct_or_union
        {
          if (!options.std_c23)
            werror (E_ENUM_TYPE_SPECIFIER_C23);
          $$ = finalizeSpec ($2);
        }
   ;

type_qualifier
   : SD_CONST  {
                  $$=newLink(SPECIFIER);
                  SPEC_CONST($$) = 1;
               }
   | RESTRICT  {
                  $$=newLink(SPECIFIER);
                  SPEC_RESTRICT($$) = 1;
               }
   | VOLATILE  {
                  $$=newLink(SPECIFIER);
                  SPEC_VOLATILE($$) = 1;
               }
   | ATOMIC  {
                  $$=newLink(SPECIFIER);
                  werror (E_ATOMIC_UNSUPPORTED);
               }
   | ADDRSPACE_NAME {
                  $$=newLink(SPECIFIER);
                  SPEC_ADDRSPACE($$) = findSym (AddrspaceTab, 0, $1);
               }
   | XDATA     {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_XDATA;
               }
   | CODE      {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_CODE;
               }
   | EEPROM    {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_EEPROM;
               }
   | DATA      {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_DATA;
               }
   | IDATA     {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_IDATA;
               }
   | PDATA     {
                  $$ = newLink (SPECIFIER);
                  SPEC_SCLS($$) = S_PDATA;
               }
   ;

function_specifier
   : INLINE    {
                  $$ = newLink (SPECIFIER);
                  SPEC_INLINE($$) = 1;
               }
   | NORETURN  {
                  $$ = newLink (SPECIFIER);
                  SPEC_NORETURN($$) = 1;
               }
   ;

alignment_specifier
   : ALIGNAS '(' type_name ')'
              {
                 checkTypeSanity ($3, "(_Alignas)");
                 $$ = newLink (SPECIFIER);
                 SPEC_ALIGNAS($$) = 1;
              }
   | ALIGNAS '(' constant_expr ')'
              {
                 value *val = constExprValue ($3, true);
                 $$ = newLink (SPECIFIER);
                 SPEC_ALIGNAS($$) = 0;
                 if (!val)
                   werror (E_CONST_EXPECTED);
                 else if (ulFromVal (val) == 0 || isPowerOf2 (ulFromVal (val)) && ulFromVal (val) <= port->mem.maxextalign)
                   SPEC_ALIGNAS($$) = ulFromVal(val);
                 else
                   werror (E_ALIGNAS, ulFromVal(val));
              }
   ;

declarator
   : direct_declarator                        { $$ = $1; }
   | pointer direct_declarator
         {
             addDecl ($2,0,reverseLink($1));
             $$ = $2;
         }
   ;

direct_declarator
   : identifier attribute_specifier_sequence_opt
   | '(' declarator ')'     { $$ = $2; }
   | array_declarator attribute_specifier_sequence_opt
   | declarator2_function_attributes attribute_specifier_sequence_opt
   ;

declarator2
   : identifier
   | '(' declarator ')'     { $$ = $2; }
   | array_declarator
   ;

array_declarator
   : direct_declarator '[' type_qualifier_list_opt ']'
     {
       sym_link *p, *n;

       p = newLink (DECLARATOR);
       DCL_TYPE(p) = ARRAY;
       DCL_ELEM(p) = 0;

       if ($3)
         {
           if (!options.std_c99)
             werror (E_QUALIFIED_ARRAY_PARAM_C99);

           DCL_PTR_CONST(p) = SPEC_CONST ($3);
           DCL_PTR_RESTRICT(p) = SPEC_RESTRICT ($3);
           DCL_PTR_VOLATILE(p) = SPEC_VOLATILE ($3);
           DCL_PTR_ADDRSPACE(p) = SPEC_ADDRSPACE ($3);
           addDecl($1,0,p);
           n = newLink (SPECIFIER);
           SPEC_NEEDSPAR(n) = 1;
           addDecl($1,0,n);
         }
       else
         addDecl($1,0,p);
     }
   | direct_declarator '[' type_qualifier_list_opt assignment_expr ']'
     {
       sym_link *p, *n;

       p = newLink (DECLARATOR);
       DCL_TYPE(p) = ARRAY;
       DCL_ELEM_AST (p) = $4;

       if ($3)
         {
           if (!options.std_c99)
             werror (E_QUALIFIED_ARRAY_PARAM_C99);
           DCL_PTR_CONST(p) = SPEC_CONST ($3);
           DCL_PTR_RESTRICT(p) = SPEC_RESTRICT ($3);
           DCL_PTR_VOLATILE(p) = SPEC_VOLATILE ($3);
           DCL_PTR_ADDRSPACE(p) = SPEC_ADDRSPACE ($3);
           addDecl($1, 0, p);
           n = newLink (SPECIFIER);
           SPEC_NEEDSPAR(n) = 1;
           addDecl($1,0,n);
         }
       else
         addDecl($1, 0, p);
     }
   | direct_declarator '[' STATIC type_qualifier_list_opt assignment_expr ']'
     {
       sym_link *p, *n;

       if (!options.std_c99)
         werror (E_STATIC_ARRAY_PARAM_C99);

       p = newLink (DECLARATOR);
       DCL_TYPE(p) = ARRAY;
       DCL_ELEM_AST (p) = $5;

       if ($4)
         {
           if (!options.std_c99)
             werror (E_QUALIFIED_ARRAY_PARAM_C99);
           DCL_PTR_CONST(p) = SPEC_CONST ($4);
           DCL_PTR_RESTRICT(p) = SPEC_RESTRICT ($4);
           DCL_PTR_VOLATILE(p) = SPEC_VOLATILE ($4);
           DCL_PTR_ADDRSPACE(p) = SPEC_ADDRSPACE ($4);
         }
       addDecl($1, 0, p);
       n = newLink (SPECIFIER);
       SPEC_NEEDSPAR(n) = 1;
       addDecl($1,0,n);
     }
   | direct_declarator '[' type_qualifier_list STATIC assignment_expr ']'
     {
       sym_link *p, *n;

       if (!options.std_c99)
         {
           werror (E_QUALIFIED_ARRAY_PARAM_C99);
           werror (E_STATIC_ARRAY_PARAM_C99);
         }

       p = newLink (DECLARATOR);
       DCL_TYPE(p) = ARRAY;
       DCL_ELEM_AST (p) = $5;

       DCL_PTR_CONST(p) = SPEC_CONST ($3);
       DCL_PTR_RESTRICT(p) = SPEC_RESTRICT ($3);
       DCL_PTR_VOLATILE(p) = SPEC_VOLATILE ($3);
       DCL_PTR_ADDRSPACE(p) = SPEC_ADDRSPACE ($3);
       addDecl($1, 0, p);
       n = newLink (SPECIFIER);
       SPEC_NEEDSPAR(n) = 1;
       addDecl($1,0,n);
     }
   ;

declarator2_function_attributes
   : function_declarator                 { $$ = $1; }
   | function_declarator function_attributes  {
           // copy the functionAttributes (not the args and hasVargs !!)
           sym_link *funcType=$1->type;

           while (funcType && !IS_FUNC(funcType))
             funcType = funcType->next;

           if (!funcType)
             werror (E_FUNC_ATTR);
           else
             {
               struct value *args = FUNC_ARGS(funcType);
               unsigned hasVargs = FUNC_HASVARARGS(funcType);
               bool noprototype = FUNC_NOPROTOTYPE(funcType);

               memcpy (&funcType->funcAttrs, &$2->funcAttrs,
                   sizeof($2->funcAttrs));

               FUNC_ARGS(funcType)=args;
               FUNC_HASVARARGS(funcType)=hasVargs;
               FUNC_NOPROTOTYPE(funcType)=noprototype;

               // just to be sure
               memset (&$2->funcAttrs, 0,
                   sizeof($2->funcAttrs));

               addDecl ($1,0,$2);
             }
   }
   ;

function_declarator
   : declarator2 '('  ')'
     {
       addDecl ($1, FUNCTION, NULL);

       // Up to C17, this was a function declarator without a prototype.
       if (!options.std_c23)
         {
           FUNC_NOPROTOTYPE($1->type) = true;
           if (!options.lessPedantic)
             werror (W_FUNCDECL_WITH_NO_PROTOTYPE);
         }
     }
   | declarator2 '('
        {
          NestLevel += LEVEL_UNIT;
          STACK_PUSH(blockNum, currBlockno);
          btree_add_child(currBlockno, ++blockNo);
          currBlockno = blockNo;
          seqPointNo++; /* not a true sequence point, but helps resolve scope */
        }
     parameter_type_list ')'
        {
          sym_link *funcType;

          bool is_fptr = IS_FUNC($1->type); // Already a function, must be a function pointer.

          addDecl ($1, FUNCTION, NULL);
          funcType = $1->type;

          // For a function pointer, the parameter list here is for the returned type.
          if (is_fptr)
            funcType = funcType->next;

          while (funcType && !IS_FUNC(funcType))
            funcType = funcType->next;

          wassert (funcType);

          FUNC_HASVARARGS(funcType) = IS_VARG($4);
          FUNC_ARGS(funcType) = reverseVal($4);

          FUNC_SDCCCALL(funcType) = -1;

          /* nest level was incremented to take care of the parms  */
          leaveBlockScope (currBlockno);
          NestLevel -= LEVEL_UNIT;
          currBlockno = STACK_POP(blockNum);
          seqPointNo++; /* not a true sequence point, but helps resolve scope */

          // if this was a pointer (to a function)
          if (!IS_FUNC($1->type))
              cleanUpLevel(SymbolTab, NestLevel + LEVEL_UNIT);

          $$ = $1;
        }
   | declarator2 '('
        {
          NestLevel += LEVEL_UNIT;
          STACK_PUSH(blockNum, currBlockno);
          btree_add_child(currBlockno, ++blockNo);
          currBlockno = blockNo;
          seqPointNo++; /* not a true sequence point, but helps resolve scope */
        }
     identifier_list ')'
        {
          if (options.std_c23)
            werror(E_OLD_STYLE,$1->name);
          
          sym_link *funcType;

          bool is_fptr = IS_FUNC($1->type); // Already a function, must be a function pointer.

          addDecl ($1, FUNCTION, NULL);
          funcType = $1->type;

          // For a function pointer, the parameter list here is for the returned type.
          if (is_fptr)
            funcType = funcType->next;

          while (funcType && !IS_FUNC(funcType))
            funcType = funcType->next;

          wassert (funcType);

          // TODO: A K&R function does not create a prototype.
          //    => use FUNC_NOPROTOTYPE, once prototype-less functions are
          //       fully supported and K&R functions can be treated as such
          funcType->funcAttrs.oldStyle = 1;

          // initially give all parameters in the identifier_list the implicit type int
          for (symbol *loop = $4; loop ; loop = loop->next) {
              value *newVal;
              loop->type = loop->etype = newIntLink();
              loop->_isparm = 1;
              newVal = symbolVal(loop);
              newVal->next = FUNC_ARGS(funcType);
              FUNC_ARGS(funcType) = newVal;
          }

          FUNC_SDCCCALL(funcType) = -1;

          /* nest level was incremented to take care of the parms  */
          leaveBlockScope (currBlockno);
          NestLevel -= LEVEL_UNIT;
          currBlockno = STACK_POP(blockNum);
          seqPointNo++; /* not a true sequence point, but helps resolve scope */

          // if this was a pointer (to a function)
          if (!IS_FUNC($1->type))
              cleanUpLevel(SymbolTab, NestLevel + LEVEL_UNIT);

          $$ = $1;
        }
   ;

pointer
   : unqualified_pointer { $$ = $1;}
   | unqualified_pointer AT '(' constant_expr ')' /* Special case to allow __at at the end */
         {
             sym_link *n = newLink(SPECIFIER);
             /* add this to the storage class specifier  */
             SPEC_ABSA(n) = 1;   /* set the absolute addr flag */
             /* now get the abs addr from value */
             SPEC_ADDR(n) = (unsigned int) ulFromVal(constExprValue($4,true));
             n->next = $1;
             $$ = n;
         }
   | unqualified_pointer type_qualifier_list
         {
             $$ = $1;
             if (IS_SPEC($2)) {
                 DCL_TSPEC($1) = $2;
                 DCL_PTR_CONST($1) = SPEC_CONST($2);
                 DCL_PTR_VOLATILE($1) = SPEC_VOLATILE($2);
                 DCL_PTR_RESTRICT($1) = SPEC_RESTRICT($2);
                 DCL_PTR_ADDRSPACE($1) = SPEC_ADDRSPACE($2);
             }
             else
                 werror (W_PTR_TYPE_INVALID);
         }
   | unqualified_pointer type_qualifier_list AT '(' constant_expr ')' /* Special case to allow __at at the end */
         {
             if (IS_SPEC($2)) {
                 DCL_TSPEC($1) = $2;
                 DCL_PTR_CONST($1) = SPEC_CONST($2);
                 DCL_PTR_VOLATILE($1) = SPEC_VOLATILE($2);
                 DCL_PTR_RESTRICT($1) = SPEC_RESTRICT($2);
                 DCL_PTR_ADDRSPACE($1) = SPEC_ADDRSPACE($2);
             }
             else
                 werror (W_PTR_TYPE_INVALID);

             sym_link *n = newLink(SPECIFIER);
             /* add this to the storage class specifier  */
             SPEC_ABSA(n) = 1;   /* set the absolute addr flag */
             /* now get the abs addr from value */
             SPEC_ADDR(n) = (unsigned int) ulFromVal(constExprValue($5,true));
             n->next = $1;
             $$ = n;
         }
   | unqualified_pointer pointer
         {
             $$ = $1;
             $$->next = $2;
             DCL_TYPE($2)=port->unqualified_pointer;
         }
   | unqualified_pointer type_qualifier_list pointer
         {
             $$ = $1;
             if (IS_SPEC($2) && DCL_TYPE($3) == UPOINTER) {
                 DCL_PTR_CONST($1) = SPEC_CONST($2);
                 DCL_PTR_VOLATILE($1) = SPEC_VOLATILE($2);
                 DCL_PTR_RESTRICT($1) = SPEC_RESTRICT($2);
                 DCL_PTR_ADDRSPACE($1) = SPEC_ADDRSPACE($2);
                 switch (SPEC_SCLS($2)) {
                 case S_XDATA:
                     DCL_TYPE($3) = FPOINTER;
                     break;
                 case S_IDATA:
                     DCL_TYPE($3) = IPOINTER;
                     break;
                 case S_PDATA:
                     DCL_TYPE($3) = PPOINTER;
                     break;
                 case S_DATA:
                     DCL_TYPE($3) = POINTER;
                     break;
                 case S_CODE:
                     DCL_TYPE($3) = CPOINTER;
                     break;
                 case S_EEPROM:
                     DCL_TYPE($3) = EEPPOINTER;
                     break;
                 default:
                   // this could be just "constant"
                   // werror(W_PTR_TYPE_INVALID);
                     ;
                 }
             }
             else
                 werror (W_PTR_TYPE_INVALID);
             $$->next = $3;
         }
   ;

unqualified_pointer
   :  '*'
      {
        $$ = newLink(DECLARATOR);
        DCL_TYPE($$)=UPOINTER;
      }
   ;

type_qualifier_list
  : type_qualifier
  | type_qualifier_list type_qualifier
               {
                 $$ = mergeDeclSpec($1, $2, "type_qualifier_list type_qualifier skipped");
               }
  ;

type_qualifier_list_opt
  :
    {
      $$ = 0;
    }
  | type_qualifier_list
    {
      $$ = $1;
    }
  ;

parameter_type_list
        : parameter_list
        | parameter_list ',' ELLIPSIS { $1->vArgs = 1;}
        ;

parameter_list
   : parameter_declaration
   | parameter_list ',' parameter_declaration
         {
            $3->next = $1;
            $$ = $3;
         }
   ;

parameter_declaration
   : declaration_specifiers declarator
        {
          symbol *loop;

          if (IS_SPEC ($1) && !IS_VALID_PARAMETER_STORAGE_CLASS_SPEC ($1))
            {
              werror (E_STORAGE_CLASS_FOR_PARAMETER, $2->name);
            }
          pointerTypes ($2->type, $1);
          if (IS_SPEC ($2->etype))
            SPEC_NEEDSPAR($2->etype) = 0;
          addDecl ($2, 0, $1);
          for (loop = $2; loop; loop->_isparm = 1, loop = loop->next)
            ;
          $$ = symbolVal ($2);
          ignoreTypedefType = 0;
        }
   | type_name
        {
          $$ = newValue ();
          $$->type = $1;
          $$->etype = getSpec ($$->type);
          ignoreTypedefType = 0;
         }
   ;

abstract_declarator
   : pointer { $$ = reverseLink($1); }
   | direct_abstract_declarator
   | pointer direct_abstract_declarator   { $1 = reverseLink($1); $2->next = $1; $$ = $2;
          if (IS_PTR($1) && IS_FUNC($2))
            DCL_TYPE($1) = CPOINTER;
        }
   ;

direct_abstract_declarator
   : '(' abstract_declarator ')'    { $$ = $2; }
   | array_abstract_declarator
   | function_abstract_declarator
   ;
   
direct_abstract_declarator_opt
   :                             { $$ = NULL; }
   | direct_abstract_declarator
   ;

array_abstract_declarator
   : direct_abstract_declarator_opt '[' ']'   {
                                       $$ = newLink (DECLARATOR);
                                       DCL_TYPE($$) = ARRAY;
                                       DCL_ELEM($$) = 0;
                                       if($1)
                                         $$->next = $1;
                                    }
   | direct_abstract_declarator_opt '[' constant_expr ']'
                                    {
                                       value *val;
                                       $$ = newLink (DECLARATOR);
                                       DCL_TYPE($$) = ARRAY;
                                       DCL_ELEM($$) = (int) ulFromVal(val = constExprValue($3, true));
                                       if($1)
                                         $$->next = $1;
                                    }
   ;

function_abstract_declarator
   : '(' ')'                        { $$ = NULL;}
   | direct_abstract_declarator '(' ')' {
     // $1 must be a pointer to a function
     sym_link *p=newLink(DECLARATOR);
     DCL_TYPE(p) = FUNCTION;
     if (!$1) {
       // ((void (code *) ()) 0) ()
       $1=newLink(DECLARATOR);
       DCL_TYPE($1)=CPOINTER;
       $$ = $1;
     }
     $1->next=p;
   }
   | '(' parameter_type_list ')'    { $$ = NULL;}
   | direct_abstract_declarator '('
        {
          NestLevel += LEVEL_UNIT;
          STACK_PUSH(blockNum, currBlockno);
          btree_add_child(currBlockno, ++blockNo);
          currBlockno = blockNo;
        }
     parameter_type_list ')'
        {
          sym_link *p = newLink(DECLARATOR), *q;
          DCL_TYPE(p) = FUNCTION;

          FUNC_HASVARARGS(p) = IS_VARG($4);
          FUNC_ARGS(p) = reverseVal($4);

          /* nest level was incremented to take care of the parms  */
          NestLevel -= LEVEL_UNIT;
          currBlockno = STACK_POP(blockNum);
          if (!$1)
            {
              /* ((void (code *) (void)) 0) () */
              $1 = newLink(DECLARATOR);
              DCL_TYPE($1) = CPOINTER;
              $$ = $1;
            }
          for (q = $1; q && q->next; q = q->next);
          q->next = p;
        }
   ;

initializer
   : assignment_expr                { $$ = newiList(INIT_NODE,$1); }
   | '{' '}'
     {
       if (!options.std_c23)
         werror(W_EMPTY_INIT_C23);
       // {} behaves mostly like {0}, so we emulate that, and use the isempty flag for the few cases where there is a difference.
       $$ = newiList(INIT_DEEP, revinit(newiList(INIT_NODE, newAst_VALUE(constIntVal("0")))));
       $$->isempty = true;
     }
   | '{'  initializer_list '}'      { $$ = newiList(INIT_DEEP,revinit($2)); }
   | '{'  initializer_list ',' '}'  { $$ = newiList(INIT_DEEP,revinit($2)); }
   ;

initializer_list
   : designation_opt initializer    { $2->designation = $1; $$ = $2; }
   | initializer_list ',' designation_opt initializer
                                    {
                                       $4->designation = $3;
                                       $4->next = $1;
                                       $$ = $4;
                                    }
   ;

designation_opt
   :                             { $$ = NULL; }
   | designation
   ;

designation
   : designator_list '='         { $$ = revDesignation($1); }
   ;

designator_list
   : designator
   | designator_list designator  { $2->next = $1; $$ = $2; }
   ;

designator
   : '[' constant_expr ']'
         {
            value *tval;
            int elemno;

            tval = constExprValue($2, true);
            /* if it is not a constant then Error  */
            if (!tval || (SPEC_SCLS(tval->etype) != S_LITERAL))
              {
                werror (E_CONST_EXPECTED);
                elemno = 0; /* arbitrary fixup */
              }
            else
              {
                if ((elemno = (int) ulFromVal(tval)) < 0)
                  {
                    werror (E_BAD_DESIGNATOR);
                    elemno = 0; /* arbitrary fixup */
                  }
              }
            $$ = newDesignation(DESIGNATOR_ARRAY, &elemno);
         }
   | '.' identifier              { $$ = newDesignation(DESIGNATOR_STRUCT,$2); }
   ;

static_assert_declaration
   : STATIC_ASSERT '(' constant_expr ',' STRING_LITERAL ')' ';'
                                    {
                                       value *val = constExprValue ($3, true);
                                       if (!val)
                                         werror (E_CONST_EXPECTED);
                                       else if (!ullFromVal(val))
                                         werror (W_STATIC_ASSERTION, $5);
                                    }
   | STATIC_ASSERT '(' constant_expr ')' ';'
                                    {
                                       value *val = constExprValue ($3, true);
                                       if (!options.std_c23)
                                         werror (E_STATIC_ASSERTION_C23);
                                       if (!val)
                                         werror (E_CONST_EXPECTED);
                                       else if (!ullFromVal(val))
                                         werror (W_STATIC_ASSERTION_2);
                                    }
   ;

attribute_specifier_sequence
   : attribute_specifier_sequence attribute_specifier
     {
       $$ = $1;
       attribute *a;
       for (a = $$; a->next; a = a->next);
       a->next = $2;
     }
   | attribute_specifier
     {
       $$ = $1;
     }
   ;

attribute_specifier_sequence_opt
   : /* empty */
     {
       $$ = 0;
     }
   | attribute_specifier_sequence
     {
       $$ = $1;
     }
   ;

attribute_specifier
   : ATTR_START attribute_list ']' ']'
     {
       if (!options.std_c23)
         werror(E_ATTRIBUTE_C23);
       $$ = $2;
     }
   ;

attribute_list
   : attribute_opt
     {
       $$ = $1;
     }
   | attribute_list ',' attribute_opt
     {
       $$ = $1;
       if ($3)
         {
           attribute *a;
           for (a = $$; a->next; a = a->next);
           a->next = $3;
         }
     }
   ;

attribute
   : attribute_token
   {
     $$ = newAttribute ($1, 0);
   }
   | attribute_token attribute_argument_clause
   {
     $$ = newAttribute ($1, 0);
   }
   ;

attribute_opt
   : /* empty */
     {
       $$ = 0;
     }
   | attribute
     {
       $$ = $1;
     }
   ;

attribute_token
   : identifier
     {
       $$ = $1;
       $$->next = 0;
       werror (W_UNKNOWN_ATTRIBUTE, $1->name);
     }
   | identifier TOK_SEP identifier
     {
       $$ = $1;
       $$->next = $3;
       struct dbuf_s dbuf;
       dbuf_init (&dbuf, 64);
       dbuf_printf (&dbuf, "%s::%s", $1->name, $3->name);
       werror (W_UNKNOWN_ATTRIBUTE, dbuf_c_str (&dbuf));
       dbuf_destroy (&dbuf);
     }
   ;

attribute_argument_clause
   : '(' ')'
   | '(' balanced_token_sequence ')'
   ;

balanced_token_sequence
   : balanced_token
   | balanced_token_sequence balanced_token
   ;

balanced_token
   : identifier
   | STRING_LITERAL
   | CONSTANT
   ;

   /* C23 A.2.3 Statements */

statement
   : labeled_statement
   | unlabeled_statement
   ;
   

unlabeled_statement
   : expression_statement
   | attribute_specifier_sequence_opt primary_block
     {
       $$ = $2;
     }
   | attribute_specifier_sequence_opt jump_statement
     {
       $$ = $2;
     }
   | critical_statement
   | asm_statement
   ;

primary_block
   : compound_statement
     {
       $$ = $1;
     }
   | selection_statement
     {
       $$ = $1;
     }
   | iteration_statement
     {
       $$ = $1;
     }
   ;

secondary_block
   : statement
     {
       $$ = $1;
     }
   ;

labeled_statement
   : label statement 
     {
       if ($1)
         {
           $$ = $1;
           $1->right = $2;
         }
       else
         $$ = newNode (BLOCK, NULL, NULL);
     }
   ;

label
   : identifier ':'
     {
       $$ = createLabel($1,NULL);
       $1->isitmp = 0;
     }
   | attribute_specifier_sequence_opt CASE constant_range_expr ':'
     {
       if (!options.std_c2y)
         werror (E_CASE_RANGE_C2Y);

       if (STACK_EMPTY(swStk))
         $$ = createCaseRange(NULL,$3->left,$3->right,NULL);
       else
         $$ = createCaseRange(STACK_PEEK(swStk),$3->left,$3->right,NULL);
     }
   | attribute_specifier_sequence_opt CASE constant_expr ':'
     {
       if (STACK_EMPTY(swStk))
         $$ = createCase(NULL,$3,NULL);
       else
         $$ = createCase(STACK_PEEK(swStk),$3,NULL);
     }
   | attribute_specifier_sequence_opt DEFAULT { $<asts>$ = newNode(DEFAULT,NULL,NULL); } ':'
     {
       if (STACK_EMPTY(swStk))
         $$ = createDefault(NULL,$<asts>3,NULL);
       else
         $$ = createDefault(STACK_PEEK(swStk),$<asts>3,NULL);
     }
   ;

start_block
   : '{'
        {
          NestLevel += LEVEL_UNIT;
          STACK_PUSH(blockNum, currBlockno);
          btree_add_child(currBlockno, ++blockNo);
          currBlockno = blockNo;
          ignoreTypedefType = 0;
        }
   ;

end_block
   : '}'
        {
          leaveBlockScope (currBlockno);
          NestLevel -= LEVEL_UNIT;
          currBlockno = STACK_POP(blockNum);
        }
   ;

compound_statement
   : start_block end_block                    { $$ = createBlock(NULL, NULL); }
   | start_block block_item_list end_block
     {
       $$ = $2;
       cleanUpLevel(StructTab, NestLevel + LEVEL_UNIT);
     }
   ;

block_item_list
   : statements_and_implicit                  { $$ = createBlock(NULL, $1); }
   | declaration_list                         { $$ = createBlock($1, NULL); }
   | declaration_list statements_and_implicit { $$ = createBlock($1, $2); }
   ;

expression_statement
   : expression_opt ';'
   | attribute_specifier_sequence expression ';'           { $$ = $2; seqPointNo++;}
   ;

else_statement
   :  ELSE  statement   { $$ = $2; }
   |                    { $$ = NULL; }
   ;

selection_statement
   /* This gives us an unavoidable shift / reduce conflict, but yacc does the right thing for C */
   : IF '(' expression ')' { seqPointNo++;} statement else_statement
                           {
                              noLineno++;
                              $$ = createIf ($3, $6, $7 );
                              $$->lineno = $3->lineno;
                              $$->filename = $3->filename;
                              noLineno--;
                           }
   | SWITCH '(' expression ')'   {
                              ast *ex;
                              static   int swLabel = 0;

                              seqPointNo++;
                              /* create a node for expression  */
                              ex = newNode(SWITCH,$3,NULL);
                              STACK_PUSH(swStk,ex);   /* save it in the stack */
                              ex->values.switchVals.swNum = swLabel;

                              /* now create the label */
                              SNPRINTF(lbuff, sizeof(lbuff),
                                       "_swBrk_%d",swLabel++);
                              $<sym>$  =  newSymbol(lbuff,NestLevel);
                              /* put label in the break stack  */
                              STACK_PUSH(breakStack,$<sym>$);
                           }
     secondary_block       {
                              /* get back the switch form the stack  */
                              $$ = STACK_POP(swStk);
                              $$->right = newNode (NULLOP,$6,createLabel($<sym>5,NULL));
                              STACK_POP(breakStack);
                           }
   ;

iteration_statement
   : while '(' expression ')' { seqPointNo++;} secondary_block
                         {
                           noLineno++;
                           $$ = createWhile ( $1, STACK_POP(continueStack),
                                              STACK_POP(breakStack), $3, $6 );
                           $$->lineno = $1->lineDef;
                           $$->filename = $1->fileDef;
                           noLineno--;
                         }
   | do secondary_block WHILE '(' expression ')' ';'
                        {
                          seqPointNo++;
                          noLineno++;
                          $$ = createDo ( $1 , STACK_POP(continueStack),
                                          STACK_POP(breakStack), $5, $2);
                          $$->lineno = $1->lineDef;
                          $$->filename = $1->fileDef;
                          noLineno--;
                        }
   | for '(' expression_opt   ';' expression_opt ';' expression_opt ')' secondary_block
                        {
                          noLineno++;

                          $$ = newNode(FOR,$9,NULL);
                          AST_FOR($$,trueLabel) = $1;
                          AST_FOR($$,continueLabel) =  STACK_POP(continueStack);
                          AST_FOR($$,falseLabel) = STACK_POP(breakStack);
                          AST_FOR($$,condLabel)  = STACK_POP(forStack);
                          AST_FOR($$,initExpr)   = $3;
                          AST_FOR($$,condExpr)   = $5;
                          AST_FOR($$,loopExpr)   = $7;
                          
                          /* This continue label is not at the correct location, */
                          /* but we need to create it now for proper binding. The */
                          /* code that handles the FOR node will move it to the */
                          /* proper location inside the for loop. */
                          if (AST_FOR($$,continueLabel)->isref)
                            $$->right = createLabel(AST_FOR($$,continueLabel),NULL);
                          $$ = newNode(NULLOP,$$,createLabel(AST_FOR($$,falseLabel),NULL));
                          noLineno--;

                          NestLevel -= LEVEL_UNIT;
                          currBlockno = STACK_POP(blockNum);
                        }
   | for '(' declaration expression_opt ';' expression_opt ')'
                        {
                          if (!options.std_c99)
                            werror (E_FOR_INITAL_DECLARATION_C99);

                          if ( $3 && IS_TYPEDEF($3->etype))
                            allocVariables ($3);
                          ignoreTypedefType = 0;
                          addSymChain(&$3);
                        }
     secondary_block
                        {

                          noLineno++;

                          $$ = newNode(FOR,$9,NULL);
                          AST_FOR($$,trueLabel) = $1;
                          AST_FOR($$,continueLabel) =  STACK_POP(continueStack);
                          AST_FOR($$,falseLabel) = STACK_POP(breakStack);
                          AST_FOR($$,condLabel)  = STACK_POP(forStack);
                          AST_FOR($$,initExpr)   = 0;
                          AST_FOR($$,condExpr)   = $4;
                          AST_FOR($$,loopExpr)   = $6;

                          /* This continue label is not at the correct location, */
                          /* but we need to create it now for proper binding. The */
                          /* code that handles the FOR node will move it to the */
                          /* proper location inside the for loop. */
                          if (AST_FOR($$,continueLabel)->isref)
                            $$->right = createLabel(AST_FOR($$,continueLabel),NULL);
                          $$ = createBlock($3, newNode(NULLOP,$$,createLabel(AST_FOR($$,falseLabel),NULL)));
                          cleanUpLevel(StructTab, NestLevel + LEVEL_UNIT);
                          noLineno--;

                          leaveBlockScope (currBlockno);
                          NestLevel -= LEVEL_UNIT;
                          currBlockno = STACK_POP(blockNum);
                        }
   ;

jump_statement
   : GOTO identifier ';'   {
                              if (inCriticalBlock) {
                                werror(E_INVALID_CRITICAL);
                                $$ = NULL;
                              } else {
                                $2->islbl = 1;
                                $$ = newAst_VALUE(symbolVal($2));
                                $$ = newNode(GOTO,$$,NULL);
                              }
                           }
   | CONTINUE ';'          {
       /* make sure continue is in context */
       if (STACK_EMPTY(continueStack) || STACK_PEEK(continueStack) == NULL) {
           werror(E_BREAK_CONTEXT);
           $$ = NULL;
       }
       else {
           $$ = newAst_VALUE(symbolVal(STACK_PEEK(continueStack)));
           $$ = newNode(GOTO,$$,NULL);
           /* mark the continue label as referenced */
           STACK_PEEK(continueStack)->isref = 1;
       }
   }
   | BREAK ';'             {
       if (STACK_EMPTY(breakStack) || STACK_PEEK(breakStack) == NULL) {
           werror(E_BREAK_CONTEXT);
           $$ = NULL;
       } else {
           $$ = newAst_VALUE(symbolVal(STACK_PEEK(breakStack)));
           $$ = newNode(GOTO,$$,NULL);
           STACK_PEEK(breakStack)->isref = 1;
       }
   }
   | RETURN ';'            {
       seqPointNo++;
       if (inCriticalBlock) {
           werror(E_INVALID_CRITICAL);
           $$ = NULL;
       } else {
           $$ = newNode(RETURN,NULL,NULL);
       }
   }
   | RETURN expression ';'       {
       seqPointNo++;
       if (inCriticalBlock) {
           werror(E_INVALID_CRITICAL);
           $$ = NULL;
       } else {
           $$ = newNode(RETURN,NULL,$2);
       }
   }
   ;

   /* C23 A.2.4 External definitions */

translation_unit
   : external_declaration
   | translation_unit external_declaration
   ;

external_declaration
   : function_definition
        {
          // blockNo = 0;
        }
   | declaration
        {
          ignoreTypedefType = 0;
          if ($1 && $1->type && IS_FUNC($1->type))
            {
              /* The only legal storage classes for
               * a function prototype (declaration)
               * are extern and static. extern is the
               * default. Thus, if this function isn't
               * explicitly marked static, mark it
               * extern.
               */
              if ($1->etype && IS_SPEC($1->etype) && !SPEC_STAT($1->etype))
                {
                  SPEC_EXTR($1->etype) = 1;
                }
            }
          addSymChain (&$1);
          allocVariables ($1);
          cleanUpLevel (SymbolTab, 1);
        }
   | addressmod
        {
          /* These empty braces here are apparently required by some version of GNU bison on MS Windows. See bug #2858. */
        }
   ;

function_definition
   : declarator
        {   /* function return type not specified, which is allowed in C90 (and means int), but disallowed in C99 and later */
            werror (W_RETURN_TYPE_OMITTED_INT);
            addDecl($1,0,newIntLink());
            $1 = createFunctionDecl($1);
            if ($1)
                {
                    if (FUNC_ISCRITICAL ($1->type))
                        inCriticalFunction = 1;
                    strncpyz (function_name, $1->name, sizeof (function_name) - 3);
                    memset (function_name + sizeof (function_name) - 4, 0x00, 4);
                }
        }
   function_body
        {
            // merge kr_declaration_list from auxiliary node into function declaration
            mergeKRDeclListIntoFuncDecl($1, (symbol *) $3->left);
            // discard auxiliary node and keep compound_statement as function_body
            $3 = $3->right;

            $$ = createFunction($1, $3);
            if ($1 && FUNC_ISCRITICAL ($1->type))
                inCriticalFunction = 0;
        }
   | declaration_specifiers declarator
        {
            sym_link *p = copyLinkChain($1);
            pointerTypes($2->type,p);
            addDecl($2,0,p);
            $2 = createFunctionDecl($2);
            if ($2)
                {
                    if (!strcmp ($2->name, "_sdcc_external_startup")) // The rename (and semantics change happened) in SDCC 4.2.10. Keep this warning for two major releases afterwards.
                        werror (W__SDCC_EXTERNAL_STARTUP_DEF);
                    if (FUNC_ISCRITICAL ($2->type))
                        inCriticalFunction = 1;
                    // warn for loss of calling convention for inlined functions.
                    if (FUNC_ISINLINE ($2->type) &&
                        ( FUNC_ISZ88DK_CALLEE ($2->type) || FUNC_ISZ88DK_FASTCALL ($2->type) ||
                          FUNC_BANKED ($2->type)         || FUNC_REGBANK ($2->type)          ||
                          FUNC_ISOVERLAY ($2->type)      || FUNC_ISISR ($2->type) ))
                        werror (W_INLINE_FUNCATTR, $2->name);
                    strncpyz (function_name, $2->name, sizeof (function_name) - 3);
                    memset (function_name + sizeof (function_name) - 4, 0x00, 4);
                }
        }
   function_body
        {
            // merge kr_declaration_list from auxiliary node into function declaration
            mergeKRDeclListIntoFuncDecl($2, (symbol *) $4->left);
            // discard auxiliary node and keep compound_statement as function_body
            $4 = $4->right;

            $$ = createFunction($2, $4);
            if ($2 && FUNC_ISCRITICAL ($2->type))
                inCriticalFunction = 0;
        }
   ;

function_body
   : compound_statement
     {
       // auxiliary node transports kr_declaration_list into function_definition, where the node is discarded
       $$ = newNode (0, NULL, $1);
     }
   | kr_declaration_list compound_statement
     {
       // auxiliary node transports kr_declaration_list into function_definition, where the node is discarded
       $$ = newNode (0, (ast *) $1, $2);
     }
   ;

   /* SDCC-specific stuff */

file
   : /* empty */
     {
       werror(W_EMPTY_TRANSLATION_UNIT);
     }
   | translation_unit
   ;




function_attributes
   : function_attribute
   | function_attributes function_attribute { $$ = mergeSpec($1,$2,"function_attribute"); }
   ;

function_attribute
   :  USING '(' constant_expr ')' {
                        $$ = newLink(SPECIFIER);
                        FUNC_REGBANK($$) = (int) ulFromVal(constExprValue($3, true));
                     }
   |  REENTRANT      {  $$ = newLink (SPECIFIER);
                        FUNC_ISREENT($$)=1;
                     }
   |  CRITICAL       {  $$ = newLink (SPECIFIER);
                        FUNC_ISCRITICAL($$) = 1;
                     }
   |  NAKED          {  $$ = newLink (SPECIFIER);
                        FUNC_ISNAKED($$)=1;
                     }
   |  JAVANATIVE     {  $$ = newLink (SPECIFIER);
                        FUNC_ISJAVANATIVE($$)=1;
                     }
   |  OVERLAY        {  $$ = newLink (SPECIFIER);
                        FUNC_ISOVERLAY($$)=1;
                     }
   |  NONBANKED      {$$ = newLink (SPECIFIER);
                        FUNC_NONBANKED($$) = 1;
                        if (FUNC_BANKED($$)) {
                            werror(W_BANKED_WITH_NONBANKED);
                        }
                     }
   |  SHADOWREGS     {$$ = newLink (SPECIFIER);
                        FUNC_ISSHADOWREGS($$) = 1;
                     }
   |  SD_WPARAM      {$$ = newLink (SPECIFIER);
                        FUNC_ISWPARAM($$) = 1;
                     }
   |  BANKED         {$$ = newLink (SPECIFIER);
                        FUNC_BANKED($$) = 1;
                        if (FUNC_NONBANKED($$)) {
                            werror(W_BANKED_WITH_NONBANKED);
                        }
                     }
   |  Interrupt_storage
                     {
                        $$ = newLink (SPECIFIER);
                        FUNC_INTNO($$) = $1;
                        FUNC_ISISR($$) = 1;
                     }
   |  TRAP
                     {
                        $$ = newLink (SPECIFIER);
                        FUNC_INTNO($$) = INTNO_TRAP;
                        FUNC_ISISR($$) = 1;
                     }
   |  SMALLC         {  $$ = newLink (SPECIFIER);
                        FUNC_ISSMALLC($$) = 1;
                     }
   |  RAISONANCE     {  $$ = newLink (SPECIFIER);
                        FUNC_ISRAISONANCE($$) = 1;
                     }
   |  IAR            {  $$ = newLink (SPECIFIER);
                        FUNC_ISIAR($$) = 1;
                     }
   |  COSMIC         {  $$ = newLink (SPECIFIER);
                        FUNC_ISCOSMIC($$) = 1;
                     }
   |  SDCCCALL '(' constant_expr ')'
                     {  $$ = newLink (SPECIFIER);
                        FUNC_SDCCCALL($$) = ulFromVal(constExprValue ($3, true));
                     }
   |  Z88DK_FASTCALL {  $$ = newLink (SPECIFIER);
                        FUNC_ISZ88DK_FASTCALL($$) = 1;
                     }
   |  Z88DK_CALLEE   {  $$ = newLink (SPECIFIER);
                        FUNC_ISZ88DK_CALLEE($$) = 1;
                     }
   |  Z88DK_PARAMS_OFFSET '(' constant_expr ')' 
                     {
                        value *offset_v = constExprValue ($3, true);
                        int    offset = 0;
                        $$ = newLink(SPECIFIER);
                        if  ( offset_v ) 
                          offset = ulFromVal(offset_v);
                        $$->funcAttrs.z88dk_params_offset = offset;
                     } 
   |  Z88DK_SHORTCALL '(' constant_expr ',' constant_expr ')'
                     {
                        value *rst_v = constExprValue ($3, true);
                        value *value_v = constExprValue ($5, true);
                        int rst = -1, value = -1;
                        $$ = newLink(SPECIFIER);

                        if  ( rst_v ) 
                          rst = ulFromVal(rst_v);
                        if  ( value_v ) 
                          value = ulFromVal(value_v);
          
                        if ( rst < 0 || rst > 56 || ( rst % 8 ) )
                          {
                            werror(E_SHORTCALL_INVALID_VALUE, "rst", rst);
                          }
                        if ( value < 0 || value > 0xfff )
                          {
                            werror(E_SHORTCALL_INVALID_VALUE, "value", value);
                          }
                        $$->funcAttrs.z88dk_shortcall_rst = rst;
                        $$->funcAttrs.z88dk_shortcall_val = value;
                        FUNC_ISZ88DK_SHORTCALL($$) = 1;
                     }
   |  PRESERVES_REGS '(' identifier_list ')'
                     {
                        const struct symbol *regsym;
                        $$ = newLink (SPECIFIER);

                        for(regsym = $3; regsym; regsym = regsym->next)
                          {
                            int regnum;

                            if (!port->getRegByName || ((regnum = port->getRegByName(regsym->name)) < 0))
                              werror (W_UNKNOWN_REG, regsym->name);
                            else
                              $$->funcAttrs.preserved_regs[regnum] = true;
                          }
                     }
   ;

offsetof_member_designator
   : identifier      { $$ = newAst_VALUE (symbolVal ($1)); }
   | offsetof_member_designator '.' { ignoreTypedefType = 1; } identifier
                     {
                       ignoreTypedefType = 0;
                       $4 = newSymbol ($4->name, NestLevel);
                       $4->implicit = 1;
                       $$ = newNode ('.', $1, newAst_VALUE (symbolVal ($4)));
                     }
   | offsetof_member_designator '[' expression ']'
                     {
                       $$ = newNode ('[', $1, $3);
                     }
   ;

string_literal_val
   : FUNC
       {
         // essentially do $$ = newAst_VALUE (strVal("\"$function_name\""));

         value* val = newValue ();
         { // BUG: duplicate from strVal
           val->type = newLink (DECLARATOR);
           DCL_TYPE (val->type) = ARRAY;
           val->type->next = val->etype = newLink (SPECIFIER);
           SPEC_SCLS (val->etype) = S_LITERAL;
           SPEC_CONST (val->etype) = 1;
           SPEC_NOUN (val->etype) = V_CHAR;
           SPEC_USIGN (val->etype) = !options.signed_char;
           val->etype->select.s.b_implicit_sign = true;
         }

         int ll = 1 + strlen(function_name);
         char* s = (char*) Safe_alloc(ll*sizeof(char));
         if(ll > 1){
           s = strncpy(s, function_name, ll);
         }else{
           *s = 0;
         }
         SPEC_CVAL (val->etype).v_char = s;
         DCL_ELEM (val->type) = ll;
         $$ = newAst_VALUE ( val );
       }
   | STRING_LITERAL { $$ = newAst_VALUE (strVal ($1)); }
   ;

Interrupt_storage
   : INTERRUPT { $$ = INTNO_UNSPEC; }
   | INTERRUPT '(' constant_expr ')'
        { 
          value *val = constExprValue($3, true);
          int intno = (int) ulFromVal(val);
          if (val && (intno >= 0) && (intno <= INTNO_MAX))
            $$ = intno;
          else
            {
              werror(E_INT_BAD_INTNO, intno);
              $$ = INTNO_UNSPEC;
            }
        }
   ;

sfr_reg_bit
   :  SBIT  {
               $$ = newLink(SPECIFIER);
               SPEC_NOUN($$) = V_SBIT;
               SPEC_SCLS($$) = S_SBIT;
               SPEC_BLEN($$) = 1;
               SPEC_BSTR($$) = 0;
               ignoreTypedefType = 1;
            }
   |  sfr_attributes
   ;

sfr_attributes
   : SFR    {
               $$ = newLink(SPECIFIER);
               FUNC_REGBANK($$) = 0;
               SPEC_NOUN($$)    = V_CHAR;
               SPEC_SCLS($$)    = S_SFR;
               SPEC_USIGN($$)   = 1;
               ignoreTypedefType = 1;
            }
   | SFR BANKED {
               $$ = newLink(SPECIFIER);
               FUNC_REGBANK($$) = 1;
               SPEC_NOUN($$)    = V_CHAR;
               SPEC_SCLS($$)    = S_SFR;
               SPEC_USIGN($$)   = 1;
               ignoreTypedefType = 1;
            }
   ;

sfr_attributes
   : SFR16  {
               $$ = newLink(SPECIFIER);
               FUNC_REGBANK($$) = 0;
               SPEC_NOUN($$)    = V_INT;
               SPEC_SCLS($$)    = S_SFR;
               SPEC_USIGN($$)   = 1;
               ignoreTypedefType = 1;
            }
   ;

sfr_attributes
   : SFR32  {
               $$ = newLink(SPECIFIER);
               FUNC_REGBANK($$) = 0;
               SPEC_NOUN($$)    = V_INT;
               SPEC_SCLS($$)    = S_SFR;
               SPEC_LONG($$)    = 1;
               SPEC_USIGN($$)   = 1;
               ignoreTypedefType = 1;
            }
   ;

opt_stag
   : stag
   |    {  /* synthesize a name add to structtable */
          ignoreTypedefType = 0;
          $$ = newStruct(genSymName(NestLevel));
          $$->level = NestLevel;
          $$->block = currBlockno;
          $$->tagsym = NULL;
          //addSym (StructTab, $$, $$->tag, $$->level, currBlockno, 0);
        }
   ;

stag
   : identifier
        {  /* add name to structure table */
          ignoreTypedefType = 0;
          $$ = newStruct($1->name);
          $$->level = NestLevel;
          $$->block = currBlockno;
          $$->tagsym = $1;
          //$$ = findSymWithBlock (StructTab, $1, currBlockno);
          //if (! $$ )
          //  {
          //    $$ = newStruct($1->name);
          //    $$->level = NestLevel;
          //    $$->tagsym = $1;
          //    //addSym (StructTab, $$, $$->tag, $$->level, currBlockno, 0);
          //  }
        }
   ;

opt_assign_expr
   : '=' constant_expr
        {
          value *val;

          val = constExprValue($2, true);
          if (!val) // Not a constant expression
            {
              werror (E_CONST_EXPECTED);
              val = constIntVal("0");
            }
          else if (!IS_INT(val->type) && !IS_CHAR(val->type) && !IS_BOOL(val->type))
            {
              werror(E_ENUM_NON_INTEGER);
              SNPRINTF(lbuff, sizeof(lbuff), "%lld", (long long int) ullFromVal(val));
              val = constVal(lbuff);
            }
          $$ = cenum = val;
        }
   |    {
          if (cenum)
            {
              SNPRINTF(lbuff, sizeof(lbuff), "%lld", (long long int) ullFromVal(cenum)+1);
              $$ = cenum = constVal(lbuff);
            }
          else
            {
              $$ = cenum = constCharVal(0);
            }
        }
   ;

specifier_qualifier_list : type_specifier_list_ { $$ = finalizeSpec($1); };

type_specifier_list_
   : type_specifier_qualifier
   //| type_specifier_list_ type_specifier         {  $$ = mergeSpec ($1,$2, "type_specifier_list"); }
   | type_specifier_list_ type_specifier_qualifier {
     /* if the decl $2 is not a specifier */
     /* find the spec and replace it      */
     $$ = mergeDeclSpec($1, $2, "type_specifier_list type_specifier skipped");
   }
   ;

type_specifier_list_without_struct_or_union
   : type_specifier_qualifier_without_struct_or_union
   | type_specifier_list_without_struct_or_union type_specifier_qualifier_without_struct_or_union
        {
          /* if the decl $2 is not a specifier */
          /* find the spec and replace it      */
          $$ = mergeDeclSpec($1, $2, "type_specifier_list type_specifier skipped");
        }
   ;

identifier_list
   : identifier
   | identifier_list ',' identifier
         {
           $3->next = $1;
           $$ = $3;
         }
   ;

type_name
   : declaration_specifiers
        {
          if (IS_SPEC ($1) && !IS_VALID_PARAMETER_STORAGE_CLASS_SPEC ($1))
            {
              werror (E_STORAGE_CLASS_FOR_PARAMETER, "type name");
            }
          $$ = $1; ignoreTypedefType = 0;
        }
   | declaration_specifiers abstract_declarator
        {
          /* go to the end of the list */
          sym_link *p;

          if (IS_SPEC ($1) && !IS_VALID_PARAMETER_STORAGE_CLASS_SPEC ($1))
            {
              werror (E_STORAGE_CLASS_FOR_PARAMETER, "type name");
            }
          pointerTypes ($2,$1);
          for (p = $2; p && p->next; p = p->next)
            ;
          if (!p)
            {
              werror(E_SYNTAX_ERROR, yytext);
            }
          else
            {
              p->next = $1;
            }
          $$ = $2;
          ignoreTypedefType = 0;
        }
   ;

critical
   : CRITICAL   {
                   if (inCriticalFunction || inCriticalBlock)
                     werror(E_INVALID_CRITICAL);
                   inCriticalBlock = 1;
                   STACK_PUSH(continueStack,NULL);
                   STACK_PUSH(breakStack,NULL);
                   $$ = NULL;
                }
   ;

critical_statement
   : critical statement  {
                   STACK_POP(breakStack);
                   STACK_POP(continueStack);
                   $$ = newNode(CRITICAL,$2,NULL);
                   inCriticalBlock = 0;
                }
   ;

statements_and_implicit
   : statement_list
   | statement_list implicit_block
     {
       $$ = newNode(NULLOP, $1, $2);
       if (!options.std_c99)
         werror(E_DECL_AFTER_STATEMENT_C99);
     }
   ;

declaration_after_statement
   : {
       NestLevel += SUBLEVEL_UNIT;
       STACK_PUSH(blockNum, currBlockno);
       btree_add_child(currBlockno, ++blockNo);
       currBlockno = blockNo;
       ignoreTypedefType = 0;
     }
     declaration_list                         { $$ = $2; }
   ;

implicit_block
   : declaration_after_statement statements_and_implicit
     {
       leaveBlockScope (currBlockno);
       NestLevel -= SUBLEVEL_UNIT;
       currBlockno = STACK_POP(blockNum);
       $$ = createBlock($1, $2);
       cleanUpLevel(StructTab, NestLevel + SUBLEVEL_UNIT);
     }
   | declaration_after_statement
     {
       leaveBlockScope (currBlockno);
       NestLevel -= SUBLEVEL_UNIT;
       currBlockno = STACK_POP(blockNum);
       $$ = createBlock($1, NULL);
       cleanUpLevel(StructTab, NestLevel + SUBLEVEL_UNIT);
     }
   ;

declaration_list
   : declaration
     {
       /* if this is typedef declare it immediately */
       if ( $1 && IS_TYPEDEF($1->etype)) {
         allocVariables ($1);
         $$ = NULL;
       }
       else
         $$ = $1;
       ignoreTypedefType = 0;/*printf("1 %s %d %d\n", $1->name, IS_ARRAY ($1->type), DCL_ARRAY_VLA ($1->type));
       if (IS_ARRAY ($1->type) && DCL_ARRAY_VLA ($1->type))
         werror (E_VLA_OBJECT);
       else*/
         addSymChain(&$1);
     }

   | declaration_list declaration
     {
       symbol   *sym;

       /* if this is a typedef */
       if ($2 && IS_TYPEDEF($2->etype)) {
         allocVariables ($2);
         $$ = $1;
       }
       else {
         /* get to the end of the previous decl */
         if ( $1 ) {
           $$ = sym = $1;
           while (sym->next)
             sym = sym->next;
           sym->next = $2;
         }
         else
           $$ = $2;
       }
       ignoreTypedefType = 0;/*printf("2 %s %d %d\n", $1->name, IS_ARRAY ($1->type), DCL_ARRAY_VLA ($1->type));
       if (IS_ARRAY ($2->type) && DCL_ARRAY_VLA ($2->type))
         werror (E_VLA_OBJECT);
       else*/
         addSymChain(&$2);
     }
   ;

// The parameter declarations in K&R-style functions need to be handled in a special way to avoid
// ambiguities in the grammer. We do this by not allowing attributes on the parameter declarations.
// Otherwise, in e.g.
//  void f(x) [[attribute]] int x; {}
// it would be unclear if the attribute applies to the type of f vs. the declaration of x.
kr_declaration
   : declaration_specifiers init_declarator_list ';'
      {
         /* add the specifier list to the id */
         symbol *sym , *sym1;

         for (sym1 = sym = reverseSyms($2);sym != NULL;sym = sym->next) {
             sym_link *lnk = copyLinkChain($1);
             sym_link *l0 = NULL, *l1 = NULL, *l2 = NULL;
             /* check illegal declaration */
             for (l0 = sym->type; l0 != NULL; l0 = l0->next)
               if (IS_PTR (l0))
                 break;
             /* check if creating instances of structs with flexible arrays */
             for (l1 = lnk; l1 != NULL; l1 = l1->next)
               if (IS_STRUCT (l1) && SPEC_STRUCT (l1)->b_flexArrayMember)
                 break;
             if (!options.std_c99 && l0 == NULL && l1 != NULL && SPEC_EXTR($1) != 1)
               werror (W_FLEXARRAY_INSTRUCT, sym->name);
             /* check if creating instances of function type */
             for (l1 = lnk; l1 != NULL; l1 = l1->next)
               if (IS_FUNC (l1))
                 break;
             for (l2 = lnk; l2 != NULL; l2 = l2->next)
               if (IS_PTR (l2))
                 break;
             if (l0 == NULL && l2 == NULL && l1 != NULL)
               werrorfl(sym->fileDef, sym->lineDef, E_TYPE_IS_FUNCTION, sym->name);
             /* do the pointer stuff */
             pointerTypes(sym->type,lnk);
             addDecl (sym,0,lnk);
         }

         uselessDecl = true;
         $$ = sym1;
      }
   ;

kr_declaration_list
   : kr_declaration
     {
       /* if this is typedef declare it immediately */
       if ( $1 && IS_TYPEDEF($1->etype)) {
         allocVariables ($1);
         $$ = NULL;
         ignoreTypedefType = 0;
         addSymChain(&$1);
       }
       else {
         checkTypeSanity($1->etype, $1->name);
         $$ = $1;
       }
     }
   | kr_declaration_list kr_declaration
     {
       symbol   *sym;

       /* if this is a typedef */
       if ($2 && IS_TYPEDEF($2->etype)) {
         allocVariables ($2);
         $$ = $1;
         ignoreTypedefType = 0;
         addSymChain(&$2);
       }
       else {
         checkTypeSanity($2->etype, $2->name);
         /* get to the end of the previous decl */
         if ( $1 ) {
           $$ = sym = $1;
           while (sym->next)
             sym = sym->next;
           sym->next = $2;
         }
         else
           $$ = $2;
       }
     }
   ;

statement_list
   : unlabeled_statement                { $$ = $1; }
   | label                              { $$ = $1; }
   | statement_list unlabeled_statement { $$ = newNode(NULLOP,$1,$2);}
   | statement_list label               { $$ = newNode(NULLOP,$1,$2);}
   ;

while : WHILE  {  /* create and push the continue , break & body labels */
                  static int Lblnum = 0;
                  /* continue */
                  SNPRINTF (lbuff, sizeof(lbuff), "_whilecontinue_%d",Lblnum);
                  STACK_PUSH(continueStack,newSymbol(lbuff,NestLevel));
                  /* break */
                  SNPRINTF (lbuff, sizeof(lbuff), "_whilebreak_%d",Lblnum);
                  STACK_PUSH(breakStack,newSymbol(lbuff,NestLevel));
                  /* body */
                  SNPRINTF (lbuff, sizeof(lbuff), "_whilebody_%d",Lblnum++);
                  $$ = newSymbol(lbuff,NestLevel);
               }
   ;

do : DO {  /* create and push the continue , break & body Labels */
           static int Lblnum = 0;

           /* continue */
           SNPRINTF(lbuff, sizeof(lbuff), "_docontinue_%d",Lblnum);
           STACK_PUSH(continueStack,newSymbol(lbuff,NestLevel));
           /* break */
           SNPRINTF(lbuff, sizeof(lbuff), "_dobreak_%d",Lblnum);
           STACK_PUSH(breakStack,newSymbol(lbuff,NestLevel));
           /* do body */
           SNPRINTF(lbuff, sizeof(lbuff), "_dobody_%d",Lblnum++);
           $$ = newSymbol (lbuff,NestLevel);
        }
   ;

for : FOR { /* create & push continue, break & body labels */
            static int Lblnum = 0;

           NestLevel += LEVEL_UNIT;
           STACK_PUSH(blockNum, currBlockno);
           btree_add_child(currBlockno, ++blockNo);
           currBlockno = blockNo;
           ignoreTypedefType = 0;

            /* continue */
            SNPRINTF(lbuff, sizeof(lbuff), "_forcontinue_%d",Lblnum);
            STACK_PUSH(continueStack,newSymbol(lbuff,NestLevel));
            /* break    */
            SNPRINTF(lbuff, sizeof(lbuff), "_forbreak_%d",Lblnum);
            STACK_PUSH(breakStack,newSymbol(lbuff,NestLevel));
            /* body */
            SNPRINTF(lbuff, sizeof(lbuff), "_forbody_%d",Lblnum);
            $$ = newSymbol(lbuff,NestLevel);
            /* condition */
            SNPRINTF(lbuff, sizeof(lbuff), "_forcond_%d",Lblnum++);
            STACK_PUSH(forStack,newSymbol(lbuff,NestLevel));
          }
   ;

asm_string_literal
   : STRING_LITERAL
   ;

asm_statement
   : ASM '(' asm_string_literal ')' ';'
      {
        ast *ex;

        seqPointNo++;
        ex = newNode(INLINEASM, NULL, NULL);
        ex->values.inlineasm = strdup(copyStr ($3, NULL));
        seqPointNo++;
        $$ = ex;
     }
   | INLINEASM ';'
      {
        ast *ex;

        seqPointNo++;
        ex = newNode(INLINEASM, NULL, NULL);
        ex->values.inlineasm = strdup($1);
        seqPointNo++;
        $$ = ex;
      }
   ;

addressmod
   : ADDRESSMOD identifier identifier ';' {
     symbol *sym;
     if ((sym = findSymWithLevel (AddrspaceTab, $3)) && sym->level == $3->level)
       werrorfl (sym->fileDef, sym->lineDef, E_PREVIOUS_DEF);
     if (!findSymWithLevel (SymbolTab, $2))
       werror (E_ID_UNDEF, $2->name);
     addSym (AddrspaceTab, $3, $3->name, $3->level, $3->block, 0);
     sym = findSymWithLevel (AddrspaceTab, $3);
     sym->addressmod[0] = findSymWithLevel (SymbolTab, $2);
     sym->addressmod[1] = 0;
   }
   | ADDRESSMOD identifier SD_CONST identifier ';' {
     symbol *sym;
     sym_link *type;
     if ((sym = findSymWithLevel (AddrspaceTab, $4)) && sym->level == $4->level)
       werrorfl (sym->fileDef, sym->lineDef, E_PREVIOUS_DEF);
     if (!findSymWithLevel (SymbolTab, $2))
       werror (E_ID_UNDEF, $2->name);
     addSym (AddrspaceTab, $4, $4->name, $4->level, $4->block, 0);
     sym = findSymWithLevel (AddrspaceTab, $4);
     sym->addressmod[0] = findSymWithLevel (SymbolTab, $2);
     sym->addressmod[1] = 0;
     type = newLink (SPECIFIER);
     SPEC_CONST(type) = 1;
     sym->type = sym->etype = type;
   }
   ;

identifier
   : IDENTIFIER   { $$ = newSymbol ($1, NestLevel); }
   ;
%%
