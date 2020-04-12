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

#if !defined(__SDCCERR_H)

#define __SDCCERR_H

#include <stdio.h>
#include <stdarg.h>

/* ERROR Message Definition */

enum {
  E_DUPLICATE                   =   0, /* Duplicate variable   */
  E_SYNTAX_ERROR                =   1, /* Syntax Error         */
  E_CONST_EXPECTED              =   2, /* constant expected    */
  E_OUT_OF_MEM                  =   3, /* malloc failed        */
  E_FILE_OPEN_ERR               =   4, /* File open failed     */
  E_INVALID_OCLASS              =   5, /* output class invalid */
  E_CANNOT_ALLOC                =   6, /* cannot allocate space*/
  E_OLD_STYLE                   =   7, /* old style C ! allowed*/
  E_STACK_OUT                   =   8, /* v r out of stack     */
  E_INTERNAL_ERROR              =   9, /* unable to alloc tvar */
  E_LVALUE_REQUIRED             =  10, /* lvalue required      */
  E_TMPFILE_FAILED              =  11, /* tmpfile creation failed */
  E_FUNCTION_EXPECTED           =  12, /* function expected    */
  E_USING_ERROR                 =  13, /* using in error       */
  E_SFR_INIT                    =  14, /* init error for sbit  */
  W_INIT_IGNORED                =  15, /* initialiser ignored  */
  E_AUTO_ASSUMED                =  16, /* sclass auto assumed  */
  E_AUTO_ABSA                   =  17, /* abs addr for auto var*/
  W_INIT_WRONG                  =  18, /* initializer type !=  */
  E_FUNC_REDEF                  =  19, /* func name redefined  */
  E_ID_UNDEF                    =  20, /* identifer undefined  */
  W_STACK_OVERFLOW              =  21, /* stack overflow       */
  E_NEED_ARRAY_PTR              =  22, /* array or pointer reqd*/
  E_IDX_NOT_INT                 =  23, /* index not an integer */
  W_IDX_OUT_OF_BOUNDS           =  24, /* array index out of bounds */
  E_STRUCT_UNION                =  25, /* struct,union expected*/
  E_NOT_MEMBER                  =  26, /* !struct/union member */
  E_PTR_REQD                    =  27, /* pointer required     */
  E_UNARY_OP                    =  28, /* unary operator bad op*/
  E_CONV_ERR                    =  29, /* conversion error     */
  E_BITFLD_TYPE                 =  30, /* invalid type for bit-field */
  E_BITFLD_SIZE                 =  31, /* bit-field too wide for type */
  W_TRUNCATION                  =  32, /* high order trucation */
  E_CODE_WRITE                  =  33, /* trying 2 write to code */
  E_LVALUE_CONST                =  34, /* lvalue is a const   */
  E_ILLEGAL_ADDR                =  35, /* address of bit      */
  E_CAST_ILLEGAL                =  36, /* cast illegal        */
  E_MULT_INTEGRAL               =  37, /* mult opernd must b integral */
  E_ARG_ERROR                   =  38, /* argument count error*/
  E_ARG_COUNT                   =  39, /* func expecting more */
  E_FUNC_EXPECTED               =  40, /* func name expected  */
  E_PLUS_INVALID                =  41, /* plus invalid        */
  E_PTR_PLUS_PTR                =  42, /* pointer + pointer   */
  E_SHIFT_OP_INVALID            =  43, /* shft op op invalid  */
  E_COMPARE_OP                  =  44, /* compare operand     */
  E_BITWISE_OP                  =  45, /* bit op invalid op   */
  E_ANDOR_OP                    =  46, /* && || op invalid    */
  E_TYPE_MISMATCH               =  47, /* type mismatch       */
  E_ARRAY_ASSIGN                =  48, /* array assign        */
  E_ARRAY_DIRECT                =  49, /* array indexing in   */
  E_BIT_ARRAY                   =  50, /* bit array not allowed  */
  E_DUPLICATE_TYPEDEF           =  51, /* typedef name duplicate */
  E_ARG_TYPE                    =  52, /* arg type mismatch   */
  E_RET_VALUE                   =  53, /* return value mismatch */
  E_FUNC_AGGR                   =  54, /* function returing aggr */
  E_FUNC_DEF                    =  55, /* ANSI Style def neede */
  E_DUPLICATE_LABEL             =  56, /* duplicate label name */
  E_LABEL_UNDEF                 =  57, /* undefined label used */
  E_FUNC_VOID                   =  58, /* void func ret value  */
  W_VOID_FUNC                   =  59, /* func must return value */
  W_RETURN_MISMATCH             =  60, /* return value mismatch */
  E_CASE_CONTEXT                =  61, /* case stmnt without switch */
  E_CASE_CONSTANT               =  62, /* case expression ! const */
  E_BREAK_CONTEXT               =  63, /* break statement invalid */
  E_SWITCH_AGGR                 =  64, /* non integral for switch */
  E_FUNC_BODY                   =  65, /* func has body already */
  E_UNKNOWN_SIZE                =  66, /* variable has unknown size */
  E_AUTO_AGGR_INIT              =  67, /* auto aggregates no init */
  E_INIT_COUNT                  =  68, /* too many initializers */
  E_INIT_STRUCT                 =  69, /* struct init wrong   */
  E_INIT_NON_ADDR               =  70, /* non address xpr for init */
  E_INT_DEFINED                 =  71, /* interrupt already over */
  E_INT_ARGS                    =  72, /* interrupt rtn cannot have args */
  E_INCLUDE_MISSING             =  73, /* compiler include missing */
  E_NO_MAIN                     =  74, /* main function undefined */
  E_UNUSED_75                   =  75, /* - removed error code - */
  E_PRE_PROC_FAILED             =  76, /* preprocessor failed */
  E_DUP_FAILED                  =  77, /* file DUP failed     */
  E_INCOMPAT_TYPES              =  78, /* incompatible types casting */
  W_LOOP_ELIMINATE              =  79, /* loop eliminated     */
  W_NO_SIDE_EFFECTS             =  80, /* expression has no side effects */
  W_CONST_TOO_LARGE             =  81, /* constant out of range */
  W_BAD_COMPARE                 =  82, /* bad comparison      */
  E_TERMINATING                 =  83, /* compiler terminating */
  W_LOCAL_NOINIT                =  84, /* local reference before assignment */
  W_NO_REFERENCE                =  85, /* no reference to local variable */
  E_OP_UNKNOWN_SIZE             =  86, /* unknown size for operand */
  W_LONG_UNSUPPORTED            =  87, /* 'long' not supported yet */
  W_LITERAL_GENERIC             =  88, /* literal being cast to generic pointer */
  E_SFR_ADDR_RANGE              =  89, /* sfr address out of range */
  E_BITVAR_STORAGE              =  90, /* storage given for 'bit' variable */
  E_EXTERN_MISMATCH             =  91, /* extern declaration mismatches */
  E_NONRENT_ARGS                =  92, /* fptr non reentrant has args */
  W_DOUBLE_UNSUPPORTED          =  93, /* 'double' not supported yet */
  W_COMP_RANGE                  =  94, /* comparison is always %s due to limited range of data type */
  W_FUNC_NO_RETURN              =  95, /* no return statement found */
  W_PRE_PROC_WARNING            =  96, /* preprocessor generated warning */
  E_STRUCT_AS_ARG               =  97, /* structure passed as argument */
  E_PREV_DECL_CONFLICT          =  98, /* previous declaration conflicts with current */
  E_CODE_NO_INIT                =  99, /* vars in code space must have initializer */
  E_OPS_INTEGRAL                = 100, /* operans must be integral for certain assignments */
  E_TOO_MANY_PARMS              = 101, /* too many parameters */
  E_TOO_FEW_PARMS               = 102, /* too few parameters  */
  E_FUNC_NO_CODE                = 103, /* fatalError          */
  E_TYPE_MISMATCH_PARM          = 104, /* type mismatch for parameter */
  E_INVALID_FLOAT_CONST         = 105, /* invalid floating point literal string */
  E_INVALID_OP                  = 106, /* invalid operand for some operation */
  E_SWITCH_NON_INTEGER          = 107, /* switch value not integer */
  E_CASE_NON_INTEGER            = 108, /* case value not integer */
  W_FUNC_TOO_LARGE              = 109, /* function too large  */
  W_CONTROL_FLOW                = 110, /* control flow changed due to optimization */
  W_PTR_TYPE_INVALID            = 111, /* invalid type specifier for pointer */
  W_IMPLICIT_FUNC               = 112, /* function declared implicitly */
  W_CONTINUE                    = 113, /* more than one line  */
  I_EXTENDED_STACK_SPILS        = 114, /* too many spils occured */
  W_UNKNOWN_PRAGMA              = 115, /* #pragma directive unsupported */
  W_SHIFT_CHANGED               = 116, /* shift changed to zero */
  W_UNKNOWN_OPTION              = 117, /* don't know the option */
  W_UNSUPP_OPTION               = 118, /* processor reset has been redifned */
  E_UNKNOWN_FEXT                = 119, /* unknown file extension */
  W_TOO_MANY_SRC                = 120, /* can only compile one .c file at a time */
  I_CYCLOMATIC                  = 121, /* information message */
  E_DIVIDE_BY_ZERO              = 122, /* / 0 */
  E_FUNC_BIT                    = 123, /* function cannot return bit */
  E_CAST_ZERO                   = 124, /* casting to from size zero */
  W_CONST_RANGE                 = 125, /* constant too large  */
  W_CODE_UNREACH                = 126, /* unreachable code    */
  W_NONPTR2_GENPTR              = 127, /* non pointer cast to generic pointer */
  W_POSSBUG                     = 128, /* possible code generation error */
  E_INCOMPAT_PTYPES             = 129, /* incompatible pointer assignment */
  W_UNKNOWN_MODEL               = 130, /* Unknown memory model */
  E_UNKNOWN_TARGET              = 131, /* target not defined  */
  W_INDIR_BANKED                = 132, /* Indirect call to a banked fun */
  W_UNSUPPORTED_MODEL           = 133, /* Unsupported model, ignored */
  W_BANKED_WITH_NONBANKED       = 134, /* banked and nonbanked attributes mixed */
  W_BANKED_WITH_STATIC          = 135, /* banked and static mixed */
  W_INT_TO_GEN_PTR_CAST         = 136, /* Converting integer type to generic pointer. */
  W_ESC_SEQ_OOR_FOR_CHAR        = 137, /* Escape sequence of of range for char */
  E_INVALID_HEX                 = 138, /* \x used with no following hex digits */
  W_FUNCPTR_IN_USING_ISR        = 139, /* Call via function pointer in ISR with using attribute. */
  E_NO_SUCH_BANK                = 140, /* 'using' attribute specifies non-existant register bank. */
  E_TWO_OR_MORE_DATA_TYPES      = 141, /* two or more data types in declaration for '%s' */
  E_LONG_OR_SHORT_INVALID       = 142, /* long or short invalid for .. */
  E_SIGNED_OR_UNSIGNED_INVALID  = 143, /* signed or unsigned invalid for .. */
  E_LONG_AND_SHORT_INVALID      = 144, /* long and short invalid for .. */
  E_SIGNED_AND_UNSIGNED_INVALID = 145, /* signed and unsigned invalid for .. */
  E_TWO_OR_MORE_STORAGE_CLASSES = 146, /* two or more storage classes in declaration for '%s' */
  W_EXCESS_INITIALIZERS         = 147, /* too much initializers for array */
  E_ARGUMENT_MISSING            = 148, /* Option requires an argument. */
  W_STRAY_BACKSLASH             = 149, /* stray '\\' at column %d" */
  W_NEWLINE_IN_STRING           = 150, /* newline in string constant */
  W_USING_GENERIC_POINTER       = 151, /* using generic pointer %s to initialize %s */
  W_EXCESS_SHORT_OPTIONS        = 152, /* Only one short option can be specified at a time.  Rest of %s ignored. */
  E_VOID_VALUE_USED             = 153, /* void value not ignored as it ought to be */
  W_INTEGRAL2PTR_NOCAST         = 154, /* converting integral to pointer without a cast */
  W_PTR2INTEGRAL_NOCAST         = 155, /* converting pointer to integral without a cast */
  W_SYMBOL_NAME_TOO_LONG        = 156, /* symbol name too long, truncated to %d chars */
  W_CAST_STRUCT_PTR             = 157, /* pointer to different structure types */
  W_LIT_OVERFLOW                = 158, /* overflow in implicit constant conversion */
  E_PARAM_NAME_OMITTED          = 159, /* { in function %s: name omitted for parameter %d */
  W_NO_FILE_ARG_IN_C1           = 160, /* only standard input is compiled in c1 mode. file '%s' ignored */
  E_NEED_OPT_O_IN_C1            = 161, /* must specify assembler file name with -o in c1 mode */
  W_ILLEGAL_OPT_COMBINATION     = 162, /* illegal combination of options (--c1mode, -E, -S -c) */
  E_DUPLICATE_MEMBER            = 163, /* duplicate %s member '%s' */
  E_STACK_VIOLATION             = 164, /* internal stack violation */
  W_INT_OVL                     = 165, /* integer overflow in expression */
  W_USELESS_DECL                = 166, /* useless declaration */
  E_INT_BAD_INTNO               = 167, /* invalid interrupt number */
  W_BITFLD_NAMED                = 168, /* declarator used with 0 length bit-field */
  E_FUNC_ATTR                   = 169, /* function attribute without function */
  W_SAVE_RESTORE                = 170, /* unmatched #pragma SAVE and #pragma RESTORE */
  E_INVALID_CRITICAL            = 171, /* operation invalid in critical sequence */
  E_NOT_ALLOWED                 = 172, /* %s not allowed here */
  E_BAD_TAG                     = 173, /* '%s' is not a %s tag */
  E_ENUM_NON_INTEGER            = 174, /* enumeration constant not an integer */
  W_DEPRECATED_PRAGMA           = 175, /* deprecated pragma */
  E_SIZEOF_INCOMPLETE_TYPE      = 176, /* sizeof applied to an incomplete type */
  E_PREVIOUS_DEF                = 177, /* previously defined here */
  W_SIZEOF_VOID                 = 178, /* size of void is zero */
  W_POSSBUG2                    = 179, /* possible bug, new format */
  W_COMPLEMENT                  = 180, /* ~bit can give unexpected results */
  E_SHADOWREGS_NO_ISR           = 181, /* shadowregs keyword following non-ISR function */
  W_SFR_ABSRANGE                = 182, /* sfr at address out of range */
  E_BANKED_WITH_CALLEESAVES     = 183, /* banked and callee-saves mixed */
  W_INVALID_INT_CONST           = 184, /* invalid integer literal string */
  W_CMP_SU_CHAR                 = 185, /* comparison of 'signed char' with 'unsigned char' requires promotion to int */
  W_INVALID_FLEXARRAY           = 186, /* invalid use of structure with flexible array member */
  W_C89_NO_FLEXARRAY            = 187, /* ISO C90 does not support flexible array members */
  E_FLEXARRAY_NOTATEND          = 188, /* flexible array member not at end of struct */
  E_FLEXARRAY_INEMPTYSTRCT      = 189, /* flexible array in otherwise empty struct */
  W_EMPTY_SOURCE_FILE           = 190, /* ISO C forbids an empty source file */
  W_BAD_PRAGMA_ARGUMENTS        = 191, /* #pragma %s: bad argument(s); pragma ignored */
  E_BAD_RESTRICT                = 192, /* Only object pointers may be qualified with 'restrict' */
  E_BAD_INLINE                  = 193, /* Only functions may be qualified with 'inline' */
  E_BAD_INT_ARGUMENT            = 194, /* Bad integer option argument */
  E_NEGATIVE_ARRAY_SIZE         = 195, /* Size of array '%s' is negative */
  W_TARGET_LOST_QUALIFIER       = 196, /* Pointer target lost qualifier */
  W_DEPRECATED_KEYWORD          = 197, /* keyword '%s' is deprecated, use '%s' instead */
  E_STORAGE_CLASS_FOR_PARAMETER = 198, /* storage class specified for parameter '%s' */
  E_OFFSETOF_TYPE               = 199, /* offsetof can only be applied to structs/unions */
  E_INCOMPLETE_FIELD            = 200, /* struct field has incomplete type */
  W_DEPRECATED_OPTION           = 201, /* deprecated compiler option '%s' */
  E_BAD_DESIGNATOR              = 202, /* Bad designated initializer */
  W_DUPLICATE_INIT              = 203, /* duplicate initializer */
  E_INVALID_UNIVERSAL           = 204, /* invalid universal character name %s. */
  W_UNIVERSAL_C95               = 205, /* universal character names are only valid in C95 or later */
  E_SHORTLONG                   = 206, /* Invalid combination of short / long */
  E_INTEGERSUFFIX               = 207, /* Invalid integer suffix */
  E_AUTO_ADDRSPACE              = 208, /* named address space for auto var */
  W_NORETURNRETURN              = 209, /* return in _noreturn function */
  E_STRUCT_REDEF                = 210, /* struct or union tag redefined */
  W_STRING_CANNOT_BE_TERMINATED = 211, /* string cannot be terminated within array */
  W_LONGLONG_LITERAL            = 212, /* long long literal */
  S_SYNTAX_ERROR                = 213, /* syntax error */
  E_MIXING_CONFIG               = 214, /* mixing __CONFIG and CONFIG directives */
  W_STATIC_ASSERTION            = 215, /* static assertion failed */
  E_ALIGNAS                     = 216, /* invalid alignment specified */
  W_INTERNAL_ERROR              = 217, /* warning for non-fatal internal errors - things that should not have happened, but can be handled */
  W_UNRECOGNIZED_ASM            = 218, /* unrecognized asm instruction in peephole optimizer */
  W_FLEXARRAY_INSTRUCT          = 219, /* using flexible arrays in a struct */
  E_TYPE_IS_FUNCTION            = 220, /* typedef void foo_t(void); foo_t foo; */
  W_INLINE_NAKED                = 221, /* inline function is naked */
  E_Z88DK_FASTCALL_PARAMETERS   = 222, /* invalid number of parameters in __z88dk_fastcall */
  E_Z88DK_FASTCALL_PARAMETER    = 223, /* invalid parameter type in __z88dk_fastcall */
  W_REPEAT_QUALIFIER            = 224, /* the same qualifier appears more than once */
  W_NO_TYPE_SPECIFIER           = 225, /* type specifier missing in declaration */
  E_NO_TYPE_SPECIFIER           = 226, /* type specifier missing in declaration */
  E_MULTIPLE_DEFAULT_IN_GENERIC = 227, /* multiple default expressions in generic association */
  E_MULTIPLE_MATCHES_IN_GENERIC = 228, /* multiple matching expressions in generic association */
  E_NO_MATCH_IN_GENERIC         = 229, /* no matching expression in generic association */
  W_LABEL_WITHOUT_STATEMENT     = 230, /* label without statement, not allowed in standard C */
  E_WCHAR_CONST_C95             = 231, /* character constant of type wchar_t requires ISO C 95 or later */
  E_WCHAR_CONST_C11             = 232, /* character constant of type char16_t or char32_t equires ISO C 11 or later */
  E_WCHAR_STRING_C95            = 233, /* wide character string literal requires ISO C 95 or later */
  E_WCHAR_STRING_C11            = 234, /* wide character string literal requires ISO C 11 or later */
  W_UNKNOWN_REG                 = 235, /* unknown register specified */
  E_HEXFLOAT_C99                = 236, /* hexadecimal floating constant requires ISO C99 or later */
  E_ANONYMOUS_STRUCT_TAG        = 237, /* anonymous struct/union should not have a tag */
  W_INLINE_FUNCATTR             = 238, /* inline functions should not be z88dk_fastcall or z88dk_callee */
  E_FOR_INITAL_DECLARATION_C99  = 239, /* initial declaration in for loop requires ISO C99 or later */
  E_QUALIFIED_ARRAY_PARAM_C99   = 240, /* qualifiers in array parameters require ISO C99 or later */
  E_QUALIFIED_ARRAY_NOPARAM     = 241, /* qualifier or static in array declarator that is not a parameter */
  E_STATIC_ARRAY_PARAM_C99      = 242, /* static in array parameters requires ISO C99 or later */
  E_INT_MULTIPLE                = 243, /* multiple interrupt numbers */
  W_INCOMPAT_PTYPES             = 244, /* incompatible pointer assignment (not allowed by the standard, but allowed in SDCC) */
  E_STATIC_ASSERTION_C2X        = 245, /* static assertion with one argument requires C2X or later */
  W_STATIC_ASSERTION_2          = 246, /* static assertion failed */
  E_DECL_AFTER_STATEMENT_C99    = 247, /* declaration after statement requires ISO C99 or later */
  E_SHORTCALL_INVALID_VALUE     = 248, /* Invalid value for a __z88dk_shortcall specifier */
  E_DUPLICATE_PARAMTER_NAME     = 249, /* duplicate parameter name */
  E_AUTO_FILE_SCOPE             = 250, /* auto in declaration at file scope */
  E_U8_CHAR_C2X                 = 251, /* u8 character constant requires ISO C2X or later */
  E_U8_CHAR_INVALID             = 252, /* invalid u8 character constant */
  E_ATTRIBUTE_C2X               = 253, /* attribute requires ISO C2X or later */
  E_COMPOUND_LITERALS_C99       = 254, /* compound literals require ISO C99 or later */

  /* don't touch this! */
  NUMBER_OF_ERROR_MESSAGES             /* Number of error messages */
};

/** sdcc style assertion */
#ifdef assert
#undef assert
#endif
#ifdef NDEBUG
# define assert(expr) (void)0
#else
# define assert(expr) ((expr) ? (void)0 : fatal (1, E_INTERNAL_ERROR, __FILE__, __LINE__, #expr))
#endif

#define wassertl_bt(a,s)   (void)((a) ? 0 : \
        (werror_bt (E_INTERNAL_ERROR, __FILE__, __LINE__, s), 0))

#define wassert_bt(a) wassertl_bt(a, "code generator internal error")

/** Describes the maximum error level that will be logged.  Any level
 *  includes all of the levels listed after it.
 *
 *
 */
enum _ERROR_LOG_LEVEL {
  /** Everything.  Currently the same as PEDANTIC. */
  ERROR_LEVEL_ALL,
  /** All warnings, including those considered 'reasonable to use,
      on occasion, in clean programs' (man 3 gcc). */
  ERROR_LEVEL_PEDANTIC,
  /** 'informational' warnings */
  ERROR_LEVEL_INFO,
  /** Most warnings. */
  ERROR_LEVEL_WARNING,
  /** Errors only. */
  ERROR_LEVEL_ERROR,
  /** Syntax error only. */
  ERROR_LEVEL_SYNTAX_ERROR
};

typedef enum _ERROR_LOG_LEVEL ERROR_LOG_LEVEL;

struct SDCCERRG {
  ERROR_LOG_LEVEL logLevel;
  FILE *out;
  int style;                        /* 1=MSVC */
  int werror;                       /* treat the warnings as errors */
};

extern struct SDCCERRG _SDCCERRG;

/** Sets the maximum error level to log.
    See MAX_ERROR_LEVEL.  The default is ERROR_LEVEL_ALL.
*/
void setErrorLogLevel (ERROR_LOG_LEVEL level);

/*
-------------------------------------------------------------------------------
SetErrorOut - Set the error output file

-------------------------------------------------------------------------------
*/

FILE * SetErrorOut (FILE *NewErrorOut);

/*
-------------------------------------------------------------------------------
vwerror - Output a standard eror message with variable number of arguements

-------------------------------------------------------------------------------
*/

int vwerror (int errNum, va_list marker);

/*
-------------------------------------------------------------------------------
werror - Output a standard eror message with variable number of arguements

-------------------------------------------------------------------------------
*/

int werror (int errNum, ... );

/*
-------------------------------------------------------------------------------
werror_bt - like werror(), but als provide a backtrace

-------------------------------------------------------------------------------
*/

int werror_bt (int errNum, ... );

/*
-------------------------------------------------------------------------------
werrorfl - Output a standard eror message with variable number of arguements.
           Use a specified filename and line number instead of the default.

-------------------------------------------------------------------------------
*/

int werrorfl (char *newFilename, int newLineno, int errNum, ...);

/*
-------------------------------------------------------------------------------
fatal - Output a standard eror message with variable number of arguements and
        call exit()
-------------------------------------------------------------------------------
*/

void fatal (int exitCode, int errNum, ... );

/*
-------------------------------------------------------------------------------
style - Change the output error style to MSVC
-------------------------------------------------------------------------------
*/

void MSVC_style (int style);

/*
-------------------------------------------------------------------------------
disabled - Disable output of specified warning
-------------------------------------------------------------------------------
*/

void setWarningDisabled (int errNum);

/*
-------------------------------------------------------------------------------
disabledState - Enable/Disable output of specified warning
-------------------------------------------------------------------------------
*/

int setWarningDisabledState (int errNum, int disabled);

/*
-------------------------------------------------------------------------------
Set the flag to treat warnings as errors
-------------------------------------------------------------------------------
*/
void setWError (int flag);

#endif
