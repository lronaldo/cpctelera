#ifndef _COLOR_COMBINATOR_H_
#define _COLOR_COMBINATOR_H_

#include <vector>

class ColorCombinator {
public:
  static unsigned char CombineColData(vector<unsigned char> sourceColors) {
    vector<unsigned char> sourceBits;
    if(sourceColors.size() == 2) {
  		sourceBits = {1, 4, 2, 8};
  	} else if(sourceColors.size() == 4) {
  		sourceBits = {1, 2};
  	} else if(sourceColors.size() == 8) {
  		sourceBits = {1};
  	}
    unsigned char mask = 0x80;
    unsigned char result = 0;
    for (unsigned char sourceBit : sourceBits) {
      for(unsigned char sourceColor : sourceColors) {
      	unsigned char sourceColorBit = (unsigned char)(sourceColor & sourceBit);
      	if(sourceColorBit != 0) {
      		result = (unsigned char)(result | mask);
      	}
      	mask = (unsigned char)(mask >> 1);
      }
    }
    return result;
  }

  static unsigned char MaxValueByMode(int mode) {
  	vector<unsigned char> maxValues = { 15, 3, 1};
  	return maxValues[mode];
  }

  static string FlipByte(int mode, int i) {
  	stringstream ss;
  	ss << hex << ColorCombinator::FlipByteNum(mode, i);
    return ss.str();
  }

  static unsigned char FlipByteNum(int mode, int i) {
    unsigned char byte = (unsigned char) i;
    unsigned char result = 0;
    vector<unsigned char> masks = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 };
    vector<unsigned char> mode0 = { 2, 1, 8, 4, 0x20, 0x10, 0x80, 0x40 };
    vector<unsigned char> mode1 = { 8, 4, 2, 1, 0x80, 0x40, 0x20, 0x10 };
    vector<unsigned char> mode2 = { 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1 };
    vector<unsigned char> currentOrder = mode0;
    switch (mode) {
      case 0:
        currentOrder = mode0;
        break;
      case 1:
        currentOrder = mode1;
        break;
      case 2:
        currentOrder = mode2;
        break;
      default:
      	stringstream ss;
      	ss << "Current graphic mode value is not allowed. Mode " << mode;
        throw ss.str();
    }
    for (int j = 0; j < 8; j++) {
      if ((byte & masks[j]) != 0) {
        result = (unsigned char)(result | currentOrder[j]);
      }
    }
    return result;
  }
};

#endif