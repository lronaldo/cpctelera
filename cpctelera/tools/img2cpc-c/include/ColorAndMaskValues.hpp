#ifndef _COLOR_AND_MASK_VALUES_H_
#define _COLOR_AND_MASK_VALUES_H_

#include <vector>

using namespace std;

class ColorAndMaskValues {
public:
	vector<unsigned char> ColorValues;
	vector<unsigned char> TransparentValues;
	unsigned char ColorByte;
	unsigned char MaskByte;
};

#endif