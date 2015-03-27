/* Generic target-file-type support for the BFD library.
   Copyright 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
   2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
   Free Software Foundation, Inc.
   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "fnmatch.h"

/*
   It's okay to see some:
#if 0
   directives in this source file, as targets.c uses them to exclude
   certain BFD vectors.  This comment is specially formatted to catch
   users who grep for ^#if 0, so please keep it this way!
*/

/*
SECTION
	Targets

DESCRIPTION
	Each port of BFD to a different machine requires the creation
	of a target back end. All the back end provides to the root
	part of BFD is a structure containing pointers to functions
	which perform certain low level operations on files. BFD
	translates the applications's requests through a pointer into
	calls to the back end routines.

	When a file is opened with <<bfd_openr>>, its format and
	target are unknown. BFD uses various mechanisms to determine
	how to interpret the file. The operations performed are:

	o Create a BFD by calling the internal routine
	<<_bfd_new_bfd>>, then call <<bfd_find_target>> with the
	target string supplied to <<bfd_openr>> and the new BFD pointer.

	o If a null target string was provided to <<bfd_find_target>>,
	look up the environment variable <<GNUTARGET>> and use
	that as the target string.

	o If the target string is still <<NULL>>, or the target string is
	<<default>>, then use the first item in the target vector
	as the target type, and set <<target_defaulted>> in the BFD to
	cause <<bfd_check_format>> to loop through all the targets.
	@xref{bfd_target}.  @xref{Formats}.

	o Otherwise, inspect the elements in the target vector
	one by one, until a match on target name is found. When found,
	use it.

	o Otherwise return the error <<bfd_error_invalid_target>> to
	<<bfd_openr>>.

	o <<bfd_openr>> attempts to open the file using
	<<bfd_open_file>>, and returns the BFD.

	Once the BFD has been opened and the target selected, the file
	format may be determined. This is done by calling
	<<bfd_check_format>> on the BFD with a suggested format.
	If <<target_defaulted>> has been set, each possible target
	type is tried to see if it recognizes the specified format.
	<<bfd_check_format>> returns <<TRUE>> when the caller guesses right.
@menu
@* bfd_target::
@end menu
*/

/*

INODE
	bfd_target,  , Targets, Targets
DOCDD
SUBSECTION
	bfd_target

DESCRIPTION
	This structure contains everything that BFD knows about a
	target. It includes things like its byte order, name, and which
	routines to call to do various operations.

	Every BFD points to a target structure with its <<xvec>>
	member.

	The macros below are used to dispatch to functions through the
	<<bfd_target>> vector. They are used in a number of macros further
	down in @file{bfd.h}, and are also used when calling various
	routines by hand inside the BFD implementation.  The @var{arglist}
	argument must be parenthesized; it contains all the arguments
	to the called function.

	They make the documentation (more) unpleasant to read, so if
	someone wants to fix this and not break the above, please do.

.#define BFD_SEND(bfd, message, arglist) \
.  ((*((bfd)->xvec->message)) arglist)
.
.#ifdef DEBUG_BFD_SEND
.#undef BFD_SEND
.#define BFD_SEND(bfd, message, arglist) \
.  (((bfd) && (bfd)->xvec && (bfd)->xvec->message) ? \
.    ((*((bfd)->xvec->message)) arglist) : \
.    (bfd_assert (__FILE__,__LINE__), NULL))
.#endif

	For operations which index on the BFD format:

.#define BFD_SEND_FMT(bfd, message, arglist) \
.  (((bfd)->xvec->message[(int) ((bfd)->format)]) arglist)
.
.#ifdef DEBUG_BFD_SEND
.#undef BFD_SEND_FMT
.#define BFD_SEND_FMT(bfd, message, arglist) \
.  (((bfd) && (bfd)->xvec && (bfd)->xvec->message) ? \
.   (((bfd)->xvec->message[(int) ((bfd)->format)]) arglist) : \
.   (bfd_assert (__FILE__,__LINE__), NULL))
.#endif
.
	This is the structure which defines the type of BFD this is.  The
	<<xvec>> member of the struct <<bfd>> itself points here.  Each
	module that implements access to a different target under BFD,
	defines one of these.

	FIXME, these names should be rationalised with the names of
	the entry points which call them. Too bad we can't have one
	macro to define them both!

.enum bfd_flavour
.{
.  bfd_target_unknown_flavour,
.  bfd_target_srec_flavour,
.  bfd_target_ihex_flavour,
.  bfd_target_asxxxx_flavour
.};
.
.enum bfd_endian { BFD_ENDIAN_BIG, BFD_ENDIAN_LITTLE, BFD_ENDIAN_UNKNOWN };
.
.{* Forward declaration.  *}
.typedef struct bfd_link_info _bfd_link_info;
.
.{* Forward declaration.  *}
.typedef struct flag_info flag_info;
.
.typedef struct bfd_target
.{
.  {* Identifies the kind of target, e.g., SunOS4, Ultrix, etc.  *}
.  char *name;
.
. {* The "flavour" of a back end is a general indication about
.    the contents of a file.  *}
.  enum bfd_flavour flavour;
.
.  {* The order of bytes within the data area of a file.  *}
.  enum bfd_endian byteorder;
.
. {* The order of bytes within the header parts of a file.  *}
.  enum bfd_endian header_byteorder;
.
.  {* A mask of all the flags which an executable may have set -
.     from the set <<BFD_NO_FLAGS>>, <<HAS_RELOC>>, ...<<D_PAGED>>.  *}
.  flagword object_flags;
.
. {* A mask of all the flags which a section may have set - from
.    the set <<SEC_NO_FLAGS>>, <<SEC_ALLOC>>, ...<<SET_NEVER_LOAD>>.  *}
.  flagword section_flags;
.
. {* The character normally found at the front of a symbol.
.    (if any), perhaps `_'.  *}
.  char symbol_leading_char;
.
. {* The pad character for file names within an archive header.  *}
.  char ar_pad_char;
.
.  {* The maximum number of characters in an archive header.  *}
.  unsigned char ar_max_namelen;
.
.  {* How well this target matches, used to select between various
.     possible targets when more than one target matches.  *}
.  unsigned char match_priority;
.
.  {* Entries for byte swapping for data. These are different from the
.     other entry points, since they don't take a BFD as the first argument.
.     Certain other handlers could do the same.  *}
.  bfd_uint64_t   (*bfd_getx64) (const void *);
.  bfd_int64_t    (*bfd_getx_signed_64) (const void *);
.  void           (*bfd_putx64) (bfd_uint64_t, void *);
.  bfd_vma        (*bfd_getx32) (const void *);
.  bfd_signed_vma (*bfd_getx_signed_32) (const void *);
.  void           (*bfd_putx32) (bfd_vma, void *);
.  bfd_vma        (*bfd_getx16) (const void *);
.  bfd_signed_vma (*bfd_getx_signed_16) (const void *);
.  void           (*bfd_putx16) (bfd_vma, void *);
.
.  {* Byte swapping for the headers.  *}
.  bfd_uint64_t   (*bfd_h_getx64) (const void *);
.  bfd_int64_t    (*bfd_h_getx_signed_64) (const void *);
.  void           (*bfd_h_putx64) (bfd_uint64_t, void *);
.  bfd_vma        (*bfd_h_getx32) (const void *);
.  bfd_signed_vma (*bfd_h_getx_signed_32) (const void *);
.  void           (*bfd_h_putx32) (bfd_vma, void *);
.  bfd_vma        (*bfd_h_getx16) (const void *);
.  bfd_signed_vma (*bfd_h_getx_signed_16) (const void *);
.  void           (*bfd_h_putx16) (bfd_vma, void *);
.
.  {* Format dependent routines: these are vectors of entry points
.     within the target vector structure, one for each format to check.  *}
.
.  {* Check the format of a file being read.  Return a <<bfd_target *>> or zero.  *}
.  const struct bfd_target *(*_bfd_check_format[bfd_type_end]) (bfd *);
.
.  {* Set the format of a file being written.  *}
.  bfd_boolean (*_bfd_set_format[bfd_type_end]) (bfd *);
.
.  {* Write cached information into a file being written, at <<bfd_close>>.  *}
.  bfd_boolean (*_bfd_write_contents[bfd_type_end]) (bfd *);
.
The general target vector.  These vectors are initialized using the
BFD_JUMP_TABLE macros.
.
.  {* Generic entry points.  *}
.#define BFD_JUMP_TABLE_GENERIC(NAME) \
.  NAME##_close_and_cleanup, \
.  NAME##_bfd_free_cached_info, \
.  NAME##_new_section_hook, \
.  NAME##_get_section_contents, \
.  NAME##_get_section_contents_in_window
.
.  {* Called when the BFD is being closed to do any necessary cleanup.  *}
.  bfd_boolean (*_close_and_cleanup) (bfd *);
.  {* Ask the BFD to free all cached information.  *}
.  bfd_boolean (*_bfd_free_cached_info) (bfd *);
.  {* Called when a new section is created.  *}
.  bfd_boolean (*_new_section_hook) (bfd *, sec_ptr);
.  {* Read the contents of a section.  *}
.  bfd_boolean (*_bfd_get_section_contents)
.    (bfd *, sec_ptr, void *, file_ptr, bfd_size_type);
.  bfd_boolean (*_bfd_get_section_contents_in_window)
.    (bfd *, sec_ptr, bfd_window *, file_ptr, bfd_size_type);
.
.  {* Entry points to copy private data.  *}
.#define BFD_JUMP_TABLE_COPY(NAME) \
.  NAME##_bfd_copy_private_bfd_data, \
.  NAME##_bfd_merge_private_bfd_data, \
.  _bfd_generic_init_private_section_data, \
.  NAME##_bfd_copy_private_section_data, \
.  NAME##_bfd_copy_private_symbol_data, \
.  NAME##_bfd_copy_private_header_data, \
.  NAME##_bfd_set_private_flags, \
.  NAME##_bfd_print_private_bfd_data
.
.  {* Called to copy BFD general private data from one object file
.     to another.  *}
.  bfd_boolean (*_bfd_copy_private_bfd_data) (bfd *, bfd *);
.  {* Called to merge BFD general private data from one object file
.     to a common output file when linking.  *}
.  bfd_boolean (*_bfd_merge_private_bfd_data) (bfd *, bfd *);
.  {* Called to initialize BFD private section data from one object file
.     to another.  *}
.#define bfd_init_private_section_data(ibfd, isec, obfd, osec, link_info) \
.  BFD_SEND (obfd, _bfd_init_private_section_data, (ibfd, isec, obfd, osec, link_info))
.  bfd_boolean (*_bfd_init_private_section_data)
.    (bfd *, sec_ptr, bfd *, sec_ptr, struct bfd_link_info *);
.  {* Called to copy BFD private section data from one object file
.     to another.  *}
.  bfd_boolean (*_bfd_copy_private_section_data)
.    (bfd *, sec_ptr, bfd *, sec_ptr);
.  {* Called to copy BFD private symbol data from one symbol
.     to another.  *}
.  bfd_boolean (*_bfd_copy_private_symbol_data)
.    (bfd *, asymbol *, bfd *, asymbol *);
.  {* Called to copy BFD private header data from one object file
.     to another.  *}
.  bfd_boolean (*_bfd_copy_private_header_data)
.    (bfd *, bfd *);
.  {* Called to set private backend flags.  *}
.  bfd_boolean (*_bfd_set_private_flags) (bfd *, flagword);
.
.  {* Called to print private BFD data.  *}
.  bfd_boolean (*_bfd_print_private_bfd_data) (bfd *, void *);
.
.  {* Core file entry points.  *}
.#define BFD_JUMP_TABLE_CORE(NAME) \
.  NAME##_core_file_failing_command, \
.  NAME##_core_file_failing_signal, \
.  NAME##_core_file_matches_executable_p, \
.  NAME##_core_file_pid
.
.  char *      (*_core_file_failing_command) (bfd *);
.  int         (*_core_file_failing_signal) (bfd *);
.  bfd_boolean (*_core_file_matches_executable_p) (bfd *, bfd *);
.  int         (*_core_file_pid) (bfd *);
.
.  {* Archive entry points.  *}
.#define BFD_JUMP_TABLE_ARCHIVE(NAME) \
.  NAME##_slurp_armap, \
.  NAME##_slurp_extended_name_table, \
.  NAME##_construct_extended_name_table, \
.  NAME##_truncate_arname, \
.  NAME##_write_armap, \
.  NAME##_read_ar_hdr, \
.  NAME##_write_ar_hdr, \
.  NAME##_openr_next_archived_file, \
.  NAME##_get_elt_at_index, \
.  NAME##_generic_stat_arch_elt, \
.  NAME##_update_armap_timestamp
.
.  bfd_boolean (*_bfd_slurp_armap) (bfd *);
.  bfd_boolean (*_bfd_slurp_extended_name_table) (bfd *);
.  bfd_boolean (*_bfd_construct_extended_name_table)
.    (bfd *, char **, bfd_size_type *, const char **);
.  void        (*_bfd_truncate_arname) (bfd *, const char *, char *);
.  bfd_boolean (*write_armap)
.    (bfd *, unsigned int, struct orl *, unsigned int, int);
.  void *      (*_bfd_read_ar_hdr_fn) (bfd *);
.  bfd_boolean (*_bfd_write_ar_hdr_fn) (bfd *, bfd *);
.  bfd *       (*openr_next_archived_file) (bfd *, bfd *);
.#define bfd_get_elt_at_index(b,i) BFD_SEND (b, _bfd_get_elt_at_index, (b,i))
.  bfd *       (*_bfd_get_elt_at_index) (bfd *, symindex);
.  int         (*_bfd_stat_arch_elt) (bfd *, struct stat *);
.  bfd_boolean (*_bfd_update_armap_timestamp) (bfd *);
.
.  {* Entry points used for symbols.  *}
.#define BFD_JUMP_TABLE_SYMBOLS(NAME) \
.  NAME##_get_symtab_upper_bound, \
.  NAME##_canonicalize_symtab, \
.  NAME##_make_empty_symbol, \
.  NAME##_print_symbol, \
.  NAME##_get_symbol_info, \
.  NAME##_bfd_is_local_label_name, \
.  NAME##_bfd_is_target_special_symbol, \
.  NAME##_get_lineno, \
.  NAME##_find_nearest_line, \
.  _bfd_generic_find_nearest_line_discriminator, \
.  _bfd_generic_find_line, \
.  NAME##_find_inliner_info, \
.  NAME##_bfd_make_debug_symbol, \
.  NAME##_read_minisymbols, \
.  NAME##_minisymbol_to_symbol
.
.  long        (*_bfd_get_symtab_upper_bound) (bfd *);
.  long        (*_bfd_canonicalize_symtab)
.    (bfd *, struct bfd_symbol **);
.  struct bfd_symbol *
.              (*_bfd_make_empty_symbol) (bfd *);
.  void        (*_bfd_print_symbol)
.    (bfd *, void *, struct bfd_symbol *, bfd_print_symbol_type);
.#define bfd_print_symbol(b,p,s,e) BFD_SEND (b, _bfd_print_symbol, (b,p,s,e))
.  void        (*_bfd_get_symbol_info)
.    (bfd *, struct bfd_symbol *, symbol_info *);
.#define bfd_get_symbol_info(b,p,e) BFD_SEND (b, _bfd_get_symbol_info, (b,p,e))
.  bfd_boolean (*_bfd_is_local_label_name) (bfd *, const char *);
.  bfd_boolean (*_bfd_is_target_special_symbol) (bfd *, asymbol *);
.  alent *     (*_get_lineno) (bfd *, struct bfd_symbol *);
.  bfd_boolean (*_bfd_find_nearest_line)
.    (bfd *, struct bfd_section *, struct bfd_symbol **, bfd_vma,
.     const char **, const char **, unsigned int *);
.  bfd_boolean (*_bfd_find_nearest_line_discriminator)
.    (bfd *, struct bfd_section *, struct bfd_symbol **, bfd_vma,
.     const char **, const char **, unsigned int *, unsigned int *);
.  bfd_boolean (*_bfd_find_line)
.    (bfd *, struct bfd_symbol **, struct bfd_symbol *,
.     const char **, unsigned int *);
.  bfd_boolean (*_bfd_find_inliner_info)
.    (bfd *, const char **, const char **, unsigned int *);
. {* Back-door to allow format-aware applications to create debug symbols
.    while using BFD for everything else.  Currently used by the assembler
.    when creating COFF files.  *}
.  asymbol *   (*_bfd_make_debug_symbol)
.    (bfd *, void *, unsigned long size);
.#define bfd_read_minisymbols(b, d, m, s) \
.  BFD_SEND (b, _read_minisymbols, (b, d, m, s))
.  long        (*_read_minisymbols)
.    (bfd *, bfd_boolean, void **, unsigned int *);
.#define bfd_minisymbol_to_symbol(b, d, m, f) \
.  BFD_SEND (b, _minisymbol_to_symbol, (b, d, m, f))
.  asymbol *   (*_minisymbol_to_symbol)
.    (bfd *, bfd_boolean, const void *, asymbol *);
.
.  {* Routines for relocs.  *}
.#define BFD_JUMP_TABLE_RELOCS(NAME) \
.  NAME##_get_reloc_upper_bound, \
.  NAME##_canonicalize_reloc, \
.  NAME##_bfd_reloc_type_lookup, \
.  NAME##_bfd_reloc_name_lookup
.
.  long        (*_get_reloc_upper_bound) (bfd *, sec_ptr);
.  long        (*_bfd_canonicalize_reloc)
.    (bfd *, sec_ptr, arelent **, struct bfd_symbol **);
.  {* See documentation on reloc types.  *}
.  reloc_howto_type *
.              (*reloc_type_lookup) (bfd *, bfd_reloc_code_real_type);
.  reloc_howto_type *
.              (*reloc_name_lookup) (bfd *, const char *);
.
.
.  {* Routines used when writing an object file.  *}
.#define BFD_JUMP_TABLE_WRITE(NAME) \
.  NAME##_set_arch_mach, \
.  NAME##_set_section_contents
.
.  bfd_boolean (*_bfd_set_arch_mach)
.    (bfd *, enum bfd_architecture, unsigned long);
.  bfd_boolean (*_bfd_set_section_contents)
.    (bfd *, sec_ptr, const void *, file_ptr, bfd_size_type);
.
.  {* Routines used by the linker.  *}
.#define BFD_JUMP_TABLE_LINK(NAME) \
.  NAME##_sizeof_headers, \
.  NAME##_bfd_get_relocated_section_contents, \
.  NAME##_bfd_relax_section, \
.  NAME##_bfd_link_hash_table_create, \
.  NAME##_bfd_link_hash_table_free, \
.  NAME##_bfd_link_add_symbols, \
.  NAME##_bfd_link_just_syms, \
.  NAME##_bfd_copy_link_hash_symbol_type, \
.  NAME##_bfd_final_link, \
.  NAME##_bfd_link_split_section, \
.  NAME##_bfd_gc_sections, \
.  NAME##_bfd_lookup_section_flags, \
.  NAME##_bfd_merge_sections, \
.  NAME##_bfd_is_group_section, \
.  NAME##_bfd_discard_group, \
.  NAME##_section_already_linked, \
.  NAME##_bfd_define_common_symbol
.
.  int         (*_bfd_sizeof_headers) (bfd *, struct bfd_link_info *);
.  bfd_byte *  (*_bfd_get_relocated_section_contents)
.    (bfd *, struct bfd_link_info *, struct bfd_link_order *,
.     bfd_byte *, bfd_boolean, struct bfd_symbol **);
.
.  bfd_boolean (*_bfd_relax_section)
.    (bfd *, struct bfd_section *, struct bfd_link_info *, bfd_boolean *);
.
.  {* Create a hash table for the linker.  Different backends store
.     different information in this table.  *}
.  struct bfd_link_hash_table *
.              (*_bfd_link_hash_table_create) (bfd *);
.
.  {* Release the memory associated with the linker hash table.  *}
.  void        (*_bfd_link_hash_table_free) (struct bfd_link_hash_table *);
.
.  {* Add symbols from this object file into the hash table.  *}
.  bfd_boolean (*_bfd_link_add_symbols) (bfd *, struct bfd_link_info *);
.
.  {* Indicate that we are only retrieving symbol values from this section.  *}
.  void        (*_bfd_link_just_syms) (asection *, struct bfd_link_info *);
.
.  {* Copy the symbol type of a linker hash table entry.  *}
.#define bfd_copy_link_hash_symbol_type(b, t, f) \
.  BFD_SEND (b, _bfd_copy_link_hash_symbol_type, (b, t, f))
.  void (*_bfd_copy_link_hash_symbol_type)
.    (bfd *, struct bfd_link_hash_entry *, struct bfd_link_hash_entry *);
.
.  {* Do a link based on the link_order structures attached to each
.     section of the BFD.  *}
.  bfd_boolean (*_bfd_final_link) (bfd *, struct bfd_link_info *);
.
.  {* Should this section be split up into smaller pieces during linking.  *}
.  bfd_boolean (*_bfd_link_split_section) (bfd *, struct bfd_section *);
.
.  {* Remove sections that are not referenced from the output.  *}
.  bfd_boolean (*_bfd_gc_sections) (bfd *, struct bfd_link_info *);
.
.  {* Sets the bitmask of allowed and disallowed section flags.  *}
.  bfd_boolean (*_bfd_lookup_section_flags) (struct bfd_link_info *,
.					     struct flag_info *,
.					     asection *);
.
.  {* Attempt to merge SEC_MERGE sections.  *}
.  bfd_boolean (*_bfd_merge_sections) (bfd *, struct bfd_link_info *);
.
.  {* Is this section a member of a group?  *}
.  bfd_boolean (*_bfd_is_group_section) (bfd *, const struct bfd_section *);
.
.  {* Discard members of a group.  *}
.  bfd_boolean (*_bfd_discard_group) (bfd *, struct bfd_section *);
.
.  {* Check if SEC has been already linked during a reloceatable or
.     final link.  *}
.  bfd_boolean (*_section_already_linked) (bfd *, asection *,
.					   struct bfd_link_info *);
.
.  {* Define a common symbol.  *}
.  bfd_boolean (*_bfd_define_common_symbol) (bfd *, struct bfd_link_info *,
.					     struct bfd_link_hash_entry *);
.
.  {* Routines to handle dynamic symbols and relocs.  *}
.#define BFD_JUMP_TABLE_DYNAMIC(NAME) \
.  NAME##_get_dynamic_symtab_upper_bound, \
.  NAME##_canonicalize_dynamic_symtab, \
.  NAME##_get_synthetic_symtab, \
.  NAME##_get_dynamic_reloc_upper_bound, \
.  NAME##_canonicalize_dynamic_reloc
.
.  {* Get the amount of memory required to hold the dynamic symbols.  *}
.  long        (*_bfd_get_dynamic_symtab_upper_bound) (bfd *);
.  {* Read in the dynamic symbols.  *}
.  long        (*_bfd_canonicalize_dynamic_symtab)
.    (bfd *, struct bfd_symbol **);
.  {* Create synthetized symbols.  *}
.  long        (*_bfd_get_synthetic_symtab)
.    (bfd *, long, struct bfd_symbol **, long, struct bfd_symbol **,
.     struct bfd_symbol **);
.  {* Get the amount of memory required to hold the dynamic relocs.  *}
.  long        (*_bfd_get_dynamic_reloc_upper_bound) (bfd *);
.  {* Read in the dynamic relocs.  *}
.  long        (*_bfd_canonicalize_dynamic_reloc)
.    (bfd *, arelent **, struct bfd_symbol **);
.

A pointer to an alternative bfd_target in case the current one is not
satisfactory.  This can happen when the target cpu supports both big
and little endian code, and target chosen by the linker has the wrong
endianness.  The function open_output() in ld/ldlang.c uses this field
to find an alternative output format that is suitable.

.  {* Opposite endian version of this target.  *}
.  const struct bfd_target * alternative_target;
.

.  {* Data for use by back-end routines, which isn't
.     generic enough to belong in this structure.  *}
.  const void *backend_data;
.
.} bfd_target;
.
*/

extern const bfd_target asxxxx_vec;
/* These are always included.  */
extern const bfd_target srec_vec;
extern const bfd_target symbolsrec_vec;
extern const bfd_target binary_vec;
extern const bfd_target ihex_vec;
static const bfd_target * const _bfd_target_vector[] =
{
//#ifdef SELECT_VECS
#if 0

	SELECT_VECS,

#else /* not SELECT_VECS */

//#ifdef DEFAULT_VECTOR
#if 0
	&DEFAULT_VECTOR,
#endif
	&asxxxx_vec,
/* Always support S-records, for convenience.  */
	&srec_vec,
	&symbolsrec_vec,
/* Likewise for binary output.  */
	&binary_vec,
/* Likewise for ihex.  */
	&ihex_vec,
#endif
	NULL /* end of list marker */
};
const bfd_target * const *bfd_target_vector = _bfd_target_vector;

/* bfd_default_vector[0] contains either the address of the default vector,
   if there is one, or zero if there isn't.  */

const bfd_target *bfd_default_vector[] = {
//#ifdef DEFAULT_VECTOR
#if 0
	&DEFAULT_VECTOR,
#else
	&asxxxx_vec,
#endif
	NULL
};

/* bfd_associated_vector[] contains the associated target vectors used
   to reduce the ambiguity in bfd_check_format_matches.  */

static const bfd_target *_bfd_associated_vector[] = {
//#ifdef ASSOCIATED_VECS
#if 0
	ASSOCIATED_VECS,
#endif
	NULL
};

const bfd_target * const *bfd_associated_vector = _bfd_associated_vector;

/* When there is an ambiguous match, bfd_check_format_matches puts the
   names of the matching targets in an array.  This variable is the maximum
   number of entries that the array could possibly need.  */
const size_t _bfd_target_vector_entries = sizeof (_bfd_target_vector)/sizeof (*_bfd_target_vector);

/* This array maps configuration triplets onto BFD vectors.  */

struct targmatch
{
  /* The configuration triplet.  */
  const char *triplet;
  /* The BFD vector.  If this is NULL, then the vector is found by
     searching forward for the next structure with a non NULL vector
     field.  */
  const bfd_target *vector;
};

/* targmatch.h is built by Makefile out of config.bfd.  */
static const struct targmatch bfd_target_match[] = {
#include "targmatch.h"
  { NULL, NULL }
};

/* Find a target vector, given a name or configuration triplet.  */

static const bfd_target *
find_target (const char *name)
{
  const bfd_target * const *target;
  const struct targmatch *match;

  for (target = &bfd_target_vector[0]; *target != NULL; target++)
    if (strcmp (name, (*target)->name) == 0)
      return *target;

  /* If we couldn't match on the exact name, try matching on the
     configuration triplet.  FIXME: We should run the triplet through
     config.sub first, but that is hard.  */
  for (match = &bfd_target_match[0]; match->triplet != NULL; match++)
    {
      if (fnmatch (match->triplet, name, 0) == 0)
	{
	  while (match->vector == NULL)
	    ++match;
	  return match->vector;
	}
    }

  bfd_set_error (bfd_error_invalid_target);
  return NULL;
}

/*
FUNCTION
	bfd_set_default_target

SYNOPSIS
	bfd_boolean bfd_set_default_target (const char *name);

DESCRIPTION
	Set the default target vector to use when recognizing a BFD.
	This takes the name of the target, which may be a BFD target
	name or a configuration triplet.
*/

bfd_boolean
bfd_set_default_target (const char *name)
{
  const bfd_target *target;

  if (bfd_default_vector[0] != NULL
      && strcmp (name, bfd_default_vector[0]->name) == 0)
    return TRUE;

  target = find_target (name);
  if (target == NULL)
    return FALSE;

  bfd_default_vector[0] = target;
  return TRUE;
}

/*
FUNCTION
	bfd_find_target

SYNOPSIS
	const bfd_target *bfd_find_target (const char *target_name, bfd *abfd);

DESCRIPTION
	Return a pointer to the transfer vector for the object target
	named @var{target_name}.  If @var{target_name} is <<NULL>>,
	choose the one in the environment variable <<GNUTARGET>>; if
	that is null or not defined, then choose the first entry in the
	target list.  Passing in the string "default" or setting the
	environment variable to "default" will cause the first entry in
	the target list to be returned, and "target_defaulted" will be
	set in the BFD if @var{abfd} isn't <<NULL>>.  This causes
	<<bfd_check_format>> to loop over all the targets to find the
	one that matches the file being read.
*/

const bfd_target *
bfd_find_target (const char *target_name, bfd *abfd)
{
  const char *targname;
  const bfd_target *target;

  if (target_name != NULL)
    targname = target_name;
  else
    targname = getenv ("GNUTARGET");

  /* This is safe; the vector cannot be null.  */
  if (targname == NULL || strcmp (targname, "default") == 0)
    {
      if (bfd_default_vector[0] != NULL)
	target = bfd_default_vector[0];
      else
	target = bfd_target_vector[0];
      if (abfd)
	{
	  abfd->xvec = target;
	  abfd->target_defaulted = TRUE;
	}
      return target;
    }

  if (abfd)
    abfd->target_defaulted = FALSE;

  target = find_target (targname);
  if (target == NULL)
    return NULL;

  if (abfd)
    abfd->xvec = target;
  return target;
}

/* Helper function for bfd_get_target_info to determine the target's
   architecture.  This method handles bfd internal target names as
   tuples and triplets.  */
static bfd_boolean
_bfd_find_arch_match (const char *tname, const char **arch,
		      const char **def_target_arch)
{
  if (!arch)
    return FALSE;

  while (*arch != NULL)
    {
      const char *in_a = strstr (*arch, tname);
      char end_ch = (in_a ? in_a[strlen (tname)] : 0);

      if (in_a && (in_a == *arch || in_a[-1] == ':')
          && end_ch == 0)
        {
          *def_target_arch = *arch;
          return TRUE;
        }
      arch++;
    }
  return FALSE;
}

/*
FUNCTION
	bfd_get_target_info
SYNOPSIS
	const bfd_target *bfd_get_target_info (const char *target_name,
					       bfd *abfd,
				 	       bfd_boolean *is_bigendian,
					       int *underscoring,
					       const char **def_target_arch);
DESCRIPTION
        Return a pointer to the transfer vector for the object target
        named @var{target_name}.  If @var{target_name} is <<NULL>>,
        choose the one in the environment variable <<GNUTARGET>>; if
        that is null or not defined, then choose the first entry in the
        target list.  Passing in the string "default" or setting the
        environment variable to "default" will cause the first entry in
        the target list to be returned, and "target_defaulted" will be
        set in the BFD if @var{abfd} isn't <<NULL>>.  This causes
        <<bfd_check_format>> to loop over all the targets to find the
        one that matches the file being read.
	If @var{is_bigendian} is not <<NULL>>, then set this value to target's
	endian mode. True for big-endian, FALSE for little-endian or for
	invalid target.
	If @var{underscoring} is not <<NULL>>, then set this value to target's
	underscoring mode. Zero for none-underscoring, -1 for invalid target,
	else the value of target vector's symbol underscoring.
	If @var{def_target_arch} is not <<NULL>>, then set it to the architecture
	string specified by the target_name.
*/
const bfd_target *
bfd_get_target_info (const char *target_name, bfd *abfd,
		     bfd_boolean *is_bigendian,
		     int *underscoring, const char **def_target_arch)
{
  const bfd_target *target_vec;

  if (is_bigendian)
    *is_bigendian = FALSE;
  if (underscoring)
    *underscoring = -1;
  if (def_target_arch)
    *def_target_arch = NULL;
  target_vec = bfd_find_target (target_name, abfd);
  if (! target_vec)
    return NULL;
  if (is_bigendian)
    *is_bigendian = ((target_vec->byteorder == BFD_ENDIAN_BIG) ? TRUE
							       : FALSE);
  if (underscoring)
    *underscoring = ((int) target_vec->symbol_leading_char) & 0xff;

  if (def_target_arch)
    {
      const char *tname = target_vec->name;
      const char **arches = bfd_arch_list ();

      if (arches && tname)
        {
          char *hyp = strchr (tname, '-');

          if (hyp != NULL)
            {
              tname = ++hyp;

	      /* Make sure we detect architecture names
		 for triplets like "pe-arm-wince-little".  */
	      if (!_bfd_find_arch_match (tname, arches, def_target_arch))
		{
		  char new_tname[50];

		  strcpy (new_tname, hyp);
		  while ((hyp = strrchr (new_tname, '-')) != NULL)
		    {
		      *hyp = 0;
		      if (_bfd_find_arch_match (new_tname, arches,
			  			def_target_arch))
			break;
		    }
		}
	    }
	  else
	    _bfd_find_arch_match (tname, arches, def_target_arch);
	}

      if (arches)
        free (arches);
    }
  return target_vec;
}

/*
FUNCTION
	bfd_target_list

SYNOPSIS
	const char ** bfd_target_list (void);

DESCRIPTION
	Return a freshly malloced NULL-terminated
	vector of the names of all the valid BFD targets. Do not
	modify the names.

*/

const char **
bfd_target_list (void)
{
  int vec_length = 0;
  bfd_size_type amt;
  const bfd_target * const *target;
  const  char **name_list, **name_ptr;

  for (target = &bfd_target_vector[0]; *target != NULL; target++)
    vec_length++;

  amt = (vec_length + 1) * sizeof (char **);
  name_ptr = name_list = (const  char **) bfd_malloc (amt);

  if (name_list == NULL)
    return NULL;

  for (target = &bfd_target_vector[0]; *target != NULL; target++)
    if (target == &bfd_target_vector[0]
	|| *target != bfd_target_vector[0])
      *name_ptr++ = (*target)->name;

  *name_ptr = NULL;
  return name_list;
}

/*
FUNCTION
	bfd_seach_for_target

SYNOPSIS
	const bfd_target *bfd_search_for_target
	  (int (*search_func) (const bfd_target *, void *),
	   void *);

DESCRIPTION
	Return a pointer to the first transfer vector in the list of
	transfer vectors maintained by BFD that produces a non-zero
	result when passed to the function @var{search_func}.  The
	parameter @var{data} is passed, unexamined, to the search
	function.
*/

const bfd_target *
bfd_search_for_target (int (*search_func) (const bfd_target *, void *),
		       void *data)
{
  const bfd_target * const *target;

  for (target = bfd_target_vector; *target != NULL; target ++)
    if (search_func (*target, data))
      return *target;

  return NULL;
}
