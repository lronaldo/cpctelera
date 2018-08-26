#include "img2cpc.hpp"
#include "FileUtils.hpp"

int initializeParser(ezOptionParser &parser) {
	parser.overview = "img2cpc - (c) 2007-2018 Retroworks.";
	parser.syntax = "img2cpc [OPTIONS] fileNames";
	parser.example = "img2cpc -w 8 -h 8 --outputFormat asm -bn tile -m 0 tiles.png\n";
	parser.footer = "If you liked this program, drop an email at: augusto.ruiz@gmail.com\n";

	parser.add("", 0, 1, 0, "Tile width. If not set, the image width is used.", "-w", "--tileWidth");
	parser.add("", 0, 1, 0, "Tile height. If not set, the image height is used.", "-h", "--tileHeight");
	parser.add("", 0, 1, 0, "Tile base name. If not set, the filename will be used.", "-bn", "--baseName");
	parser.add("", 0, 0, 0, "Absolute base name. Use the tile base name as the variable identifier, no composition.", "-abn", "--absoluteBaseName");

	parser.add("asm", 0, 1, 0, "Output format. If not set, data will be generated in assembly format.", "-of", "--outputFormat");
	parser.add("", 0, 0, 0, "Output tile size as constants.", "-osz", "--outputSize");
	parser.add("", 0, 0, 0, "Create a flipped values look-up table for the current palette and mode.", "-f");

	parser.add("0", 0, 1, 0, "Specifies the CPC Mode to generate data for. Valid values are 0 (default), 1 or 2.", "-m", "--mode");
	parser.add("", 0, 0, 0, "Generate tile map", "-map", "--tilemap");
	parser.add("", 0, 1, 0, "Output file name. img2cpc will append the proper extension based on format.", "-o", "--outputFileName");
	parser.add("", 0, 1, ',', "Scanline order. Default value is 01234567", "-s", "--scanlineOrder");
	parser.add("", 0, 0, 0, "Zigzag. Generate data in zigzag order.", "-z", "--zigzag");
	parser.add("", 0, 0, 0, "HalfFlip. Generate data with odd lines preflipped.", "-hf", "--halfflip");
	parser.add("", 0, 0, 0, "RLE. Generate data with run length encoding.", "-r", "-rle");
	parser.add("", 0, 0, 0, "SCR format.", "-scr");

	parser.add("row", 0, 1, 0, "Specifies the byte order, whether the data is stored by rows (default) or by columns. Valid values are row, col.", "-bo", "--byteOrder");

	parser.add("", 0, 1, ',', "Palette specified in firmware values.\nThe palette can have up to 16 entries if no masks are used, or 17 entries if masks are used. The 17th entry should be used as transparent color.", "-fwp", "--firmwarePalette");
	parser.add("", 0, 1, ',', "Palette specified in RGB values. RGB values must be specified as 0xRRGGBB, where RR, GG and BB are hexadecimal, or as an int value.\nThe palette can have up to 16 entries if no masks are used, or 17 entries if masks are used. The 17th entry should be used as transparent color.\nExamples: 0x1122FF, 255", "-rgbp", "--rgbPalette");
	parser.add("", 0, 1, ',', "Palette specified in hardware values.\nThe palette can have up to 16 entries if no masks are used, or 17 entries if masks are used. The 17th entry should be used as transparent color.", "-hwp", "--hardwarePalette");
	parser.add("", 0, 1, 0, "Specifies input palette file.\nThe palette can have up to 16 entries if no masks are used, or 17 entries if masks are used. The 17th entry should be used as transparent color.", "-p", "--palette");
	parser.add("", 0, 1, 0, "Specifies transparent color (as index in palette). If masks are used, you can use the 17th entry (specify value 16) in the palette.", "-t", "--transparentColor");
	parser.add("", 0, 0, 0, "Specifies that image transparent color should be used (pixels with alpha values equal to 0).", "-ta", "--transparentAlpha");
	parser.add("", 0, 0, 0, "Interlaced masks. Mask values will be interlaced with pixel values.", "-im", "--interlacedMasks");
	parser.add("", 0, 0, 0, "No mask data. And/Or tables will be used.", "-nm", "--noMasks");

	parser.add("", 0, 0, 0, "Don't create tileset. Use this if you are creating sprites and do not need a table with all the tile pointers.", "-nt", "--noTileset");

	parser.add("", 0, 0, 0, "Output only information about sizes of the images provided.", "--img-size");

	parser.add("", 0, 0, 0, "Output palette (hardware values).", "-ophw");
	parser.add("", 0, 0, 0, "Output palette (firmware values).", "-opfw");

	parser.add("", 0, 0, 0, "Generates PNG images to check tile output.", "-g", "--generatePNG");
	parser.add("", 0, 0, 0, "Generate one output file per processed file. Files will be named using the base name (if specified) and the source file name .", "--oneFile");

	parser.add("", 0, 1, ',', "Additional includes to add to header file when generating C data files.", "--includes");

	parser.add("", 0, 0, 0, "Help. Show usage.", "--help");

	return 0;
}

void showUsage(ezOptionParser &options) {
	string usage;
	options.getUsage(usage);
	cout << usage << endl;
}

int initializeImageLoader() {
	FreeImage_Initialise();
	return 0;
}

int processImage(const string& filename, vector<Tile*>& tiles, ConversionOptions &convOptions, ezOptionParser &options) {
	TileExtractor extractor;
	extractor.Options = convOptions;
	vector<Tile*> tmp = extractor.GetTiles(filename);
	for (Tile* t : tmp) {
		//t.Dump();
		if (options.isSet("-g")) {
			t->GenImage();
			if (convOptions.Palette.TransparentIndex!=-1) {
				t->GenMaskImage();
			}
		}
	}
	tiles.insert(tiles.end(), tmp.begin(), tmp.end());
	return 0;
}

int dumpTiles(vector<Tile*>& tiles, ConversionOptions &convOptions) {
	if(!tiles.empty()) {
		OutputGenerator generator;	
		string outputName = convOptions.OutputFileName;
		string currentFileName;
		vector<Tile*> currentTiles;
		vector<Tile*>::iterator it = tiles.begin();

		do {
			if(convOptions.OneFilePerSourceFile) {
				currentFileName = (*it)->SourceFileName;
				currentTiles.clear();
				stringstream ss;
				if(!outputName.empty()) {
					ss << outputName;					
				}
				ss << FileUtils::RemoveExtension(currentFileName);
				convOptions.OutputFileName = ss.str();

				while(it!=tiles.end() && (*it)->SourceFileName == currentFileName) {
					currentTiles.push_back(*it);
					it++;
				}
			} else {
				it = tiles.end();
				currentTiles.insert(currentTiles.end(), tiles.begin(), tiles.end());
			}

			switch (convOptions.Format) {
				case ConversionOptions::ASSEMBLER:
					generator.GenerateASM(currentTiles, convOptions);
					break;
				case ConversionOptions::ASSEMBLER_ASXXXX:
					generator.GenerateASXXXX(currentTiles, convOptions);
					break;
				case ConversionOptions::BINARY:
					generator.GenerateBIN(currentTiles, convOptions);
					break;
				case ConversionOptions::PURE_C:
					generator.GenerateC(currentTiles, convOptions);
					generator.GenerateH(currentTiles, convOptions);
					break;
			}		
		} while(it != tiles.end());
	}
	return 0;
}

int extractPalette(ezOptionParser &options, ConversionOptions &convOptions) {
	int result = 0;

	TPalette &palette = convOptions.Palette;
	palette.UpdateMaxEntries(convOptions.Mode, !convOptions.NoMaskData);

	if (!options.isSet("-fwp") && !options.isSet("-rgbp") && !options.isSet("-hwp") && !options.isSet("-p")) {
		return -1;
	}
	if (options.isSet("-fwp")) {
		// Palette specified in firmware indexes.
		vector<string> fwIndices;
		options.get("-fwp")->getStrings(fwIndices);
		result = palette.ParseFW(fwIndices);
	}
	else if (options.isSet("-rgbp")) {
		// Palette specified as rgb values.
		vector<string> rgbValues;
		options.get("-rgbp")->getStrings(rgbValues);
		result = palette.ParseRGB(rgbValues);
	}
	else if (options.isSet("-hwp")) {
		// Palette specified as hardware values.
		vector<string> hwIndices;
		options.get("-hwp")->getStrings(hwIndices);
		result = palette.ParseHW(hwIndices);
	}
	else if (options.isSet("-p")) {
		string paletteFile;
		options.get("-p")->getString(paletteFile);
		result = palette.ParseFile(paletteFile);
	}

/*
	if (options.isSet("-ta")) {
		if (convOptions.NoMaskData) {
			result = -1;
			cout << "You must generate masks in order to use image transparent pixels. Otherwise you must select a palette index." << endl;
		}
		else {
			palette.TransparentIndex = palette.Current.size();
			palette.Current.push_back(Color(0, 0, 0, 0));
		}
	}
*/

	if (options.isSet("-t")) {
		int maxValue = palette.Current.size();
		int value;
		options.get("-t")->getInt(value);
		palette.TransparentIndex = value;
		if (palette.TransparentIndex < 0 || palette.TransparentIndex >= maxValue) {
			cout << "Transparent color must be between 0 and " << maxValue - 1 << "." << endl;
			result = -1;
		}
	}
	return result;
}

int extractConversionOptions(ezOptionParser &options, ConversionOptions &convOptions) {
	int result = 0;
	convOptions.CreateTileset = !(options.isSet("-nt"));
	convOptions.OneFilePerSourceFile = options.isSet("--oneFile");

	convOptions.PaletteFormat = ConversionOptions::NONE;
	if (options.isSet("-ophw")) {
		convOptions.PaletteFormat = ConversionOptions::HARDWARE;
	}
	else if (options.isSet("-opfw")) {
		convOptions.PaletteFormat = ConversionOptions::FIRMWARE;
	}

	//convOptions.Palette.Dump();
	if (options.isSet("-w")) {
		options.get("-w")->getInt(convOptions.TileWidth);
	}
	if (options.isSet("-h")) {
		options.get("-h")->getInt(convOptions.TileHeight);
	}
	if (options.isSet("-bn")) {
		options.get("-bn")->getString(convOptions.BaseName);
	}
	convOptions.AbsoluteBaseName = options.isSet("-abn");

	if(convOptions.AbsoluteBaseName && options.lastArgs.size() > 1) {
		cout << "ERROR: Cannot specify more than one file with the --absoluteBaseName or -abn switch." << endl;
		return -1;
	}
	convOptions.OutputSize = options.isSet("-osz");

	string outputFmt;
	options.get("-of")->getString(outputFmt);
	//cout << outputFmt << endl;
	convOptions.ParseFormat(outputFmt);

	string byteOrder;
	options.get("-bo")->getString(byteOrder);
	convOptions.ParseByteOrder(byteOrder);

	options.get("-m")->getInt(convOptions.Mode);
	if (convOptions.Mode < 0 || convOptions.Mode > 2) {
		result = -1;
		cout << "Error." << endl
			<< "CPC Mode must be 0, 1 or 2." << endl;
	}

	if(options.isSet("-o")) {
		options.get("-o")->getString(convOptions.OutputFileName);			
	} else if(!convOptions.OneFilePerSourceFile) {
		// If one file per source file, empty file name is allowed.
		convOptions.OutputFileName = "gfx";
	}

	if (options.isSet("-s")) {
		convOptions.ScanlineOrder.clear();
		options.get("-s")->getInts(convOptions.ScanlineOrder);
		for(int s:convOptions.ScanlineOrder) {
			if(s < 0 || s > 7) {
				cout << "ERROR: Scanlines are not specified correctly. Valid scanline values are [0-7]";
				result = -1;
			}
		}
		cout << endl;
	}

	convOptions.ZigZag = options.isSet("-z");
	convOptions.HalfFlip = options.isSet("-hf");
	convOptions.CreateFlipLut = options.isSet("-f");
	convOptions.NoMaskData = options.isSet("-nm");
	convOptions.RLE = options.isSet("-r");

	if(options.isSet("--includes")) {
		if(convOptions.Format == ConversionOptions::PURE_C) {
			options.get("--includes")->getStrings(convOptions.AdditionalIncludes);
		} else {
			cout << "Warning: Additional includes for output format different than C files. Ignored." << endl;
		}
	}
	
	if(!result) {
		result = extractPalette(options, convOptions);		
	}

	convOptions.InterlaceMasks = options.isSet("-im");

	if (convOptions.Palette.TransparentIndex == -1) {
		//cout << "Warning: No transparent color specified. If images with transparent pixels are used, transparent pixels will be considered as such." << endl;
		//convOptions.Palette.TransparentIndex = convOptions.Palette.Current.size();
		//convOptions.Palette.Current.push_back(Color(0, 0, 0, 0));
	}

	if(!result) {
		convOptions.InitLutTables();
	}

	convOptions.IsScr = options.isSet("-scr");

	return result;
}

void createFlipLut(ConversionOptions &convOptions) {
	unsigned char flipLut[0x100];
    for (int i = 0; i < 0x100; i++) {
        flipLut[i] = ColorCombinator::FlipByteNum(convOptions.Mode, i);
    }

    OutputGenerator generator;
	switch (convOptions.Format) {
		case ConversionOptions::ASSEMBLER:
			generator.DumpTableASM(flipLut, 0x100, "flipLut");
			break;
		case ConversionOptions::ASSEMBLER_ASXXXX:
			generator.DumpTableASXXXX(flipLut, 0x100, "flipLut");
			break;
		case ConversionOptions::BINARY:
			generator.DumpTableBIN(flipLut, 0x100, "flipLut");
			break;
		case ConversionOptions::PURE_C:
			generator.DumpTableC(flipLut, 0x100, "flipLut");
			break;
	}
}

void createAndOrTables(ConversionOptions &convOptions) {
    OutputGenerator generator;
	switch (convOptions.Format) {
		case ConversionOptions::ASSEMBLER:
			generator.DumpTableASM(convOptions.AndMaskLut, 0x100, "andMasks");
			generator.DumpTableASM(convOptions.OrMaskLut, 0x100, "orMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableASM(convOptions.FlippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableASM(convOptions.FlippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
		case ConversionOptions::ASSEMBLER_ASXXXX:
			generator.DumpTableASXXXX(convOptions.AndMaskLut, 0x100, "andMasks");
			generator.DumpTableASXXXX(convOptions.OrMaskLut, 0x100, "orMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableASXXXX(convOptions.FlippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableASXXXX(convOptions.FlippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
		case ConversionOptions::BINARY:
			generator.DumpTableBIN(convOptions.AndMaskLut, 0x100, "andMasks");
			generator.DumpTableBIN(convOptions.OrMaskLut, 0x100, "orMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableBIN(convOptions.FlippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableBIN(convOptions.FlippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
		case ConversionOptions::PURE_C:
			generator.DumpTableC(convOptions.AndMaskLut, 0x100, "andMasks");
			generator.DumpTableC(convOptions.AndMaskLut, 0x100, "andMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableC(convOptions.FlippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableC(convOptions.FlippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
	}
}

int outputImageWidthAndHeightOnly(const vector<string*> lastArgs) {
	int result = 0;
	for (auto* fileName : lastArgs) {
		FIBITMAP *dib = FileUtils::LoadImage(*fileName);
		if ( dib ) {
			cout << FreeImage_GetWidth(dib) << '\t' << FreeImage_GetHeight(dib) << '\t' << *fileName << '\n';
		} else {
			cerr << "xxx" << '\t' << "xxx" << '\t' << "Error processing image '"<< *fileName << "' Please check that file is readable and is a valid image file." << '\n';
			result = 1;
		}
	}

	return result;
}

int main(int argc, const char** argv)
{
	if (initializeImageLoader()) {
		return -1;
	}
	ezOptionParser options;
	if (initializeParser(options)) {
		return -1;
	}

	options.parse(argc, argv);

	if (options.isSet("--help")) {
		showUsage(options);
		return 0;
	}

	vector<string *> &lastArgs = options.lastArgs;

	if (options.isSet("--img-size")) {
		return outputImageWidthAndHeightOnly(lastArgs);
	}

	ConversionOptions convOptions;
	int optionsResult = extractConversionOptions(options, convOptions);
	if (optionsResult) {
		cout << "Couldn't parse conversion options. Use img2cpc --help for more information." << endl;
		return optionsResult;
	}
		//convOptions.Dump();

	vector<Tile*> tiles;
	for (long int i = 0, li = lastArgs.size(); i < li; ++i) {
		string filename = *lastArgs[i];
		int result = processImage(filename, tiles, convOptions, options);
		if (result) {
			cerr << "Error processing image: " << filename << endl;
			return result;
		}
	}

	if(convOptions.CreateFlipLut) {
		createFlipLut(convOptions);
	}
	if(convOptions.NoMaskData) {
		createAndOrTables(convOptions);		
	}

	dumpTiles(tiles, convOptions);

	for(vector<Tile*>::iterator tileIter = tiles.begin(); tileIter!=tiles.end(); ++tileIter) {
		delete (*tileIter);
	}
	tiles.clear();
	
	return 0;
}