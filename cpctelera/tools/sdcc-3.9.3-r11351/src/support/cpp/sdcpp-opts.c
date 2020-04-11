/* C/ObjC/C++ command line option handling.
   Copyright (C) 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
   Contributed by Neil Booth.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "config.h"
#include "system.h"
#include "intl.h"
#include "cppdefault.h"
#include "c-incpath.h"
#include "opts.h"
#include "options.h"
#include "mkdeps.h"

#ifndef DOLLARS_IN_IDENTIFIERS
# define DOLLARS_IN_IDENTIFIERS true
#endif

#ifndef TARGET_SYSTEM_ROOT
# define TARGET_SYSTEM_ROOT NULL
#endif

/* CPP's options.  */
static cpp_options *cpp_opts;

/* Input filename.  */
static const char *this_input_filename;

/* Filename and stream for preprocessed output.  */
static const char *out_fname;
static FILE *out_stream;

/* Append dependencies to deps_file.  */
static bool deps_append;

/* If dependency switches (-MF etc.) have been given.  */
static bool deps_seen;

/* If -v seen.  */
static bool verbose;

/* Dependency output file.  */
static const char *deps_file;

/* The prefix given by -iprefix, if any.  */
static const char *iprefix;

/* The multilib directory given by -imultilib, if any.  */
static const char *imultilib;

/* The system root, if any.  Overridden by -isysroot.  */
static const char *sysroot = TARGET_SYSTEM_ROOT;

/* Zero disables all standard directories for headers.  */
static bool std_inc = true;

/* If the quote chain has been split by -I-.  */
static bool quote_chain_split;

/* If -Wunused-macros.  */
static bool warn_unused_macros;

/* If -Wvariadic-macros.  */
static bool warn_variadic_macros = true;

/* Number of deferred options.  */
static size_t deferred_count;

/* Number of deferred options scanned for -include.  */
static size_t include_cursor;

static void handle_OPT_d (const char *);
static void set_std_c89 (int, int);
static void set_std_c99 (int);
static void set_std_c11 (void);
static void check_deps_environment_vars (void);
static void handle_deferred_opts (void);
static void sanitize_cpp_opts (void);
static void add_prefixed_path (const char *, size_t);
static void push_command_line_include (void);
static void cb_file_change (cpp_reader *, const struct line_map *);
static void cb_dir_change (cpp_reader *, const char *);
static void finish_options (void);

#ifndef STDC_0_IN_SYSTEM_HEADERS
#define STDC_0_IN_SYSTEM_HEADERS 0
#endif

/* Holds switches parsed by sdcpp_common_handle_option (), but whose
   handling is deferred to sdcpp_common_post_options ().  */
static void defer_opt (enum opt_code, const char *);
static struct deferred_opt
{
  enum opt_code code;
  const char *arg;
} *deferred_opts;

/* Complain that switch CODE expects an argument but none was
   provided.  OPT was the command-line option.  Return FALSE to get
   the default message in opts.c, TRUE if we provide a specialized
   one.  */
bool
sdcpp_common_missing_argument (const char *opt, size_t code)
{
  switch (code)
    {
    default:
      /* Pick up the default message.  */
      return false;

    case OPT_A:
      error ("assertion missing after \"%s\"", opt);
      break;

    case OPT_D:
    case OPT_U:
      error ("macro name missing after \"%s\"", opt);
      break;

    case OPT_I:
    case OPT_idirafter:
    case OPT_isysroot:
    case OPT_isystem:
    case OPT_iquote:
      error ("missing path after \"%s\"", opt);
      break;

    case OPT_MF:
    case OPT_MD:
    case OPT_MMD:
    case OPT_include:
    case OPT_imacros:
    case OPT_o:
      error ("missing filename after \"%s\"", opt);
      break;

    case OPT_MQ:
    case OPT_MT:
      error ("missing makefile target after \"%s\"", opt);
      break;
    }

  return true;
}

/* Defer option CODE with argument ARG.  */
static void
defer_opt (enum opt_code code, const char *arg)
{
  deferred_opts[deferred_count].code = code;
  deferred_opts[deferred_count].arg = arg;
  deferred_count++;
}

/* Common initialization before parsing options.  */
unsigned int
sdcpp_common_init_options (unsigned int argc, const char **argv ATTRIBUTE_UNUSED)
{
  struct cpp_callbacks *cb;

  parse_in = cpp_create_reader (CLK_GNUC89, NULL, line_table);
  cb = cpp_get_callbacks (parse_in);
  cb->error = c_cpp_error;

  cpp_opts = cpp_get_options (parse_in);
  cpp_opts->dollars_in_ident = DOLLARS_IN_IDENTIFIERS;
  cpp_opts->objc = 0;

  /* Reset to avoid warnings on internal definitions.  We set it just
     before passing on command-line options to cpplib.  */
  cpp_opts->warn_dollars = 0;

  deferred_opts = XNEWVEC (struct deferred_opt, argc);

  return CL_SDCPP;
}

/* Handle switch SCODE with argument ARG.  VALUE is true, unless no-
   form of an -f or -W option was given.  Returns 0 if the switch was
   invalid, a negative number to prevent language-independent
   processing in toplev.c (a hack necessary for the short-term).  */
int
sdcpp_common_handle_option (size_t scode, const char *arg, int value)
{
  enum opt_code code = (enum opt_code) scode;
  int result = 1;

  switch (code)
    {
    default:
      result = 0;
      break;

#if 0 /* pch not supported by sdcpp */
    case OPT__output_pch_:
      pch_file = arg;
      break;
#endif

    case OPT_A:
      defer_opt (code, arg);
      break;

    case OPT_C:
      cpp_opts->discard_comments = 0;
      break;

    case OPT_CC:
      cpp_opts->discard_comments = 0;
      cpp_opts->discard_comments_in_macro_exp = 0;
      break;

    case OPT_D:
      defer_opt (code, arg);
      break;

    case OPT_H:
      cpp_opts->print_include_names = 1;
      break;

    case OPT_I:
      if (strcmp (arg, "-"))
        add_path (xstrdup (arg), BRACKET, 0, true);
      else
        {
          if (quote_chain_split)
            error ("-I- specified twice");
          quote_chain_split = true;
          split_quote_chain ();
          inform ("obsolete option -I- used, please use -iquote instead");
        }
      break;

    case OPT_M:
    case OPT_MM:
      /* When doing dependencies with -M or -MM, suppress normal
         preprocessed output, but still do -dM etc. as software
         depends on this.  Preprocessed output does occur if -MD, -MMD
         or environment var dependency generation is used.  */
      cpp_opts->deps.style = (code == OPT_M ? DEPS_SYSTEM: DEPS_USER);
      flag_no_output = 1;
      inhibit_warnings = 1;
      break;

    case OPT_MD:
    case OPT_MMD:
      cpp_opts->deps.style = (code == OPT_MD ? DEPS_SYSTEM: DEPS_USER);
      cpp_opts->deps.need_preprocessor_output = true;
      deps_file = arg;
      break;

    case OPT_MF:
      deps_seen = true;
      deps_file = arg;
      break;

    case OPT_MG:
      deps_seen = true;
      cpp_opts->deps.missing_files = true;
      break;

    case OPT_MP:
      deps_seen = true;
      cpp_opts->deps.phony_targets = true;
      break;

    case OPT_MQ:
    case OPT_MT:
      deps_seen = true;
      defer_opt (code, arg);
      break;

    case OPT_P:
      flag_no_line_commands = 1;
      break;

    case OPT_fworking_directory:
      flag_working_directory = value;
      break;

    case OPT_U:
      defer_opt (code, arg);
      break;

    case OPT_Wall:
      cpp_opts->warn_trigraphs = value;
      cpp_opts->warn_comments = value;
      cpp_opts->warn_num_sign_change = value;
      cpp_opts->warn_multichar = value; /* Was C++ only.  */
      break;

    case OPT_Wcomment:
    case OPT_Wcomments:
      cpp_opts->warn_comments = value;
      break;

    case OPT_Wdeprecated:
      cpp_opts->cpp_warn_deprecated = value;
      break;

    case OPT_Wendif_labels:
      cpp_opts->warn_endif_labels = value;
      break;

    case OPT_Wimport:
      /* Silently ignore for now.  */
      break;

#if 0 // pch not supported by sdcpp
    case OPT_Winvalid_pch:
      cpp_opts->warn_invalid_pch = value;
      break;
#endif

    case OPT_Wtraditional:
      cpp_opts->cpp_warn_traditional = value;
      break;

    case OPT_Wtrigraphs:
      cpp_opts->warn_trigraphs = value;
      break;

    case OPT_Wundef:
      cpp_opts->warn_undef = value;
      break;

    case OPT_Wunused_macros:
      warn_unused_macros = value;
      break;

    case OPT_Wvariadic_macros:
      warn_variadic_macros = value;
      break;

    case OPT_ansi:
      set_std_c89 (false, true);
      break;

    case OPT_d:
      handle_OPT_d (arg);
      break;

    case OPT_fdollars_in_identifiers:
      cpp_opts->dollars_in_ident = value;
      break;

    case OPT_fsigned_char:
      flag_signed_char = value;
      break;

    case OPT_funsigned_char:
      flag_signed_char = !value;
      break;

#if 0 // pch not supported by sdcpp
    case OPT_fpch_deps:
      cpp_opts->restore_pch_deps = value;
      break;

    case OPT_fpch_preprocess:
      flag_pch_preprocess = value;
      break;
#endif

    case OPT_fpreprocessed:
      cpp_opts->preprocessed = value;
      break;

    case OPT_ftabstop_:
      /* It is documented that we silently ignore silly values.  */
      if (value >= 1 && value <= 100)
        cpp_opts->tabstop = value;
      break;

    case OPT_fexec_charset_:
      cpp_opts->narrow_charset = arg;
      break;

    case OPT_fwide_exec_charset_:
      cpp_opts->wide_charset = arg;
      break;

    case OPT_finput_charset_:
      cpp_opts->input_charset = arg;
      break;

    case OPT_idirafter:
      add_path (xstrdup (arg), AFTER, 0, true);
      break;

    case OPT_imacros:
    case OPT_include:
      defer_opt (code, arg);
      break;

    case OPT_iprefix:
      iprefix = arg;
      break;

    case OPT_imultilib:
      imultilib = arg;
      break;

    case OPT_iquote:
      add_path (xstrdup (arg), QUOTE, 0, true);
      break;

    case OPT_isysroot:
      sysroot = arg;
      break;

    case OPT_isystem:
      add_path (xstrdup (arg), SYSTEM, 0, true);
      break;

    case OPT_iwithprefix:
      add_prefixed_path (arg, SYSTEM);
      break;

    case OPT_iwithprefixbefore:
      add_prefixed_path (arg, BRACKET);
      break;

    case OPT_lang_asm:
      cpp_set_lang (parse_in, CLK_ASM);
      cpp_opts->dollars_in_ident = false;
      break;

    case OPT_lang_objc:
      cpp_opts->objc = 1;
      break;

    case OPT_nostdinc:
      std_inc = false;
      break;

    case OPT_o:
      if (!out_fname)
        out_fname = arg;
      else
        error ("output filename specified several times");
      break;

      /* SDCPP specfic */
    case OPT_obj_ext_:
      cpp_opts->obj_ext = arg;
      break;

      /* We need to handle the -pedantic switches here, rather than in
         sdcpp_common_post_options, so that a subsequent -Wno-endif-labels
         is not overridden.  */
    case OPT_pedantic_errors:
    case OPT_pedantic:
      cpp_opts->cpp_pedantic = 1;
      cpp_opts->warn_endif_labels = 1;
      break;

      /* SDCPP specfic */
    case OPT_pedantic_parse_number:
      cpp_opts->pedantic_parse_number = 1;
      break;

#if 0 // pch not supported by sdcpp
    case OPT_print_pch_checksum:
      c_common_print_pch_checksum (stdout);
      exit_after_options = true;
      break;
#endif

    case OPT_remap:
      cpp_opts->remap = 1;
      break;

    case OPT_std_c89:
    case OPT_std_iso9899_1990:
    case OPT_std_iso9899_199409:
      set_std_c89 (code == OPT_std_iso9899_199409 /* c94 */, true /* ISO */);
      break;

    case OPT_std_c99:
    case OPT_std_iso9899_1999:
      set_std_c99 (true /* ISO */);
      break;

    case OPT_std_c11:
      set_std_c11 ();
      break;

    case OPT_no_trigraphs:
      /* trigraphs enabled by default on sdcpp, -no-trgraphs disables them */
      cpp_opts->trigraphs = 0;
      break;

    case OPT_traditional_cpp:
      cpp_opts->traditional = 1;
      break;

    case OPT_v:
      verbose = true;
      break;
    }

  return result;
}

/* Post-switch processing.  */
bool
sdcpp_common_post_options (const char **pfilename)
{
  struct cpp_callbacks *cb;

  /* Canonicalize the input and output filenames.  */
  if (in_fnames == NULL)
    {
      in_fnames = XNEWVEC (const char *, 1);
      in_fnames[0] = "";
    }
  else if (strcmp (in_fnames[0], "-") == 0)
    in_fnames[0] = "";

  if (num_in_fnames > 1)
    {
      if (!out_fname)
        out_fname = in_fnames[1];
      else
        error ("output filename specified several times");
    }

  if (out_fname == NULL || !strcmp (out_fname, "-"))
    out_fname = "";

  if (cpp_opts->deps.style == DEPS_NONE)
    check_deps_environment_vars ();

  handle_deferred_opts ();

  sanitize_cpp_opts ();

  register_include_chains (parse_in, sysroot, iprefix, imultilib,
                           std_inc, 0, verbose);

  /* Open the output now.  We must do so even if flag_no_output is
     on, because there may be other output than from the actual
     preprocessing (e.g. from -dM).  */
  if (out_fname[0] == '\0')
    out_stream = stdout;
  else
    out_stream = fopen (out_fname, "w");

  if (out_stream == NULL)
    {
      fatal_error ("opening output file %s: %s", out_fname, strerror(errno));
      return false;
    }

  if (num_in_fnames > 2)
    error ("too many filenames given.  Type %s --help for usage",
           progname);

  init_pp_output (out_stream);

  cb = cpp_get_callbacks (parse_in);
  cb->file_change = cb_file_change;
  cb->dir_change = cb_dir_change;
  cpp_post_options (parse_in);

  *pfilename = this_input_filename
    = cpp_read_main_file (parse_in, in_fnames[0]);
  /* Don't do any compilation or preprocessing if there is no input file.  */
  if (this_input_filename == NULL)
    {
      errorcount++;
      return false;
    }

  if (flag_working_directory && !flag_no_line_commands)
    pp_dir_change (parse_in, get_src_pwd ());

  return 1;
}

/* Front end initialization. */
bool
sdcpp_common_init (void)
{
  /* Default CPP arithmetic to something sensible for the host for the
     benefit of dumb users like fix-header.  */
  cpp_opts->precision = CHAR_BIT * sizeof (long);
  cpp_opts->char_precision = CHAR_BIT;
  cpp_opts->int_precision = CHAR_BIT * sizeof (int);
  cpp_opts->wchar_precision = CHAR_BIT * sizeof (int);
  cpp_opts->unsigned_wchar = 1;
  cpp_opts->bytes_big_endian = BYTES_BIG_ENDIAN;

  /* This can't happen until after wchar_precision and bytes_big_endian
     are known.  */
  cpp_init_iconv (parse_in);

#if 0
  if (version_flag)
    c_common_print_pch_checksum (stderr);
#endif

  finish_options ();
  preprocess_file (parse_in);
  return true;
}

/* Common finish hook for the C, ObjC and C++ front ends.  */
void
sdcpp_common_finish (void)
{
  FILE *deps_stream = NULL;

  if (cpp_opts->deps.style != DEPS_NONE)
    {
      /* If -M or -MM was seen without -MF, default output to the
         output stream.  */
      if (!deps_file)
        deps_stream = out_stream;
      else
        {
          deps_stream = fopen (deps_file, deps_append ? "a": "w");
          if (!deps_stream)
            fatal_error ("opening dependency file %s: %s", deps_file, strerror(errno));
        }
    }

  /* For performance, avoid tearing down cpplib's internal structures
     with cpp_destroy ().  */
  cpp_finish (parse_in, deps_stream);

  if (deps_stream && deps_stream != out_stream
      && (ferror (deps_stream) || fclose (deps_stream)))
    fatal_error ("closing dependency file %s: %s", deps_file, strerror(errno));

  if (out_stream && (ferror (out_stream) || fclose (out_stream)))
    fatal_error ("when writing output to %s: %s", out_fname, strerror(errno));
}

/* Either of two environment variables can specify output of
   dependencies.  Their value is either "OUTPUT_FILE" or "OUTPUT_FILE
   DEPS_TARGET", where OUTPUT_FILE is the file to write deps info to
   and DEPS_TARGET is the target to mention in the deps.  They also
   result in dependency information being appended to the output file
   rather than overwriting it, and like Sun's compiler
   SUNPRO_DEPENDENCIES suppresses the dependency on the main file.  */
static void
check_deps_environment_vars (void)
{
  char *spec;

  GET_ENVIRONMENT (spec, "DEPENDENCIES_OUTPUT");
  if (spec)
    cpp_opts->deps.style = DEPS_USER;
  else
    {
      GET_ENVIRONMENT (spec, "SUNPRO_DEPENDENCIES");
      if (spec)
        {
          cpp_opts->deps.style = DEPS_SYSTEM;
          cpp_opts->deps.ignore_main_file = true;
        }
    }

  if (spec)
    {
      /* Find the space before the DEPS_TARGET, if there is one.  */
      char *s = strchr (spec, ' ');
      if (s)
        {
          /* Let the caller perform MAKE quoting.  */
          defer_opt (OPT_MT, s + 1);
          *s = '\0';
        }

      /* Command line -MF overrides environment variables and default.  */
      if (!deps_file)
        deps_file = spec;

      deps_append = 1;
      deps_seen = true;
    }
}

/* Handle deferred command line switches.  */
static void
handle_deferred_opts (void)
{
  size_t i;
  struct deps *deps;

  /* Avoid allocating the deps buffer if we don't need it.
     (This flag may be true without there having been -MT or -MQ
     options, but we'll still need the deps buffer.)  */
  if (!deps_seen)
    return;

  deps = cpp_get_deps (parse_in);

  for (i = 0; i < deferred_count; i++)
    {
      struct deferred_opt *opt = &deferred_opts[i];

      if (opt->code == OPT_MT || opt->code == OPT_MQ)
        deps_add_target (deps, opt->arg, opt->code == OPT_MQ);
    }
}

/* These settings are appropriate for GCC, but not necessarily so for
   cpplib as a library.  */
static void
sanitize_cpp_opts (void)
{
  /* If we don't know what style of dependencies to output, complain
     if any other dependency switches have been given.  */
  if (deps_seen && cpp_opts->deps.style == DEPS_NONE)
    error ("to generate dependencies you must specify either -M or -MM");

  /* -dM and dependencies suppress normal output; do it here so that
     the last -d[MDN] switch overrides earlier ones.  */
  if (flag_dump_macros == 'M')
    flag_no_output = 1;

  /* Disable -dD, -dN and -dI if normal output is suppressed.  Allow
     -dM since at least glibc relies on -M -dM to work.  */
  /* Also, flag_no_output implies flag_no_line_commands, always. */
  if (flag_no_output)
    {
      if (flag_dump_macros != 'M')
        flag_dump_macros = 0;
      flag_dump_includes = 0;
      flag_no_line_commands = 1;
    }

  cpp_opts->unsigned_char = !flag_signed_char;
  cpp_opts->stdc_0_in_system_headers = STDC_0_IN_SYSTEM_HEADERS;

  /* Similarly with -Wno-variadic-macros.  No check for c99 here, since
     this also turns off warnings about GCCs extension.  */
  cpp_opts->warn_variadic_macros
    = warn_variadic_macros && (pedantic || warn_traditional);

  /* If we're generating preprocessor output, emit current directory
     if explicitly requested  */
  if (flag_working_directory == -1)
    flag_working_directory = 0;
}

/* Add include path with a prefix at the front of its name.  */
static void
add_prefixed_path (const char *suffix, size_t chain)
{
  char *path;
  const char *prefix;
  size_t prefix_len, suffix_len;

  suffix_len = strlen (suffix);
  prefix     = iprefix ? iprefix : cpp_GCC_INCLUDE_DIR;
  prefix_len = iprefix ? strlen (iprefix) : cpp_GCC_INCLUDE_DIR_len;

  path = (char *) xmalloc (prefix_len + suffix_len + 1);
  memcpy (path, prefix, prefix_len);
  memcpy (path + prefix_len, suffix, suffix_len);
  path[prefix_len + suffix_len] = '\0';

  add_path (path, chain, 0, false);
}

/* Handle -D, -U, -A, -imacros, and the first -include.  */
static void
finish_options (void)
{
  if (!cpp_opts->preprocessed)
    {
      size_t i;

      cb_file_change (parse_in,
                      linemap_add (line_table, LC_RENAME, 0,
                                   _("<built-in>"), 0));

      cpp_init_builtins (parse_in, 0 /*flag_hosted*/);

      /* We're about to send user input to cpplib, so make it warn for
         things that we previously (when we sent it internal definitions)
         told it to not warn.

         C99 permits implementation-defined characters in identifiers.
         The documented meaning of -std= is to turn off extensions that
         conflict with the specified standard, and since a strictly
         conforming program cannot contain a '$', we do not condition
         their acceptance on the -std= setting.  */
      cpp_opts->warn_dollars = (cpp_opts->cpp_pedantic && !cpp_opts->c99);

      cpp_change_file (parse_in, LC_RENAME, _("<command line>"));
      for (i = 0; i < deferred_count; i++)
        {
          struct deferred_opt *opt = &deferred_opts[i];

          if (opt->code == OPT_D)
            cpp_define (parse_in, opt->arg);
          else if (opt->code == OPT_U)
            cpp_undef (parse_in, opt->arg);
          else if (opt->code == OPT_A)
            {
              if (opt->arg[0] == '-')
                cpp_unassert (parse_in, opt->arg + 1);
              else
                cpp_assert (parse_in, opt->arg);
            }
        }

      /* Handle -imacros after -D and -U.  */
      for (i = 0; i < deferred_count; i++)
        {
          struct deferred_opt *opt = &deferred_opts[i];

          if (opt->code == OPT_imacros
              && cpp_push_include (parse_in, opt->arg))
            {
              /* Disable push_command_line_include callback for now.  */
              include_cursor = deferred_count + 1;
              cpp_scan_nooutput (parse_in);
            }
        }
    }

  include_cursor = 0;
  push_command_line_include ();
}

/* Give CPP the next file given by -include, if any.  */
static void
push_command_line_include (void)
{
  while (include_cursor < deferred_count)
    {
      struct deferred_opt *opt = &deferred_opts[include_cursor++];

      if (!cpp_opts->preprocessed && opt->code == OPT_include
          && cpp_push_include (parse_in, opt->arg))
        return;
    }

  if (include_cursor == deferred_count)
    {
      include_cursor++;
      /* -Wunused-macros should only warn about macros defined hereafter.  */
      cpp_opts->warn_unused_macros = warn_unused_macros;
      /* Restore the line map from <command line>.  */
      if (!cpp_opts->preprocessed)
        cpp_change_file (parse_in, LC_RENAME, this_input_filename);

      /* Set this here so the client can change the option if it wishes,
         and after stacking the main file so we don't trace the main file.  */
      line_table->trace_includes = cpp_opts->print_include_names;
    }
}

/* File change callback.  Has to handle -include files.  */
static void
cb_file_change (cpp_reader * ARG_UNUSED (pfile),
                const struct line_map *new_map)
{
  pp_file_change (new_map);

  if (new_map == 0 || (new_map->reason == LC_LEAVE && MAIN_FILE_P (new_map)))
    push_command_line_include ();
}

void
cb_dir_change (cpp_reader * ARG_UNUSED (pfile), const char *dir)
{
  if (!set_src_pwd (dir))
    warning (0, "too late for # directive to set debug directory");
}

/* Set the C 89 standard (with 1994 amendments if C94, without GNU
   extensions if ISO).  There is no concept of gnu94.  */
static void
set_std_c89 (int c94, int iso)
{
  cpp_set_lang (parse_in, c94 ? CLK_STDC94: iso ? CLK_STDC89: CLK_GNUC89);
}

/* Set the C 99 standard (without GNU extensions if ISO).  */
static void
set_std_c99 (int iso)
{
  cpp_set_lang (parse_in, iso ? CLK_STDC99: CLK_GNUC99);
}

/* Set the C 11 standard (without GNU extensions).  */
static void
set_std_c11 (void)
{
  cpp_set_lang (parse_in, CLK_STDC1X);
}

/* Args to -d specify what to dump.  Silently ignore
   unrecognized options; they may be aimed at toplev.c.  */
static void
handle_OPT_d (const char *arg)
{
  char c;

  while ((c = *arg++) != '\0')
    switch (c)
      {
      case 'M':                 /* Dump macros only.  */
      case 'N':                 /* Dump names.  */
      case 'D':                 /* Dump definitions.  */
        flag_dump_macros = c;
        break;

      case 'I':
        flag_dump_includes = 1;
        break;
      }
}
