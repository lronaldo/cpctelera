#ifndef _IMG2CPC_H_
#define _IMG2CPC_H_

#include <iostream>
#include <string>
#include <FreeImage.h>
#include <json/json.h>
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
int processImage(const string& filename, ConversionOptions &options);

int main(int argc, const char** argv);

#endif
