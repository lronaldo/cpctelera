#include <iostream>
#include <iomanip>

typedef unsigned char  u8;
typedef unsigned int  u64;

#define PIXEL(DATA, P) ((DATA) & (0x01 << (P)))

u8 generateMaskM0(u8 pixel_data, u8 trasnparent_color) {
    u8 pixel0 = (PIXEL(pixel_data, 7)     ) + (PIXEL(pixel_data, 3) << 1) +
                (PIXEL(pixel_data, 5) << 2) + (PIXEL(pixel_data, 1) << 3);
    u8 pixel1 = (PIXEL(pixel_data, 6)     ) + (PIXEL(pixel_data, 2) << 1) +
                (PIXEL(pixel_data, 4) << 2) + (PIXEL(pixel_data, 0) << 3);
    u8 mask = 0;

    if (pixel0 == trasnparent_color) mask |= 0xAA;
    if (pixel1 == trasnparent_color) mask |= 0x55;

    return mask;
}

int main(void) {
    u8 pixel = 0;
    std::cout << "_cpct_transparentMaskM0::" << std::endl;
    for (u8 j=0; j < 16; j++) {
        std::cout << "   .db ";
        for (u8 i=0; i < 15; i++, pixel++)
             std::cout << "#0x" << std::setfill('0') << std::setw(2) << std::hex 
                       << (u64)generateMaskM0(pixel, 0) << ", ";
        std::cout << "#0x" << std::setfill('0') << std::setw(2) << std::hex 
                  << (u64)generateMaskM0(pixel, 0) << std::endl;
    }

    return 0;
}
