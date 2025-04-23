
#ifdef TEST1
#ifdef __SDCC
_Pragma("std_c23")
#endif

enum /* no tag */ : char { A }; /* OK */
enum tag1 : int;                /* OK */
enum tag2 : const int;          /* OK */
enum tag3 : unsigned short;     /* OK */
enum tag4 : float;              /* ERROR */

#endif


#ifdef TEST2
#ifdef __SDCC
_Pragma("std_c23")
#endif

enum tag1 : int { A };          /* OK */
enum tag2 : struct { int a; };  /* ERROR */

#endif


#ifdef TEST3
#ifdef __SDCC
_Pragma("std_c23")
#endif

typedef int my_int;
typedef struct { int a; } my_struct;

enum tag1 : my_int;             /* OK */
enum tag2 : my_struct;          /* ERROR */

#endif


#ifdef TEST4
#ifdef __SDCC
_Pragma("std_c23")
#endif

typedef struct { int a; } my_struct;
typedef enum : char { A } my_enum;
my_struct s;

enum tag1 : typeof(s.a);        /* OK */
enum tag2 : my_enum;            /* ERROR */

#endif
