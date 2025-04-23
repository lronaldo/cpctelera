// dummies for function used in cc1

#define SDCPP_DUMMY_FCT() fprintf(stderr, "unreachable dummy in cc1%s\n", __func__); gcc_assert(false)
#define untested() fprintf(stderr, "untested in cc1: %s\n", __func__)
#define CC1_DUMMIES

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "bitmap.h"

#include "tree-core.h"
#include "langhooks.h"
#include "langhooks-def.h"

#include "incpath.h"
#include "c/c-parser.h"
#include "substring-locations.h"
#include "pass_manager.h"

void bitmap_obstack_initialize(bitmap_obstack*)
{
}
void bitmap_obstack_free(bitmap_head*)
{ SDCPP_DUMMY_FCT();
}
void bitmap_obstack_release(bitmap_obstack*)
{ SDCPP_DUMMY_FCT();
}
bitmap bitmap_alloc(bitmap_obstack*)
{
  static bitmap bogus_bitmap;
  return bogus_bitmap;
}
void init_eh(void)
{
}
tree build_tree_list(tree_node*, tree_node*)
{ SDCPP_DUMMY_FCT();
	return tree();
}
bool c_block_may_fallthru(tree_node const*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void c_common_read_pch(cpp_reader*, char const*, int, char const*)
{ SDCPP_DUMMY_FCT();
}
int c_common_valid_pch(cpp_reader*, char const*, int)
{ SDCPP_DUMMY_FCT();
	return 0;
}
bool c_dump_tree(void*, tree_node*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
int c_get_alias_set(tree_node*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void c_initialize_diagnostics(diagnostic_context*)
{
}
int c_gimplify_expr(tree_node**, gimple**, gimple**)
{ SDCPP_DUMMY_FCT();
	return 0;
}
bool c_missing_noreturn_ok_p(tree_node*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void c_parse_file()
{ SDCPP_DUMMY_FCT();
}
void c_parse_final_cleanups()
{ SDCPP_DUMMY_FCT();
}
void c_print_identifier(FILE*, tree_node*, int)
{ SDCPP_DUMMY_FCT();
}
void c_stddef_cpp_builtins()
{ SDCPP_DUMMY_FCT();
}
int c_types_compatible_p(tree_node*, tree_node*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void default_options_optimization(gcc_options*, gcc_options*, cl_decoded_option*, unsigned int, unsigned int, unsigned int, cl_option_handlers const*, diagnostic_context*)
{
}
void define_language_independent_builtin_macros(cpp_reader*)
{ untested();
}
void dump_end(int, FILE*)
{ SDCPP_DUMMY_FCT();
}
void dbg_cnt_list_all_counters()
{ SDCPP_DUMMY_FCT();
}
FILE* dump_begin(int, dump_flag*, int)
{ SDCPP_DUMMY_FCT();
	return NULL;
}
void dump_profile_report()
{ SDCPP_DUMMY_FCT();
}
const struct gcc_debug_hooks *
dump_go_spec_init(char const*, const struct gcc_debug_hooks*)
{ SDCPP_DUMMY_FCT();
}
bool dwarf2out_default_as_loc_support()
{
	return false;
}
bool dwarf2out_default_as_locview_support()
{
	return false;
}
void dwarf2out_do_cfi_asm()
{ SDCPP_DUMMY_FCT();
}
void emergency_dump_function()
{ untested();
}
void finalize_plugins()
{
}
void gcc::dump_manager::register_dumps()
{
}
tree get_identifier(char const*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
int ggc_mark_stringpool()
{ SDCPP_DUMMY_FCT();
	return 0;
}
int ggc_purge_stringpool()
{ SDCPP_DUMMY_FCT();
	return 0;
}
void gt_clear_caches()
{ SDCPP_DUMMY_FCT();
}
void gt_ggc_m_S(void const*)
{ SDCPP_DUMMY_FCT();
}
void gt_pch_note_reorder (void*, void*, gt_handle_reorder)
{ SDCPP_DUMMY_FCT();
}
void gt_pch_p_S(void*, void*, void (*)(void*, void*, void*), void*)
{ SDCPP_DUMMY_FCT();
}
int gt_pch_restore_stringpool()
{ SDCPP_DUMMY_FCT();
	return 0;
}
int gt_pch_save_stringpool()
{ SDCPP_DUMMY_FCT();
	return 0;
}
int hook_int_const_tree_int(tree_node const*, int)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void init_stringpool()
{
}
void initialize_plugins()
{
}
bool lang_GNU_C()
{
	return true;
}
void lhd_append_data(void const*, size_t, void*)
{ SDCPP_DUMMY_FCT();
}
void lhd_begin_section(char const*)
{ SDCPP_DUMMY_FCT();
}
bool lhd_decl_ok_for_sibcall(tree_node const*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
const char* lhd_decl_printable_name(tree_node*, int)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void lhd_do_nothing()
{ untested();
}
void lhd_do_nothing_t(tree_node*)
{ SDCPP_DUMMY_FCT();
}
const char* lhd_dwarf_name(tree_node*, int)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void lhd_end_section()
{ SDCPP_DUMMY_FCT();
}
const char* lhd_get_substring_location(substring_loc const&, unsigned int*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void lhd_overwrite_decl_assembler_name(tree_node*, tree_node*)
{ SDCPP_DUMMY_FCT();
}
void lhd_print_error_function(diagnostic_context*, char const*, diagnostic_info*)
{ untested();
}
void lhd_print_tree_nothing(FILE*, tree_node*, int)
{ SDCPP_DUMMY_FCT();
}
void lhd_set_decl_assembler_name(tree_node*)
{ SDCPP_DUMMY_FCT();
}
int lhd_tree_dump_type_quals(tree_node const*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
size_t lhd_tree_size(tree_code)
{ SDCPP_DUMMY_FCT();
	return 0;
}
int lhd_type_dwarf_attribute(tree_node const*, int)
{ SDCPP_DUMMY_FCT();
	return 0;
}
bool names_builtin_p(char const*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void output_operand_lossage (const char *, ...)
{ SDCPP_DUMMY_FCT();
}
void parse_basever(int*, int*, int*)
{ SDCPP_DUMMY_FCT();
}
void pch_cpp_save_state()
{
}
void pch_init()
{ SDCPP_DUMMY_FCT();
}
void pop_file_scope()
{ SDCPP_DUMMY_FCT();
}
void print_plugins_help(FILE*, char const*)
{
	// no-op
}
void push_file_scope()
{ SDCPP_DUMMY_FCT();
}
tree pushdecl(tree_node*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
void tree_diagnostics_defaults(diagnostic_context*)
{
}
tree lhd_enum_underlying_base_type(tree_node const*)
{ SDCPP_DUMMY_FCT();
	return tree();
}
tree lhd_make_node(tree_code)
{ SDCPP_DUMMY_FCT();
	return tree();
}
tree* lhd_omp_get_decl_init(tree_node*)
{ SDCPP_DUMMY_FCT();
	return 0;
}
tree lhd_pass_through_t(tree_node*)
{ SDCPP_DUMMY_FCT();
	return tree();
}
void lhd_register_dumps(gcc::dump_manager*)
{ untested();
}
tree lhd_return_null_const_tree(tree_node const*)
{ SDCPP_DUMMY_FCT();
	return tree();
}
tree lhd_unit_size_without_reusable_padding(tree_node*)
{ SDCPP_DUMMY_FCT();
	return tree();
}
void statistics_early_init()
{
}
void virt_loc_aware_diagnostic_finalizer(diagnostic_context*, diagnostic_info*)
{ untested();
}
void warn_if_plugins()
{ untested();
}
/*----------------------------------------------------------------------*/
namespace gcc{
pass_manager::~pass_manager ()
{ SDCPP_DUMMY_FCT();
}
pass_manager::pass_manager (context*)
{ SDCPP_DUMMY_FCT();
}
dump_manager::~dump_manager()
{ SDCPP_DUMMY_FCT();
}
dump_manager::dump_manager()
{
}
}
/*----------------------------------------------------------------------*/
bool after_memory_report;
FILE *dump_file = NULL;
extern const struct attribute_spec c_common_format_attribute_table[] =
{
  { "dummy",                 3, 3, false, true,  true, false,
			      NULL, NULL } };
extern const struct attribute_spec c_common_attribute_table[] = {
	{ "dummy",  1, 1, false, true, false, true,
               NULL, NULL } };
struct ht *ident_hash;
/*----------------------------------------------------------------------*/
ggc_root_tab my_ggc_root_tab;
const struct ggc_root_tab * const gt_ggc_deletable_rtab[] = { &my_ggc_root_tab };
const struct ggc_root_tab * const gt_ggc_rtab[] = { &my_ggc_root_tab };
const struct ggc_root_tab * const gt_pch_scalar_rtab[] = { &my_ggc_root_tab };
/*----------------------------------------------------------------------*/
bool this_is_asm_operands = 0;
/*----------------------------------------------------------------------*/
