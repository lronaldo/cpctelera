/* bug-2993.c
   Invalid asm was generated in the stm8 backend for calls via immediate (or literal) function pointers.
 */

#include <testfwk.h>

void f(void)
{
	void (*p)(void) __reentrant = (void (*)(void))0x24;
	(*p)();
}

int func(void *var,int val) __reentrant;
typedef int (*SetVal)(void *, int) __reentrant;
typedef struct {
                int val;
                SetVal setval;
                } my_struct; 

void
testBug(void)
{  
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_HUGE)) // Bug #2994
    my_struct str1 = {0,func};
    my_struct str2 = {0,func};
    str1.setval(&str1,11);  
    str2.setval(&str2,5);
    
    ASSERT (str1.val == 11);
    ASSERT (str2.val == 5);
#endif
}

int func(void *var,int val) __reentrant
{
    my_struct *tmp = (my_struct*)var;
    tmp->val = val;
    return 0;
}

