#ifndef CMDLEXCL_HEADER
#define CMDLEXCL_HEADER

int yylex();
int yyparse();
void uc_yy_set_string_to_parse(const char *cmdstr);
void uc_yy_free_string_to_parse();

#endif
