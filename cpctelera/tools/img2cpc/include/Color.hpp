#ifndef _COLOR_H_
#define _COLOR_H_

#include <cmath>
#include <iostream>

using namespace std;

class Color {
public:
	unsigned char A;
	unsigned char R;
	unsigned char G;
	unsigned char B;

	Color(): Color(0, 0,0,0) {};

	Color(const unsigned char r, 
		  const unsigned char g, 
		  const unsigned b): A(255), R(r),G(g),B(b) { };

	Color(const unsigned char a, 
		  const unsigned char r, 
		  const unsigned char g, 
		  const unsigned b): A(a), R(r),G(g),B(b) { };

	unsigned int toInt() { return (this->A << 24) | (this->R << 16) | (this->G << 8) | (this->B); };

	double Distance(const Color &other) { return Color::Distance(*this, other); };
	
	void Dump() { 
		cout << "{ a: " << (unsigned int) this->A ;
		cout << ", r: " << (unsigned int) this->R ;
		cout << ", g: " << (unsigned int) this->G ;
		cout << ", b: " << (unsigned int) this->B ;
		cout << " }"; 
	};

	static double Distance (const Color &col1, const Color &col2) {
		if(col1.A == 0 && col2.A == 0) {
			return 0.0;
		}
		else {
			int deltaA = col1.A - col2.A;
	        int deltaR = col1.R - col2.R;
	        int deltaG = col1.G - col2.G;
	        int deltaB = col1.B - col2.B;
	        return sqrt(
	        	(double)(
	        		(deltaA * deltaA) +
	        		(deltaR * deltaR) + 
	        		(deltaG * deltaG) + 
	        		(deltaB * deltaB)
	        	));			
			}
	};
};

#endif