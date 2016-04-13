#ifndef _TILE_EXTRACTOR_H_
#define _TILE_EXTRACTOR_H_

#include <vector>
#include <string>
#include <FreeImage.h>
#include "Palette.hpp"
#include "ConversionOptions.hpp"
#include "Tile.hpp"
#include "ColorCombinator.hpp"
#include "ColorAndMaskValues.hpp"
#include "FileUtils.hpp"

using namespace std;

class TileExtractor {
	Tile getTile(FIBITMAP* bmp);
	ColorAndMaskValues getColorAndMask(FIBITMAP* bmp, int x, int y, bool flip);
	vector<unsigned char> extractPixels(FIBITMAP* bmp, int x, int y, bool flip);
	vector<unsigned char> extractTransPixels(vector<unsigned char> colors);
	unsigned int getModeIncrement();
	string removeExtension(const string &fileName);
	string sanitize(const string &name);

	unsigned int TileWidth;
	unsigned int TileHeight;
	unsigned int ModeIncrement;
	unsigned int Pitch;

public:
	ConversionOptions Options;
	vector<Tile> GetTiles(const string &fileName);
};

#endif
