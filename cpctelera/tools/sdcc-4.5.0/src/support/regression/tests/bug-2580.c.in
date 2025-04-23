/** bug-2569.c
   An error in code generation for assignment of register parameters to global variables
   in __z88dk_fastcall functions when using --reserve-regs-iy

   type: char, int, long
 */

#include <testfwk.h>

unsigned {type} game_menu_sel;
unsigned {type} s_lin1;

void game_menu_back(unsigned {type} f_start) __z88dk_fastcall
{
    game_menu_sel = 0;
    s_lin1 = f_start;
}

void testBug(void)
{
    game_menu_back(0x55aaa5a5ul);
    ASSERT(game_menu_sel == 0);
    ASSERT(s_lin1 == (unsigned {type})0x55aaa5a5ul);
}

