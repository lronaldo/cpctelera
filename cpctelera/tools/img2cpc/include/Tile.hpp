#ifndef _TILE_H_
#define _TILE_H_

#include <vector>
#include <iostream>
#include "Palette.hpp"
#include "FileUtils.hpp"
#include <FreeImage.h>

using namespace std;

class Tile {
public:
	string Name;
	vector<unsigned char> Data;
	vector<unsigned char> MaskData;
	vector<unsigned char> SourceValues;
	vector<unsigned char> MaskValues;

	int TileWidth;
	int TileHeight;
	int TileWidthInBytes;
	TPalette Palette;
	string SourceFileName;

	void Dump() {
		cout << "{ " << endl
			 << "  Name: '" << this->Name << "'," << endl
			 << "  TileWidth: " << this->TileWidth << "," << endl
			 << "  TileHeight: " << this->TileHeight << "," << endl
			 << "  Data: [ ";

	    int x=0;
		for(unsigned char c : this->Data) {
			cout << hex << (int)c << ",";
			if((++x % this->TileWidth) == 0) { cout << endl; }
		}

		cout << " ]," << endl
			 << "  MaskData: [ ";

	    x=0;
		for(unsigned char m : this->MaskData) {
			cout << hex << (int)m << ",";
			if((++x % this->TileWidth) == 0) { cout << endl; }
		}

		cout << " ]," << endl
			 << "  SourceValues: [ ";

	    x = 0;
		for(unsigned char s : this->SourceValues) {
			cout << hex << (int)s << ",";
			if((++x % this->TileWidth) == 0) { cout << endl; }
		}
		cout << " ]," << endl
			 << "  MaskValues: [ ";

	    x = 0;
		for(unsigned char mv : this->MaskValues) {
			cout << hex << (int)mv << ",";
			if((++x % this->TileWidth) == 0) { cout << endl; }
		}

		cout << " ]" << endl
			 << "}";
	}

	void GenImage() {
		unsigned char *rgbValues = new unsigned char[this->TileHeight * this->TileWidth * 3];
		unsigned char *pos = rgbValues;

		for(unsigned char s : this->SourceValues) {
			Color c = this->Palette.Current[s];
			if (c.A) {
				pos[FI_RGBA_RED] = c.R;
				pos[FI_RGBA_GREEN] = c.G;
				pos[FI_RGBA_BLUE] = c.B;
			}
			else {
				pos[FI_RGBA_RED] = 255;
				pos[FI_RGBA_GREEN] = 0;
				pos[FI_RGBA_BLUE] = 255;
			}
			pos += 3;
		}

		stringstream ss;
		ss << this->Name << ".out.png";
		string fileName = ss.str();
		FileUtils::SaveImage(fileName, this->TileWidth, this->TileHeight, rgbValues);

		delete[] rgbValues;
	}

	void GenMaskImage() {
		unsigned char *rgbValues = new unsigned char[this->TileHeight * this->TileWidth * 3];
		int pos = 0;

		for(unsigned char s : this->MaskValues) {
			rgbValues[pos++] = s == 0 ? 0 : 0xFF;
			rgbValues[pos++] = s == 0 ? 0 : 0xFF;
			rgbValues[pos++] = s == 0 ? 0 : 0xFF;
		}

		stringstream ss;
		ss << this->Name << "_mask.png";
		string fileName = ss.str();
		FileUtils::SaveImage(fileName, this->TileWidth, this->TileHeight, rgbValues);

		delete[] rgbValues;
	}

};

#endif
