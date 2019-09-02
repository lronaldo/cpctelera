#ifndef __SDCPP_H
#define __SDCPP_H

#ifdef _WIN32
/* declaration of alloca */
#include <malloc.h>
#include <string.h>
#ifdef __BORLANDC__
#define strcasecmp  stricmp
#else
#define strcasecmp  _stricmp
#endif
#endif
#define BYTES_BIG_ENDIAN  0

/*
 * From defaults.h
 */
#ifndef GET_ENVIRONMENT
#define GET_ENVIRONMENT(VALUE, NAME) do { (VALUE) = getenv (NAME); } while (0)
#endif

/* Define results of standard character escape sequences.  */
#define TARGET_BELL     007
#define TARGET_BS       010
#define TARGET_TAB      011
#define TARGET_NEWLINE  012
#define TARGET_VT       013
#define TARGET_FF       014
#define TARGET_CR       015
#define TARGET_ESC      033

#define CHAR_TYPE_SIZE 8
#define WCHAR_TYPE_SIZE 32      /* ? maybe ? */

#define SUPPORTS_ONE_ONLY 0

#define TARGET_OBJECT_SUFFIX ".rel"

#ifndef WCHAR_UNSIGNED
#define WCHAR_UNSIGNED 0
#endif

/*
 * From langhooks.h
 */
struct diagnostic_context;

struct lang_hooks
{
  /* The first callback made to the front end, for simple
     initialization needed before any calls to handle_option.  Return
     the language mask to filter the switch array with.  */
  unsigned int (*init_options) (unsigned int argc, const char **argv);

  /* Handle the switch CODE, which has real type enum opt_code from
     options.h.  If the switch takes an argument, it is passed in ARG
     which points to permanent storage.  The handler is responsible for
     checking whether ARG is NULL, which indicates that no argument
     was in fact supplied.  For -f and -W switches, VALUE is 1 or 0
     for the positive and negative forms respectively.

     Return 1 if the switch is valid, 0 if invalid, and -1 if it's
     valid and should not be treated as language-independent too.  */
  int (*handle_option) (size_t code, const char *arg, int value);

  /* Return false to use the default complaint about a missing
     argument, otherwise output a complaint and return true.  */
  bool (*missing_argument) (const char *opt, size_t code);

  /* Called when all command line options have been parsed to allow
     further processing and initialization

     Should return true to indicate that a compiler back-end is
     not required, such as with the -E option.

     If errorcount is nonzero after this call the compiler exits
     immediately and the finish hook is not called.  */
  bool (*post_options) (const char **);

  /* Called after post_options to initialize the front end.  Return
     false to indicate that no further compilation be performed, in
     which case the finish hook is called immediately.  */
  bool (*init) (void);

  /* Called at the end of compilation, as a finalizer.  */
  void (*finish) (void);
};

/* Each front end provides its own.  */
extern const struct lang_hooks lang_hooks;

/*
 * From toplev.h
 */
/* If we haven't already defined a frontend specific diagnostics
   style, use the generic one.  */
#ifndef GCC_DIAG_STYLE
#define GCC_DIAG_STYLE __gcc_tdiag__
#endif

extern void internal_error (const char *, ...) ATTRIBUTE_PRINTF_1
     ATTRIBUTE_NORETURN;
/* Pass one of the OPT_W* from options.h as the first parameter.  */
extern void warning (int, const char *, ...) ATTRIBUTE_PRINTF_2;
extern void error (const char *, ...) ATTRIBUTE_PRINTF_1;
extern void fatal_error (const char *, ...) ATTRIBUTE_PRINTF_1
     ATTRIBUTE_NORETURN;
extern void inform (const char *, ...) ATTRIBUTE_PRINTF_1;

extern const char *progname;
extern bool exit_after_options;

extern void print_version (FILE *, const char *);

/* Handle -d switch.  */
extern void decode_d_option (const char *);

/* Functions used to get and set GCC's notion of in what directory
   compilation was started.  */

extern const char *get_src_pwd (void);
extern bool set_src_pwd (const char *);

/*
 * From flags.h
 */
/* Don't suppress warnings from system headers.  -Wsystem-headers.  */

extern bool warn_system_headers;

/* If -Werror.  */

extern bool warnings_are_errors;

/* Nonzero for -pedantic switch: warn about anything
   that standard C forbids.  */

/* Temporarily suppress certain warnings.
   This is set while reading code from a system header file.  */

extern int in_system_header;

/* Nonzero means `char' should be signed.  */

extern int flag_signed_char;

/* Nonzero means change certain warnings into errors.
   Usually these are warnings about failure to conform to some standard.  */

extern int flag_pedantic_errors;

/*
 * From options.h
 */
extern int inhibit_warnings;

/*
 * From c-common.h
 */
#include "hwint.h"
#include "cpplib.h"

/* Nonzero means don't output line number information.  */

extern char flag_no_line_commands;

/* Nonzero causes -E output not to be done, but directives such as
   #define that have side effects are still obeyed.  */

extern char flag_no_output;

/* Nonzero means dump macros in some fashion; contains the 'D', 'M' or
   'N' of the command line switch.  */

extern char flag_dump_macros;

/* 0 means we want the preprocessor to not emit line directives for
   the current working directory.  1 means we want it to do it.  -1
   means we should decide depending on whether debugging information
   is being emitted or not.  */

extern int flag_working_directory;

/* Nonzero means warn about usage of long long when `-pedantic'.  */

extern int warn_long_long;

extern int sdcpp_common_handle_option (size_t code, const char *arg, int value);
extern bool sdcpp_common_missing_argument (const char *opt, size_t code);
extern unsigned int sdcpp_common_init_options (unsigned int, const char **);
extern bool sdcpp_common_post_options (const char **);
extern bool sdcpp_common_init (void);
extern void sdcpp_common_finish (void);

/* Nonzero means pass #include lines through to the output.  */

extern char flag_dump_includes;

/* In c-ppoutput.c  */
extern void init_pp_output (FILE *);
extern void preprocess_file (cpp_reader *);
extern void pp_file_change (const struct line_map *);
extern void pp_dir_change (cpp_reader *, const char *);

/*
 * From c-pragma.h
 */
extern struct cpp_reader* parse_in;

/*
 * From input.h
 */
extern struct line_maps *line_table;

typedef source_location location_t; /* deprecated typedef */

/* Top-level source file.  */
extern const char *main_input_filename;

/*
 * From tree.h
 */
/* Define the overall contents of a tree node.
   just to make diagnostic.c happy  */

union tree_node
{
  struct tree_decl
  {
    location_t locus;
  } decl;
};

#define DECL_SOURCE_LOCATION(NODE) ((NODE)->decl.locus)

/*
 * From diagnostic.h
 */
extern int errorcount;

/*
 * From c-tree.h
 */
/* In order for the format checking to accept the C frontend
   diagnostic framework extensions, you must include this file before
   toplev.h, not after.  */
#if GCC_VERSION >= 4001
#define ATTRIBUTE_GCC_CDIAG(m, n) __attribute__ ((__format__ (GCC_DIAG_STYLE, m ,n))) ATTRIBUTE_NONNULL(m)
#else
#define ATTRIBUTE_GCC_CDIAG(m, n) ATTRIBUTE_NONNULL(m)
#endif

extern bool c_cpp_error (cpp_reader *, int, int, location_t, unsigned int,
			 const char *, va_list *)
     ATTRIBUTE_GCC_CDIAG(6,0);

#endif  /* __SDCPP_H */
