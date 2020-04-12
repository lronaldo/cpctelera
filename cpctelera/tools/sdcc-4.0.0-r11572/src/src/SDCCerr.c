/*
 * SDCCerr - Standard error handler
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include "SDCCglobl.h"
#ifdef HAVE_BACKTRACE_SYMBOLS_FD
#include <unistd.h>
#include <execinfo.h>
#endif

#include "SDCCerr.h"

#define NELEM(x) (sizeof (x) / sizeof *(x))

#define USE_STDOUT_FOR_ERRORS   0

#if USE_STDOUT_FOR_ERRORS
#define DEFAULT_ERROR_OUT       stdout
#else
#define DEFAULT_ERROR_OUT       stderr
#endif

struct SDCCERRG _SDCCERRG; 

extern char *filename;
extern int lineno;
extern int fatalError;

/* Currently the errIndex field must match the position of the 
 * entry in the array. It is only included in order to make 
 * human error lookup easier.
 */
struct
{
  int errIndex;
  ERROR_LOG_LEVEL errType;
  const char *errText;
  char disabled;
} ErrTab [] =
{
  { E_DUPLICATE, ERROR_LEVEL_ERROR,
     "Duplicate symbol '%s', symbol IGNORED", 0 },
  { E_SYNTAX_ERROR, ERROR_LEVEL_ERROR,
     "Syntax error, declaration ignored at '%s'", 0 },
  { E_CONST_EXPECTED, ERROR_LEVEL_ERROR,
      "Initializer element is not a constant expression", 0 },
  { E_OUT_OF_MEM, ERROR_LEVEL_ERROR,
     "'malloc' failed file '%s' for size %ld", 0 },
  { E_FILE_OPEN_ERR, ERROR_LEVEL_ERROR,
     "'fopen' failed on file '%s'", 0 },
  { E_INVALID_OCLASS, ERROR_LEVEL_ERROR,
     "Internal Error Oclass invalid '%s'", 0 },
  { E_CANNOT_ALLOC, ERROR_LEVEL_ERROR,
     "Cannot allocate variable '%s'.", 0 },
  { E_OLD_STYLE, ERROR_LEVEL_ERROR,
     "Old style C declaration. IGNORED '%s'", 0 },
  { E_STACK_OUT, ERROR_LEVEL_ERROR,
     "Out of stack Space. '%s' not allocated", 0 },
  { E_INTERNAL_ERROR, ERROR_LEVEL_ERROR,
     "FATAL Compiler Internal Error in file '%s' line number '%d' : %s \n"
     "Contact Author with source code", 0 },
  { E_LVALUE_REQUIRED, ERROR_LEVEL_ERROR,
     "'lvalue' required for '%s' operation.", 0 },
  { E_TMPFILE_FAILED, ERROR_LEVEL_ERROR,
     "Creation of temp file failed", 0 },
  { E_FUNCTION_EXPECTED, ERROR_LEVEL_ERROR,
     "called object is not a function", 0 },
  { E_USING_ERROR, ERROR_LEVEL_ERROR,
     "'using', 'interrupt' or 'reentrant' must follow a function definition.'%s'", 0 },
  { E_SFR_INIT, ERROR_LEVEL_ERROR,
     "Absolute address & initial value both cannot be specified for\n"
     " a 'sfr','sbit' storage class, initial value ignored '%s'", 0 },
  { W_INIT_IGNORED, ERROR_LEVEL_WARNING,
     "Variable in the storage class cannot be initialized.'%s'", 0 },
  { E_AUTO_ASSUMED, ERROR_LEVEL_ERROR,
     "variable '%s' must be static to have storage class in reentrant function", 0 },
  { E_AUTO_ABSA, ERROR_LEVEL_ERROR,
     "absolute address not allowed for automatic var '%s' in reentrant function ", 0 },
  { W_INIT_WRONG, ERROR_LEVEL_WARNING,
     "Initializer different levels of indirections", 0 },
  { E_FUNC_REDEF, ERROR_LEVEL_ERROR,
     "Function name '%s' redefined ", 0 },
  { E_ID_UNDEF, ERROR_LEVEL_ERROR,
     "Undefined identifier '%s'", 0 },
  { W_STACK_OVERFLOW, ERROR_LEVEL_WARNING,
     "stack exceeds 256 bytes for function '%s'", 0 },
  { E_NEED_ARRAY_PTR, ERROR_LEVEL_ERROR,
     "Array or pointer required for '%s' operation ", 0 },
  { E_IDX_NOT_INT, ERROR_LEVEL_ERROR,
     "Array index not an integer", 0 },
  { W_IDX_OUT_OF_BOUNDS, ERROR_LEVEL_WARNING,
     "index %i is outside of the array bounds (array size is %i)", 0 },
  { E_STRUCT_UNION, ERROR_LEVEL_ERROR,
     "Structure/Union expected left of '.%s'", 0 },
  { E_NOT_MEMBER, ERROR_LEVEL_ERROR,
     "'%s' not a structure/union member", 0 },
  { E_PTR_REQD, ERROR_LEVEL_ERROR,
     "Pointer required", 0 },
  { E_UNARY_OP, ERROR_LEVEL_ERROR,
     "'unary %c': illegal operand", 0 },
  { E_CONV_ERR, ERROR_LEVEL_ERROR,
     "conversion error: integral promotion failed", 0 },
  { E_BITFLD_TYPE, ERROR_LEVEL_ERROR,
     "invalid type for bit-field", 0 },
  { E_BITFLD_SIZE, ERROR_LEVEL_ERROR,
     "bit-field size too wide for type (max %d bits)", 0 },
  { W_TRUNCATION, ERROR_LEVEL_WARNING,
     "high order truncation might occur", 0 },
  { E_CODE_WRITE, ERROR_LEVEL_ERROR,
     "Attempt to assign value to a constant variable (%s)", 0 },
  { E_LVALUE_CONST, ERROR_LEVEL_ERROR,
     "Lvalue specifies constant object", 0 },
  { E_ILLEGAL_ADDR, ERROR_LEVEL_ERROR,
     "'&' illegal operand, %s", 0 },
  { E_CAST_ILLEGAL, ERROR_LEVEL_ERROR,
     "illegal cast (cast cannot be aggregate)", 0 },
  { E_MULT_INTEGRAL, ERROR_LEVEL_ERROR,
     "'*' bad operand", 0 },
  { E_ARG_ERROR, ERROR_LEVEL_ERROR,
     "Argument count error, argument ignored", 0 },
  { E_ARG_COUNT, ERROR_LEVEL_ERROR,
     "Function was expecting more arguments", 0 },
  { E_FUNC_EXPECTED, ERROR_LEVEL_ERROR,
     "Function name expected '%s'. ANSI style declaration REQUIRED", 0 },
  { E_PLUS_INVALID, ERROR_LEVEL_ERROR,
     "invalid operand '%s'", 0 },
  { E_PTR_PLUS_PTR, ERROR_LEVEL_ERROR,
     "pointer + pointer invalid", 0 },
  { E_SHIFT_OP_INVALID, ERROR_LEVEL_ERROR,
     "invalid operand for shift operator", 0 },
  { E_COMPARE_OP, ERROR_LEVEL_ERROR,
     "operands are not comparable", 0 },
  { E_BITWISE_OP, ERROR_LEVEL_ERROR,
     "operand invalid for bitwise operation", 0 },
  { E_ANDOR_OP, ERROR_LEVEL_ERROR,
     "Invalid operand for '&&' or '||'", 0 },
  { E_TYPE_MISMATCH, ERROR_LEVEL_ERROR,
     "indirections to different types %s %s ", 0 },
  { E_ARRAY_ASSIGN, ERROR_LEVEL_ERROR,
     "cannot assign values to arrays", 0 },
  { E_ARRAY_DIRECT, ERROR_LEVEL_ERROR,
     "bit Arrays can be accessed by literal index only", 0 },
  { E_BIT_ARRAY, ERROR_LEVEL_ERROR,
     "Array or Pointer to bit|sbit|sfr not allowed.'%s'", 0 },
  { E_DUPLICATE_TYPEDEF, ERROR_LEVEL_ERROR,
     "typedef/enum '%s' duplicate. Previous definition Ignored", 0 },
  { E_ARG_TYPE, ERROR_LEVEL_ERROR,
     "Actual Argument type different from declaration %d", 0 },
  { E_RET_VALUE, ERROR_LEVEL_ERROR,
     "Function return value mismatch", 0 },
  { E_FUNC_AGGR, ERROR_LEVEL_ERROR,
     "Function cannot return aggregate. Func body ignored", 0 },
  { E_FUNC_DEF, ERROR_LEVEL_ERROR,
     "ANSI Style declaration needed", 0 },
  { E_DUPLICATE_LABEL, ERROR_LEVEL_ERROR,
     "Duplicate label '%s'", 0 },
  { E_LABEL_UNDEF, ERROR_LEVEL_ERROR,
     "Label undefined '%s'", 0 },
  { E_FUNC_VOID, ERROR_LEVEL_ERROR,
     "void function returning value", 0 },
  { W_VOID_FUNC, ERROR_LEVEL_WARNING,
     "function '%s' must return value", 0 },
  { W_RETURN_MISMATCH, ERROR_LEVEL_WARNING,
     "function return value mismatch", 0 },
  { E_CASE_CONTEXT, ERROR_LEVEL_ERROR,
     "'case/default' found without 'switch'. Statement ignored", 0 },
  { E_CASE_CONSTANT, ERROR_LEVEL_ERROR,
     "'case' expression not constant. Statement ignored", 0 },
  { E_BREAK_CONTEXT, ERROR_LEVEL_ERROR,
     "'break/continue' statement out of context", 0 },
  { E_SWITCH_AGGR, ERROR_LEVEL_ERROR,
     "nonintegral used in switch expression", 0 },
  { E_FUNC_BODY, ERROR_LEVEL_ERROR,
     "function '%s' already has body", 0 },
  { E_UNKNOWN_SIZE, ERROR_LEVEL_ERROR,
     "attempt to allocate variable of unknown size '%s'", 0 },
  { E_AUTO_AGGR_INIT, ERROR_LEVEL_ERROR,
     "aggregate 'auto' variable '%s' cannot be initialized", 0 },
  { E_INIT_COUNT, ERROR_LEVEL_ERROR,
     "too many initializers", 0 },
  { E_INIT_STRUCT, ERROR_LEVEL_ERROR,
     "struct/union/array '%s': initialization needs curly braces", 0 },
  { E_INIT_NON_ADDR, ERROR_LEVEL_ERROR,
     "non-address initialization expression", 0 },
  { E_INT_DEFINED, ERROR_LEVEL_ERROR,
     "interrupt no '%d' already has a service routine '%s'", 0 },
  { E_INT_ARGS, ERROR_LEVEL_ERROR,
     "interrupt routine cannot have arguments, arguments ignored", 0 },
  { E_INCLUDE_MISSING, ERROR_LEVEL_ERROR,
     "critical compiler #include file missing.            ", 0 },
  { E_NO_MAIN, ERROR_LEVEL_ERROR,
     "function 'main' undefined", 0 },
  { E_UNUSED_75, ERROR_LEVEL_ERROR,
     "this errorcode is no longer used", 0 },
  { E_PRE_PROC_FAILED, ERROR_LEVEL_ERROR,
     "Pre-Processor %s", 0 },
  { E_DUP_FAILED, ERROR_LEVEL_ERROR,
     "_dup call failed", 0 },
  { E_INCOMPAT_TYPES, ERROR_LEVEL_ERROR,
     "incompatible types", 0 },
  { W_LOOP_ELIMINATE, ERROR_LEVEL_WARNING,
     "'while' loop with 'zero' constant. Loop eliminated", 0 },
  { W_NO_SIDE_EFFECTS, ERROR_LEVEL_WARNING,
     "%s expression has NO side effects. Expr eliminated", 0 },
  { W_CONST_TOO_LARGE, ERROR_LEVEL_PEDANTIC,
     "constant value '%ld', out of range.", 0 },
  { W_BAD_COMPARE, ERROR_LEVEL_WARNING,
     "comparison will either, ALWAYs succeed or ALWAYs fail", 0 },
  { E_TERMINATING, ERROR_LEVEL_ERROR,
     "Compiler Terminating , contact author with source", 0 },
  { W_LOCAL_NOINIT, ERROR_LEVEL_WARNING,
     "'auto' variable '%s' may be used before initialization", 0 },
  { W_NO_REFERENCE, ERROR_LEVEL_WARNING,
     "in function %s unreferenced %s : '%s'", 0 },
  { E_OP_UNKNOWN_SIZE, ERROR_LEVEL_ERROR,
     "unknown size for operand", 0 },
  { W_LONG_UNSUPPORTED, ERROR_LEVEL_WARNING,
     "'%s' 'long' not supported , declared as 'int'.", 0 },
  { W_LITERAL_GENERIC, ERROR_LEVEL_WARNING,
     "cast of LITERAL value to 'generic' pointer", 0 },
  { E_SFR_ADDR_RANGE, ERROR_LEVEL_ERROR,
     "%s '%s' address out of range", 0 },
  { E_BITVAR_STORAGE, ERROR_LEVEL_ERROR,
     "storage class CANNOT be specified for bit variable '%s'", 0 },
  { E_EXTERN_MISMATCH, ERROR_LEVEL_ERROR,
     "extern definition for '%s' mismatches with declaration.", 0 },
  { E_NONRENT_ARGS, ERROR_LEVEL_ERROR,
     "Functions called via pointers must be 'reentrant' to take this many (bytes for) arguments", 0 },
  { W_DOUBLE_UNSUPPORTED, ERROR_LEVEL_WARNING,
     "type 'double' not supported assuming 'float'", 0 },
  { W_COMP_RANGE, ERROR_LEVEL_PEDANTIC,
     "comparison is always %s due to limited range of data type", 0 },
  { W_FUNC_NO_RETURN, ERROR_LEVEL_WARNING,
     "no 'return' statement found for function '%s'", 0 },
  { W_PRE_PROC_WARNING, ERROR_LEVEL_WARNING,
     "Pre-Processor %s", 0 },
  { E_STRUCT_AS_ARG, ERROR_LEVEL_ERROR,
     "SDCC cannot pass structure '%s' as function argument", 0 },
  { E_PREV_DECL_CONFLICT, ERROR_LEVEL_ERROR,
     "conflict with previous declaration of '%s' for attribute '%s' at %s:%d", 0 },
  { E_CODE_NO_INIT, ERROR_LEVEL_WARNING,
     "variable '%s' declared in code space must have initialiser", 0 },
  { E_OPS_INTEGRAL, ERROR_LEVEL_ERROR,
     "operands not integral for assignment operation", 0 },
  { E_TOO_MANY_PARMS, ERROR_LEVEL_ERROR,
     "too many parameters ", 0 },
  { E_TOO_FEW_PARMS, ERROR_LEVEL_ERROR,
     "too few parameters", 0 },
  { E_FUNC_NO_CODE, ERROR_LEVEL_ERROR,
     "code not generated for '%s' due to previous errors", 0 },
  { E_TYPE_MISMATCH_PARM, ERROR_LEVEL_ERROR,
     "type mismatch for parameter number %d", 0 },
  { E_INVALID_FLOAT_CONST, ERROR_LEVEL_ERROR,
     "invalid float constant '%s'", 0 },
  { E_INVALID_OP, ERROR_LEVEL_ERROR,
     "invalid operand for '%s' operation", 0 },
  { E_SWITCH_NON_INTEGER, ERROR_LEVEL_ERROR,
     "switch value not an integer", 0 },
  { E_CASE_NON_INTEGER, ERROR_LEVEL_ERROR,
     "case label not an integer", 0 },
  { W_FUNC_TOO_LARGE, ERROR_LEVEL_WARNING,
     "function '%s' too large for global optimization", 0 },
  { W_CONTROL_FLOW, ERROR_LEVEL_PEDANTIC,
     "conditional flow changed by optimizer: so said EVELYN the modified DOG", 0 },
  { W_PTR_TYPE_INVALID, ERROR_LEVEL_WARNING,
     "invalid type specifier for pointer type; specifier ignored", 0 },
  { W_IMPLICIT_FUNC, ERROR_LEVEL_WARNING,
     "function '%s' implicit declaration", 0 },
  { W_CONTINUE, ERROR_LEVEL_WARNING,
     "%s", 0 },
  { I_EXTENDED_STACK_SPILS, ERROR_LEVEL_INFO,
     "extended stack by %d bytes for compiler temp(s) :in function  '%s': %s ", 0 },
  { W_UNKNOWN_PRAGMA, ERROR_LEVEL_WARNING,
     "unknown or unsupported #pragma directive '%s'", 0 },
  { W_SHIFT_CHANGED, ERROR_LEVEL_PEDANTIC,
     "%s shifting more than size of object changed to zero", 0 },
  { W_UNKNOWN_OPTION, ERROR_LEVEL_WARNING,
     "unknown compiler option '%s' ignored", 0 },
  { W_UNSUPP_OPTION, ERROR_LEVEL_WARNING,
     "option '%s' no longer supported  '%s' ", 0 },
  { E_UNKNOWN_FEXT, ERROR_LEVEL_ERROR,
     "don't know what to do with file '%s'. file extension unsupported", 0 },
  { W_TOO_MANY_SRC, ERROR_LEVEL_WARNING,
     "cannot compile more than one source file. file '%s' ignored", 0 },
  { I_CYCLOMATIC, ERROR_LEVEL_INFO,
     "function '%s', # edges %d , # nodes %d , cyclomatic complexity %d", 0 },
  { E_DIVIDE_BY_ZERO, ERROR_LEVEL_WARNING,
     "dividing by 0", 0 },
  { E_FUNC_BIT, ERROR_LEVEL_ERROR,
     "function cannot return 'bit'", 0 },
  { E_CAST_ZERO, ERROR_LEVEL_ERROR,
     "casting from to type 'void' is illegal", 0 },
  { W_CONST_RANGE, ERROR_LEVEL_WARNING,
     "constant is out of range %s", 0 },
  { W_CODE_UNREACH, ERROR_LEVEL_PEDANTIC,
     "unreachable code", 0 },
  { W_NONPTR2_GENPTR, ERROR_LEVEL_WARNING,
     "non-pointer type cast to generic pointer", 0 },
  { W_POSSBUG, ERROR_LEVEL_WARNING,
     "possible code generation error at line %d,\n"
     " send source to sandeep.dutta@usa.net", 0 },
  { E_INCOMPAT_PTYPES, ERROR_LEVEL_ERROR,
     "pointer types incompatible ", 0 },
  { W_UNKNOWN_MODEL, ERROR_LEVEL_WARNING,
     "unknown memory model at %s : %d", 0 },
  { E_UNKNOWN_TARGET, ERROR_LEVEL_ERROR,
     "cannot generate code for target '%s'", 0 },
  { W_INDIR_BANKED, ERROR_LEVEL_WARNING,
     "Indirect call to a banked function not implemented.", 0 },
  { W_UNSUPPORTED_MODEL, ERROR_LEVEL_WARNING,
     "Model '%s' not supported for %s, ignored.", 0 },
  { W_BANKED_WITH_NONBANKED, ERROR_LEVEL_WARNING,
     "Both banked and nonbanked attributes used. nonbanked wins.", 0 },
  { W_BANKED_WITH_STATIC, ERROR_LEVEL_WARNING, // no longer used
     "Both banked and static used.  static wins.", 0 },
  { W_INT_TO_GEN_PTR_CAST, ERROR_LEVEL_WARNING,
     "converting integer type to generic pointer: assuming XDATA", 0 },
  { W_ESC_SEQ_OOR_FOR_CHAR, ERROR_LEVEL_WARNING,
     "escape sequence out of range for char.", 0 },
  { E_INVALID_HEX, ERROR_LEVEL_ERROR,
     "\\x used with no following hex digits.", 0 },
  { W_FUNCPTR_IN_USING_ISR, ERROR_LEVEL_WARNING,
     "call via function pointer in ISR using non-zero register bank.\n"
     "            Cannot determine which register bank to save.", 0 },
  { E_NO_SUCH_BANK, ERROR_LEVEL_ERROR,
     "called function uses unknown register bank %d.", 0 },
  { E_TWO_OR_MORE_DATA_TYPES, ERROR_LEVEL_ERROR,
     "two or more data types in declaration for '%s'", 0 },
  { E_LONG_OR_SHORT_INVALID, ERROR_LEVEL_ERROR,
     "long or short specified for %s '%s'", 0 },
  { E_SIGNED_OR_UNSIGNED_INVALID, ERROR_LEVEL_ERROR,
     "signed or unsigned specified for %s '%s'", 0 },
  { E_LONG_AND_SHORT_INVALID, ERROR_LEVEL_ERROR,
     "both long and short specified for %s '%s'", 0 },
  { E_SIGNED_AND_UNSIGNED_INVALID, ERROR_LEVEL_ERROR,
     "both signed and unsigned specified for %s '%s'", 0 },
  { E_TWO_OR_MORE_STORAGE_CLASSES, ERROR_LEVEL_ERROR,
     "two or more storage classes in declaration for '%s'", 0 },
  { W_EXCESS_INITIALIZERS, ERROR_LEVEL_WARNING,
     "excess elements in %s initializer after '%s'", 0 },
  { E_ARGUMENT_MISSING, ERROR_LEVEL_ERROR,
     "Option %s requires an argument.", 0 },
  { W_STRAY_BACKSLASH, ERROR_LEVEL_WARNING,
     "stray '\\' at column %d", 0 },
  { W_NEWLINE_IN_STRING, ERROR_LEVEL_WARNING,
     "newline in string constant", 0 },
  { W_USING_GENERIC_POINTER, ERROR_LEVEL_WARNING,
     "using generic pointer %s to initialize %s", 0 },
  { W_EXCESS_SHORT_OPTIONS, ERROR_LEVEL_WARNING,
     "Only one short option can be specified at a time.  Rest of %s ignored.", 0 },
  { E_VOID_VALUE_USED, ERROR_LEVEL_ERROR,
     "void value not ignored as it ought to be", 0 },
  { W_INTEGRAL2PTR_NOCAST, ERROR_LEVEL_WARNING,
     "converting integral to pointer without a cast", 0 },
  { W_PTR2INTEGRAL_NOCAST, ERROR_LEVEL_WARNING,
     "converting pointer to integral without a cast", 0 },
  { W_SYMBOL_NAME_TOO_LONG, ERROR_LEVEL_WARNING,
     "symbol name too long, truncated to %d chars", 0 },
  { W_CAST_STRUCT_PTR, ERROR_LEVEL_WARNING,
     "cast of struct %s * to struct %s * ", 0 },
  { W_LIT_OVERFLOW, ERROR_LEVEL_PEDANTIC,
     "overflow in implicit constant conversion", 0 },
  { E_PARAM_NAME_OMITTED, ERROR_LEVEL_ERROR,
     "in function %s: name omitted for parameter %d", 0 },
  { W_NO_FILE_ARG_IN_C1, ERROR_LEVEL_WARNING,
     "only standard input is compiled in c1 mode. file '%s' ignored", 0 },
  { E_NEED_OPT_O_IN_C1, ERROR_LEVEL_ERROR,
     "must specify assembler file name with -o in c1 mode", 0 },
  { W_ILLEGAL_OPT_COMBINATION, ERROR_LEVEL_WARNING,
     "illegal combination of options (--c1mode, -E, -S -c)", 0 },
  { E_DUPLICATE_MEMBER, ERROR_LEVEL_ERROR,
     "duplicate %s member '%s'", 0 },
  { E_STACK_VIOLATION, ERROR_LEVEL_ERROR,
     "'%s' internal stack %s", 0 },
  { W_INT_OVL, ERROR_LEVEL_PEDANTIC,
     "integer overflow in expression", 0 },
  { W_USELESS_DECL, ERROR_LEVEL_WARNING,
     "useless declaration (possible use of keyword as variable name)", 0 },
  { E_INT_BAD_INTNO, ERROR_LEVEL_ERROR,
     "interrupt number '%u' is not valid", 0 },
  { W_BITFLD_NAMED, ERROR_LEVEL_WARNING,
     "ignoring declarator of 0 length bitfield", 0 },
  { E_FUNC_ATTR, ERROR_LEVEL_ERROR,
     "function attribute following non-function declaration"},
  { W_SAVE_RESTORE, ERROR_LEVEL_PEDANTIC,
     "unmatched #pragma save and #pragma restore", 0 },
  { E_INVALID_CRITICAL, ERROR_LEVEL_ERROR,
     "not allowed in a critical section or critical function", 0 },
  { E_NOT_ALLOWED, ERROR_LEVEL_ERROR,
     "%s not allowed here", 0 },
  { E_BAD_TAG, ERROR_LEVEL_ERROR,
     "'%s' is not a %s tag", 0 },
  { E_ENUM_NON_INTEGER, ERROR_LEVEL_ERROR,
     "enumeration constant not an integer", 0 },
  { W_DEPRECATED_PRAGMA, ERROR_LEVEL_WARNING,
     "pragma %s is deprecated, please see documentation for details", 0 },
  { E_SIZEOF_INCOMPLETE_TYPE, ERROR_LEVEL_ERROR,
     "sizeof applied to an incomplete type", 0 },
  { E_PREVIOUS_DEF, ERROR_LEVEL_ERROR,
     "previously defined here", 0 },
  { W_SIZEOF_VOID, ERROR_LEVEL_WARNING,
     "size of void is zero", 0 },
  { W_POSSBUG2, ERROR_LEVEL_WARNING,
     "possible code generation error at %s line %d,\n"
     " please report problem and send source code at sdcc-user list on sourceforge.net"},
  { W_COMPLEMENT, ERROR_LEVEL_WARNING,
     "using ~ on bit/bool/unsigned char variables can give unexpected results due to promotion to int", 0 },
  { E_SHADOWREGS_NO_ISR, ERROR_LEVEL_ERROR,
     "ISR function attribute 'shadowregs' following non-ISR function '%s'", 0 },
  { W_SFR_ABSRANGE, ERROR_LEVEL_WARNING,
     "absolute address for sfr '%s' probably out of range.", 0 },
  { E_BANKED_WITH_CALLEESAVES, ERROR_LEVEL_ERROR,
     "Both banked and callee-saves cannot be used together.", 0 },
  { W_INVALID_INT_CONST, ERROR_LEVEL_WARNING,
     "integer constant '%s' out of range, truncated to %.0lf.", 0 },
  { W_CMP_SU_CHAR, ERROR_LEVEL_PEDANTIC,
     "comparison of 'signed char' with 'unsigned char' requires promotion to int", 0 },
  { W_INVALID_FLEXARRAY, ERROR_LEVEL_WARNING,
     "invalid use of structure with flexible array member", 0 },
  { W_C89_NO_FLEXARRAY, ERROR_LEVEL_PEDANTIC,
     "ISO C90 does not support flexible array members", 0 },
  { E_FLEXARRAY_NOTATEND, ERROR_LEVEL_ERROR,
     "flexible array member '%s' not at end of struct", 0 },
  { E_FLEXARRAY_INEMPTYSTRCT, ERROR_LEVEL_ERROR,
     "flexible array '%s' in otherwise empty struct", 0 },
  { W_EMPTY_SOURCE_FILE, ERROR_LEVEL_PEDANTIC,
     "ISO C forbids an empty source file", 0 },
  { W_BAD_PRAGMA_ARGUMENTS, ERROR_LEVEL_WARNING,
     "#pragma %s: bad argument(s); pragma ignored", 0 },
  { E_BAD_RESTRICT, ERROR_LEVEL_ERROR,
     "Only object pointers may be qualified with 'restrict'", 0 },
  { E_BAD_INLINE, ERROR_LEVEL_ERROR,
     "Only functions may be qualified with 'inline'", 0 },
  { E_BAD_INT_ARGUMENT, ERROR_LEVEL_ERROR,
     "Bad integer argument for option %s", 0 },
  { E_NEGATIVE_ARRAY_SIZE, ERROR_LEVEL_ERROR,
     "Size of array '%s' is negative", 0 },
  { W_TARGET_LOST_QUALIFIER, ERROR_LEVEL_WARNING,
     "pointer target lost %s qualifier", 0 },
  { W_DEPRECATED_KEYWORD, ERROR_LEVEL_WARNING,
     "keyword '%s' is deprecated, use '%s' instead", 0 },
  { E_STORAGE_CLASS_FOR_PARAMETER, ERROR_LEVEL_ERROR,
     "storage class other than register specified for parameter '%s'", 0 },
  { E_OFFSETOF_TYPE, ERROR_LEVEL_ERROR,
     "offsetof can only be applied to structs/unions", 0 },
  { E_INCOMPLETE_FIELD, ERROR_LEVEL_ERROR,
     "field '%s' has incomplete type", 0 },
  { W_DEPRECATED_OPTION, ERROR_LEVEL_WARNING,
     "deprecated compiler option '%s'", 0 },
  { E_BAD_DESIGNATOR, ERROR_LEVEL_ERROR,
     "Invalid designator for designated initializer", 0 },
  { W_DUPLICATE_INIT, ERROR_LEVEL_WARNING,
     "Duplicate initializer at position %d; ignoring previous.", 0 },
  { E_INVALID_UNIVERSAL, ERROR_LEVEL_ERROR,
     "invalid universal character name \\%s.", 0 },
  { W_UNIVERSAL_C95, ERROR_LEVEL_WARNING,
     "universal character names are only valid in C95 or later", 0 },
  { E_SHORTLONG, ERROR_LEVEL_ERROR,
     "invalid combination of short / long", 0 },
  { E_INTEGERSUFFIX, ERROR_LEVEL_ERROR,
     "Invalid integer suffix '%s' in integer constant", 0},
  { E_AUTO_ADDRSPACE, ERROR_LEVEL_ERROR,
     "named address space not allowed for automatic var '%s'", 0},
  { W_NORETURNRETURN, ERROR_LEVEL_WARNING,
     "return in _Noreturn function", 0},
  { E_STRUCT_REDEF, ERROR_LEVEL_ERROR,
     "struct/union '%s' redefined", 0 },
  { W_STRING_CANNOT_BE_TERMINATED, ERROR_LEVEL_PEDANTIC,
    "string '%s'cannot be terminated within array", 0 },
  { W_LONGLONG_LITERAL, ERROR_LEVEL_WARNING,
    "support for large long long literals is incomplete", 1 },
  { S_SYNTAX_ERROR, ERROR_LEVEL_SYNTAX_ERROR,
    "token -> '%s' ; column %d", 0 },
  { E_MIXING_CONFIG, ERROR_LEVEL_ERROR,
    "mixing __CONFIG and CONFIG directives", 0 },
  { W_STATIC_ASSERTION, ERROR_LEVEL_WARNING,
    "static assertion failed: %s", 0 },
  { E_ALIGNAS, ERROR_LEVEL_ERROR,
    "invalid alignment specified: %d", 0 },
  { W_INTERNAL_ERROR, ERROR_LEVEL_WARNING,
     "Non-fatal Compiler Internal Problem in file '%s' line number '%d' : %s \n"
     "Contact Author with source code", 0 },
  { W_UNRECOGNIZED_ASM, ERROR_LEVEL_INFO,
     "%s() failed to parse line node, assuming %d bytes\n'%s'\n", 0 },
  { W_FLEXARRAY_INSTRUCT, ERROR_LEVEL_WARNING,
     "type of variable '%s' is struct with flexible array field", 0},
  { E_TYPE_IS_FUNCTION, ERROR_LEVEL_ERROR,
     "'%s' has function type", 0},
  { W_INLINE_NAKED, ERROR_LEVEL_WARNING,
     "inline function '%s' is __naked", 0},
  { E_Z88DK_FASTCALL_PARAMETERS, ERROR_LEVEL_ERROR,
    "invalid number of parameters for __z88dk_fastcall", 0 },
  { E_Z88DK_FASTCALL_PARAMETER, ERROR_LEVEL_ERROR,
    "ivalid parameter type in __z88dk_fastcall", 0 },
  { W_REPEAT_QUALIFIER, ERROR_LEVEL_WARNING,
    "duplicate specifier '%s'", 0 },
  { W_NO_TYPE_SPECIFIER, ERROR_LEVEL_WARNING,
    "no type specifier for '%s'", 0 },
  { E_NO_TYPE_SPECIFIER, ERROR_LEVEL_ERROR,
    "no type specifier for '%s'", 0 },
  { E_MULTIPLE_DEFAULT_IN_GENERIC, ERROR_LEVEL_ERROR,
    "multiple default expressions in generic association", 0 },
  { E_MULTIPLE_MATCHES_IN_GENERIC, ERROR_LEVEL_ERROR,
    "multiple matching expressions in generic association", 0 },
  { E_NO_MATCH_IN_GENERIC, ERROR_LEVEL_ERROR,
    "no matching expression in generic association", 0 },
  { W_LABEL_WITHOUT_STATEMENT, ERROR_LEVEL_WARNING,
    "label without statement", 0},
  { E_WCHAR_CONST_C95, ERROR_LEVEL_ERROR,
    "character constant of type wchar_t requires C95 or later", 0},
  { E_WCHAR_CONST_C11, ERROR_LEVEL_ERROR,
    "character constant of type char16_t or char32_t requires C11 or later", 0},
  { E_WCHAR_STRING_C95, ERROR_LEVEL_ERROR,
    "wide character string of type L requires C95 or later", 0},
  { E_WCHAR_STRING_C11, ERROR_LEVEL_ERROR,
    "wide character string of type u8, u, U requires C11 or later", 0},
  { W_UNKNOWN_REG, ERROR_LEVEL_WARNING,
    "unknown register specification %s", 0},
  { E_HEXFLOAT_C99, ERROR_LEVEL_ERROR,
    "hexadecimal floating constant requires ISO C99 or later", 0},
  { E_ANONYMOUS_STRUCT_TAG, ERROR_LEVEL_ERROR,
    "tagged anonymous struct/union '%s'", 0},
  { W_INLINE_FUNCATTR, ERROR_LEVEL_WARNING,
    "inline function '%s' might lose function attributes", 0},
  { E_FOR_INITAL_DECLARATION_C99, ERROR_LEVEL_ERROR,
    "initial declaration in for loop requires ISO C99 or later", 0},
  { E_QUALIFIED_ARRAY_PARAM_C99, ERROR_LEVEL_ERROR,
    "qualifiers in array parameters require ISO C99 or later", 0},
  { E_QUALIFIED_ARRAY_NOPARAM, ERROR_LEVEL_ERROR,
    "qualifier or static in array declarator that is not a parameter", 0},
  { E_STATIC_ARRAY_PARAM_C99, ERROR_LEVEL_ERROR,
    "static in array parameters requires ISO C99 or later", 0},
  { E_INT_MULTIPLE, ERROR_LEVEL_ERROR,
    "multiple interrupt numbers for '%s'", 0},
  { W_INCOMPAT_PTYPES, ERROR_LEVEL_WARNING,
     "pointer types incompatible ", 0 },
  { E_STATIC_ASSERTION_C2X, ERROR_LEVEL_ERROR,
    "static assertion with one argument requires C2X or later", 0 },
  { W_STATIC_ASSERTION_2, ERROR_LEVEL_WARNING,
    "static assertion failed", 0 },
  { E_DECL_AFTER_STATEMENT_C99, ERROR_LEVEL_ERROR,
    "declaration after statement requires ISO C99 or later", 0 },
  { E_SHORTCALL_INVALID_VALUE, ERROR_LEVEL_ERROR,
    "invalid value for __z88dk_shortcall %s parameter: %x", 0},
  { E_DUPLICATE_PARAMTER_NAME, ERROR_LEVEL_ERROR,
    "duplicate parameter name %s for function %s", 0},
  { E_AUTO_FILE_SCOPE, ERROR_LEVEL_ERROR,
    "auto in declaration at file scope", 0},
  { E_U8_CHAR_C2X, ERROR_LEVEL_ERROR,
    "u8 character constant requires ISO C2X or later", 0},
  { E_U8_CHAR_INVALID, ERROR_LEVEL_ERROR,
    "invalid u8 character constant", 0},
  { E_ATTRIBUTE_C2X, ERROR_LEVEL_ERROR,
    "attribute requires C2X or later", 0},
  { E_COMPOUND_LITERALS_C99, ERROR_LEVEL_ERROR,
    "compound literals require ISO C99 or later and are not implemented", 0},
};

/* -------------------------------------------------------------------------------
 * SetErrorOut - Set the error output file
 * -------------------------------------------------------------------------------
 */
FILE *
SetErrorOut (FILE *NewErrorOut)
{
  _SDCCERRG.out = NewErrorOut;

  return NewErrorOut;
}

/* -------------------------------------------------------------------------------
 * setErrorLogLevel - Set the error log level:
 *                    which level has to be treated as an error
 * -------------------------------------------------------------------------------
 */
void setErrorLogLevel (ERROR_LOG_LEVEL level)
{
  _SDCCERRG.logLevel = level;
}

/* -------------------------------------------------------------------------------
 * vwerror - Output a standard error message with variable number of arguments
 * -------------------------------------------------------------------------------
 */
int
vwerror (int errNum, va_list marker)
{
  if (_SDCCERRG.out == NULL)
    {
      _SDCCERRG.out = DEFAULT_ERROR_OUT;
    }

  if (errNum > NELEM (ErrTab))
    {
      fprintf (_SDCCERRG.out,
              "Internal error: bad error number %d.", errNum);
      return 0;
    }
  if (NELEM (ErrTab) != NUMBER_OF_ERROR_MESSAGES || ErrTab[errNum].errIndex != errNum)
    {
      fprintf (_SDCCERRG.out,
              "Internal error: error table entry for %d inconsistent.", errNum);
      return 0;
    }

  if ((ErrTab[errNum].errType >= _SDCCERRG.logLevel) && (!ErrTab[errNum].disabled))
    {
      if (ErrTab[errNum].errType >= ERROR_LEVEL_ERROR || _SDCCERRG.werror)
        fatalError++;

      if (filename && lineno)
        {
          if (_SDCCERRG.style)
            fprintf (_SDCCERRG.out, "%s(%d) : ", filename, lineno);
          else
            fprintf (_SDCCERRG.out, "%s:%d: ", filename, lineno);
        }
      else if (lineno)
        {
          fprintf (_SDCCERRG.out, "at %d: ", lineno);
        }
      else
        {
          fprintf (_SDCCERRG.out, "-:0: ");
        }

      switch (ErrTab[errNum].errType)
        {
        case ERROR_LEVEL_SYNTAX_ERROR:
          fprintf (_SDCCERRG.out, "syntax error: ");
          break;

        case ERROR_LEVEL_ERROR:
          fprintf (_SDCCERRG.out, "error %d: ", errNum);
          break;

        case ERROR_LEVEL_WARNING:
        case ERROR_LEVEL_PEDANTIC:
          if (_SDCCERRG.werror)
            fprintf (_SDCCERRG.out, "error %d: ", errNum);
          else
            fprintf (_SDCCERRG.out, "warning %d: ", errNum);
          break;

        case ERROR_LEVEL_INFO:
          fprintf (_SDCCERRG.out, "info %d: ", errNum);
          break;

        default:
          break;
        }

      vfprintf (_SDCCERRG.out, ErrTab[errNum].errText, marker);
      fprintf (_SDCCERRG.out, "\n");
      return 1;
    }
  else
    {
      /* Below the logging level, drop. */
      return 0;
    }
}

/* -------------------------------------------------------------------------------
 * werror - Output a standard error message with variable number of arguments
 * -------------------------------------------------------------------------------
 */
int
werror (int errNum, ...)
{
  int ret;
  va_list marker;
  va_start (marker, errNum);
  ret = vwerror (errNum, marker);
  va_end (marker);
  return ret;
}

/* -------------------------------------------------------------------------------
werror_bt - like werror(), but als provide a backtrace
 * -------------------------------------------------------------------------------
 */
int
werror_bt (int errNum, ...)
{
#ifdef HAVE_BACKTRACE_SYMBOLS_FD
  void *callstack[16];
  int frames = backtrace (callstack, 16);
  fprintf (stderr, "Backtrace:\n");
  backtrace_symbols_fd (callstack, frames, STDERR_FILENO);
#endif

  int ret;
  va_list marker;
  va_start (marker, errNum);
  ret = vwerror (errNum, marker);
  va_end (marker);

  return ret;
}

/* -------------------------------------------------------------------------------
 * werrorfl - Output a standard error message with variable number of arguments.
 *            Use a specified filename and line number instead of the default.
 * -------------------------------------------------------------------------------
 */
int
werrorfl (char *newFilename, int newLineno, int errNum, ...)
{
  char *oldFilename = filename;
  int oldLineno = lineno;
  va_list marker;
  int ret;

  filename = newFilename;
  lineno = newLineno;

  va_start (marker,errNum);
  ret = vwerror (errNum, marker);
  va_end (marker);

  filename = oldFilename;
  lineno = oldLineno;
  return ret;
}

/* -------------------------------------------------------------------------------
 * fatal - Output a standard error message with variable number of arguments and
 *         call exit()
 * -------------------------------------------------------------------------------
 */
void
fatal (int exitCode, int errNum, ...)
{
  va_list marker;
  va_start (marker, errNum);
  vwerror (errNum, marker);
  va_end (marker);

  exit (exitCode);
}

/* -------------------------------------------------------------------------------
 * style - Change the output error style to MSVC
 * -------------------------------------------------------------------------------
 */
void
MSVC_style (int style)
{
  _SDCCERRG.style = style;
}

/* -------------------------------------------------------------------------------
 * disabled - Disable output of specified warning
 * -------------------------------------------------------------------------------
 */
void
setWarningDisabled (int errNum)
{
  if ((errNum >= 0) && (errNum < NELEM (ErrTab)) && (ErrTab[errNum].errType <= ERROR_LEVEL_WARNING))
    ErrTab[errNum].disabled = 1;
}

/* -------------------------------------------------------------------------------
 * disabledState - Enable/Disable output of specified warning
 * -------------------------------------------------------------------------------
 */
int
setWarningDisabledState (int errNum, int disabled)
{
  if ((errNum >= 0) && (errNum < NELEM (ErrTab)) && (ErrTab[errNum].errType <= ERROR_LEVEL_WARNING))
  {
    int originalState = ErrTab[errNum].disabled;
    ErrTab[errNum].disabled = disabled;
    return originalState;
  }
  return 0;
}


/* -------------------------------------------------------------------------------
 * Set the flag to treat warnings as errors
 * -------------------------------------------------------------------------------
 */
void
setWError (int flag)
{
  _SDCCERRG.werror = flag;
}
