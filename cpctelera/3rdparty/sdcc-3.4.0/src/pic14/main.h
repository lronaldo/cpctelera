#ifndef MAIN_INCLUDE
#define MAIN_INCLUDE

typedef struct {
  unsigned int isLibrarySource:1;
  int disable_df;
  int no_ext_instr;
  int no_warn_non_free;
} pic14_options_t;

extern pic14_options_t pic14_options;
extern int debug_verbose;

#endif

