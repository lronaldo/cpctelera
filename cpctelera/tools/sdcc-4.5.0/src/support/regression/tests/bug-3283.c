/*
   bug-3283.c. On z80, a variable allocated to iy was overwritten by the use of iy for an address at a cast.
 */

#include <testfwk.h>

#pragma disable_warning 85

typedef struct object_d {
    unsigned char code;
    char *desc;
    unsigned int position;
    unsigned char attributes;
} object;


object *odummy;
unsigned char current_position;

char *message1006;
char *message1005;

extern object *search_object_p(unsigned int o);
extern void show_message(const char *m);

unsigned char get(unsigned int o)
{
    odummy=search_object_p(o); // odummy in iy.
    if(odummy->position!=current_position) { // a cast from unsigned char to unsigned int here used iy to point to current_position.
        show_message(message1006);
    } else if((odummy->attributes&1)==0) {
        show_message(message1005);
    } else {
        odummy->position=1500; // odummy used again here.
        return 0;
    }
    return 1;
}

object obj = {0, 0, 0, 1};

void
testBug (void)
{
    ASSERT(!get(0));
    ASSERT(obj.position==1500);
}

object *search_object_p(unsigned int o)
{
    return &obj;
}

void show_message(const char *m)
{
    ASSERT (0);
}

