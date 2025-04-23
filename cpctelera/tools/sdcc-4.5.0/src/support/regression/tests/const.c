/* Tests usage of const qualifier.
 */
#include <testfwk.h>

char k;					/* char */
const char const_char = 123;		/* constant char */
const char * const_char_ptr;		/* pointer to constant char */
char * const char_ptr_const = &k;	/* constant pointer to char */
const char * const const_char_ptr_const = &const_char;
					/* constant pointer to constant char */
char char_array[3];			/* array of char */
const char const_char_array[] = {1,2,3}; /* array of constant char */
const char ** const_char_ptr_ptr;	/* pointer to pointer to constant char */

char
ident(char x)
{
  return x;
}

void
testConst(void)
{
  /* Since const_char_ptr is in itself not constant, we can change it */
  const_char_ptr = const_char_array;
  const_char_ptr++;
  ASSERT(*const_char_ptr == 2);

  /* Check for bug #621531 */
  const_char_ptr = const_char_array;
  ASSERT(const_char_ptr[0] == 1);
  const_char_ptr++;

  /* Since char_ptr_const is constant, we cannot change it. However, */
  /* we can change the object that it points to.                     */
  *char_ptr_const = 5;
  ASSERT(ident(*char_ptr_const)==5);
  (*char_ptr_const)++;
  ASSERT(ident(*char_ptr_const)==6);

  /* We can't modify const_char_ptr_const or the object that it points */
  /* to. Unfortunately, we can't test that compiler enforces these     */
  /* restriction, so just verify its initialization.                   */
  ASSERT(*const_char_ptr_const == const_char);

  /* We can change const_char_ptr_ptr or the object that it points to. */
  const_char_ptr = const_char_array;
  const_char_ptr_ptr = &const_char_ptr;
  ASSERT(ident(**const_char_ptr_ptr)==1);
  (*const_char_ptr_ptr)++;
  ASSERT(ident(**const_char_ptr_ptr)==2);
}
