#ifndef _TILE_EXTRACTOR_H_
#define _TILE_EXTRACTOR_H_

#include <vector>
#include <string>
#include <iomanip>
#include <FreeImage.h>
#include "Palette.hpp"
#include "ConversionOptions.hpp"
#include "Tile.hpp"
#include "ColorCombinator.hpp"
#include "ColorAndMaskValues.hpp"
#include "FileUtils.hpp"

using namespace std;

class TileExtractor {
	Tile* getTile(FIBITMAP* bmp);
	
	void fillTileByRows(Tile* tile, FIBITMAP* bmp);
	void fillTileByCols(Tile* tile, FIBITMAP* bmp);
	void fillTileRLE(Tile* tile, FIBITMAP* bmp);
	void fillTileSCR(Tile* tile, FIBITMAP* bmp);
	void getByteAt(Tile* tile, FIBITMAP* bmp, int col, int row, bool flip, bool halfFlip);

	ColorAndMaskValues getColorAndMask(FIBITMAP* bmp, int x, int y, bool flip, bool halfFlip);
	vector<unsigned char> extractPixels(FIBITMAP* bmp, int x, int y, bool flip, bool halfFlip);
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
	vector<Tile*> GetTiles(const string &fileName);
};

#endif
