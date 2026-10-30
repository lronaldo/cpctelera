#ifndef _TILE_EXTRACTOR_H_
#define _TILE_EXTRACTOR_H_

#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>
#include "Image.hpp"
#include "Palette.hpp"
#include "ConversionOptions.hpp"
#include "Tile.hpp"
#include "ColorCombinator.hpp"
#include "ColorAndMaskValues.hpp"
#include "FileUtils.hpp"

using namespace std;

class TileExtractor {
	Tile* getTile(const Image& img);
	
	void fillTileByRows(Tile* tile, const Image& img);
	void fillTileByCols(Tile* tile, const Image& img);
	void fillTileRLE(Tile* tile, const Image& img);
	void fillTileSCR(Tile* tile, const Image& img);
	void checkSCRSize();
	void getByteAt(Tile* tile, const Image& img, int col, int row, bool flip, bool halfFlip);

	ColorAndMaskValues getColorAndMask(Tile* tile, const Image& img, int x, int y, bool flip, bool halfFlip);
	vector<unsigned char> extractPixels(Tile* tile, const Image& img, int x, int y, bool flip, bool halfFlip);
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
