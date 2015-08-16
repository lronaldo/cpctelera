#ifndef _COLOR_H_
#define _COLOR_H_

#include <cmath>
#include <iostream>

using namespace std;

class Color {
public:
	unsigned char R;
	unsigned char G;
	unsigned char B;

   Color(): R(0),G(0),B(0) {};

	Color(const unsigned char r, 
		  const unsigned char g, 
		  const unsigned b): R(r),G(g),B(b) { };

	unsigned int toInt() { return (this->R << 16) | (this->G << 8) | (this->B); };

	double Distance(const Color &other) { return Color::Distance(*this, other); };
	
	void Dump() { 
		cout << "{ r: " << (unsigned int) this->R ;
		cout << ", g: " << (unsigned int) this->G ;
		cout << ", b: " << (unsigned int) this->B ;
		cout << " }"; 
	};

	static double Distance (const Color &col1, const Color &col2) {
        int deltaR = col1.R - col2.R;
        int deltaG = col1.G - col2.G;
        int deltaB = col1.B - col2.B;
        return sqrt(
        	(double)(
        		(deltaR * deltaR) + 
        		(deltaG * deltaG) + 
        		(deltaB * deltaB)
        	));
	};
};

#endif