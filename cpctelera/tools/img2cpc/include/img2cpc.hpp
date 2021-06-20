#ifndef _IMG2CPC_H_
#define _IMG2CPC_H_

#include <iostream>
#include <string>
#include <FreeImage.h>
#include <json/json.h>
#include <cstdio>
#include "Palette.hpp"
#include "ezOptionParser.hpp"
#include "TileExtractor.hpp"
#include "ConversionOptions.hpp"
#include "outputGenerator.hpp"

using namespace std;
using namespace ez;

int initializeParser(ezOptionParser &parser);
void showUsage(ezOptionParser &options);

int extractPalette(ezOptionParser &options, TPalette &palette);
int extractConversionOptions(ezOptionParser &options, ConversionOptions &convOptions);

int initializeImageLoader();
int processImage(const string& filename, vector<Tile*>& tiles, ConversionOptions &convOptions, ezOptionParser &options);
int dumpTiles(vector<Tile*>& tiles, ConversionOptions &convOptions);

void createAndOrTables(ConversionOptions &options);
void createFlipLut(ConversionOptions &options);

int main(int argc, const char** argv);

#endif