/* bug-3090.c
   A bug in gbz80 code generation for 16-bit addition with operands allocated to bc and de, result spilt.
 */
 
#include <testfwk.h>
 
#define UINT8           unsigned char
#define specialchar_nl  '\n'

#define buffer_length (16)
UINT8 buffer[buffer_length];

UINT8 smart_write(const UINT8 y, const UINT8 height, char *str){
    UINT8 length;
    UINT8 run = 1;
    UINT8 tmp_y = y;
    UINT8 jump_back = 0;
    UINT8 ret = 0;
    // string pointer
    char *str_ptr = str;
    while(run){
        for(length = 0; length < 16; ++length){
            if(*str_ptr == specialchar_nl || *str_ptr == '\0'){
                buffer[length] = '\0';
                break;
            }
            buffer[length] = *str_ptr;
            ++str_ptr;
            if(jump_back != 0){
                --jump_back;
            }
        }
        if(*str_ptr == '\0'){
            run = 0;
        } else {
            if(*str_ptr == specialchar_nl)
                ++str_ptr;
            tmp_y += 1;
        }
        if(tmp_y >= y+height){ // Wrong code generated for addition here.
            tmp_y = y;
            ++ret;
        }
    }
    return ret;
}

void
testBug(void){
    ASSERT(smart_write(1, 20, "test\n") == 0);
}

