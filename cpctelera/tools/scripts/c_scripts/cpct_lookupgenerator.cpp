#include <iostream>
#include <iomanip>

typedef unsigned char  u8;
typedef unsigned int  u64;

#define PIXEL(DATA, P) ((DATA) & (0x01 << (P)))

u8 generateMaskM0(u8 pixel_data, u8 trasnparent_color) {
    u8 pixel0 = (PIXEL(pixel_data, 7) >> 7) + (PIXEL(pixel_data, 3) >> 2) +
                (PIXEL(pixel_data, 5) >> 3) + (PIXEL(pixel_data, 1) << 2);
    u8 pixel1 = (PIXEL(pixel_data, 6) >> 6) + (PIXEL(pixel_data, 2) >> 1) +
                (PIXEL(pixel_data, 4) >> 2) + (PIXEL(pixel_data, 0) << 3);
    u8 mask = 0;

    if (pixel0 == trasnparent_color) mask |= 0xAA;
    if (pixel1 == trasnparent_color) mask |= 0x55;

    return mask;
}

void drawPixelItem(u8 pixel){
    std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex 
              << (u64)generateMaskM0(pixel, 0);
}

int main(void) {
    u8 pixel = 0;
    std::cout << "cpct_transparentMaskM0[256] = {" << std::endl;
    for (u8 j=0; j < 16; j++) {
        for (u8 i=0; i < 16; i++, pixel++) {
             drawPixelItem(pixel);
             std::cout << ", ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;    

    return 0;
}
