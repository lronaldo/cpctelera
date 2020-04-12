/** Add, sub tests.

    type: signed char, int, long
    storage: static, 
    attr: volatile
*/
#include <testfwk.h>

{type} add_func({type} i)
{
	return(i + (5ul << 16));
}

void 
testAdd(void)
{
  {storage} {attr} {type} left, right, result;

  left = 5;
  right = 26;

  result = left+right;
  ASSERT(result == 31);
  
  left = 39;
  right = -120;
  
  result = left+right;
  ASSERT(result == (39-120));

  left = -39;
  right = 80;
  
  result = left+right;
  ASSERT(result == (-39+80));

  left = -39;
  right = -70;
  
  result = left+right;
  ASSERT(result == (-39-70));

  result += 0xab00;
  ASSERT(result == ({type})(0xab00-39-70));

  left = 0x5500;
  right = 0x0a00;

  result = left + right;
  ASSERT(result == ({type})(0x5500 + 0x0a00));

  left = 0x550000ul;

  result = left + 0x0a0000ul;
  ASSERT(result == ({type})(0x550000ul + 0x0a0000ul));

  ASSERT(add_func(0) == ({type})(5ul << 16));
}

void 
testSub(void)
{
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
  {storage} {attr} {type} left, right, result;

  left = 5;
  right = 26;

  result = left-right;
  ASSERT(result == (5-26));

  left = 39;
  right = -76;

  result = left-right;
  ASSERT(result == (39+76));

  left = -12;
  right = 56;

  result = left-right;
  ASSERT(result == (-12-56));
  
  left = -39;
  right = -20;

  result = left-right;
  ASSERT(result == (-39+20));

  result = left-(signed)0x1200;
  ASSERT(result == ({type})(-39-(signed)0x1200));
#endif
}
