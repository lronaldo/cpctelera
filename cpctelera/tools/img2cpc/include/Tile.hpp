#ifndef _TILE_H_
#define _TILE_H_

#include <vector>
#include <iostream>
#include <sstream>
#include "Image.hpp"
#include "Palette.hpp"
#include "FileUtils.hpp"

using namespace std;

class Tile {
public:
	string Name;
	vector<unsigned char> Data;
	vector<unsigned char> MaskData;
	vector<unsigned char> SourceValues;
	vector<unsigned char> MaskValues;
	// Palette index of every pixel, row-major and unflipped. Used to generate PNG previews.
	vector<unsigned char> PreviewValues;
	static constexpr unsigned char NoPreviewValue = 0xFF;

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

	// Writes the tile as converted to the palette. Pixels without a palette colour are magenta.
	void GenImage() {
		Image image(this->TileWidth, this->TileHeight);
		auto pixel = image.Pixels().begin();
		for(unsigned char s : this->PreviewValues) {
			if (s < this->Palette.Current.size() && this->Palette.Current[s].A) {
				const Color& c = this->Palette.Current[s];
				*pixel = { c.R, c.G, c.B, 255 };
			}
			else {
				*pixel = { 255, 0, 255, 255 };
			}
			++pixel;
		}
		image.SavePNG(this->Name + ".out.png");
	}

	// Writes the transparency mask of the tile: white for transparent pixels, black otherwise.
	void GenMaskImage() {
		Image image(this->TileWidth, this->TileHeight);
		auto pixel = image.Pixels().begin();
		for(unsigned char s : this->PreviewValues) {
			const unsigned char v = (s == this->Palette.TransparentIndex) ? 0xFF : 0;
			*pixel++ = { v, v, v, 255 };
		}
		image.SavePNG(this->Name + "_mask.png");
	}

};

#endif
