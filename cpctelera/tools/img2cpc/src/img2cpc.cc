
#include "img2cpc.hpp"

int initializeParser(ezOptionParser &parser) {
	parser.overview = "img2cpc - (c) 2007-2015 Retroworks.";
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

	parser.add("", 0, 1, ',', "Palette specified in firmware values.", "-fwp", "--firmwarePalette");
	parser.add("", 0, 1, ',', "Palette specified in RGB values. RGB values must be specified as 0xRRGGBB, where RR, GG and BB are hexadecimal, or as an int value.\nExamples: 0x1122FF, 255", "-rgbp", "--rgbPalette");
	parser.add("", 0, 1, ',', "Palette specified in hardware values.", "-hwp", "--hardwarePalette");
	parser.add("", 0, 1, 0, "Specifies input palette file.", "-p", "--palette");
	parser.add("", 0, 1, 0, "Specifies transparent color (as index in palette).", "-t", "--transparentColor");
	parser.add("", 0, 0, 0, "Interlaced masks. Mask values will be interlaced with pixel values.", "-im", "--interlacedMasks");
	parser.add("", 0, 0, 0, "No mask data. And/Or tables will be used.", "-nm", "--noMasks");

	parser.add("", 0, 0, 0, "Don't create tileset. Use this if you are creating sprites and do not need a table with all the tile pointers.", "-nt", "--noTileset");

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

int processImage(const string& filename, vector<Tile>& tiles, ConversionOptions &convOptions, ezOptionParser &options) {
	TileExtractor extractor;
	extractor.Options = convOptions;
	vector<Tile> tmp = extractor.GetTiles(filename);
	for (Tile t : tmp) {
		//t.Dump();
		if (options.isSet("-g")) {
			t.GenImage();
			if (options.isSet("-t")) {
				t.GenMaskImage();
			}
		}
	}
	tiles.insert(tiles.end(), tmp.begin(), tmp.end());
	return 0;
}

int dumpTiles(vector<Tile>& tiles, ConversionOptions &convOptions) {
	if(!tiles.empty()) {
		OutputGenerator generator;	
		string outputName = convOptions.OutputFileName;
		string currentFileName;
		vector<Tile> currentTiles;
		vector<Tile>::iterator it = tiles.begin();

		do {
			if(convOptions.OneFilePerSourceFile) {
				currentFileName = it->SourceFileName;
				currentTiles.clear();
				stringstream ss;
				if(!outputName.empty()) {
					ss << outputName;					
				}
				ss << FileUtils::RemoveExtension(currentFileName);
				convOptions.OutputFileName = ss.str();

				while(it!=tiles.end() && it->SourceFileName == currentFileName) {
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

int extractPalette(ezOptionParser &options, TPalette &palette) {
	int result = 0;
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
	result = extractPalette(options, convOptions.Palette);
	if (!result) {
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
			cout << endl;
		}

		convOptions.ZigZag = options.isSet("-z");
		convOptions.CreateFlipLut = options.isSet("-f");
		convOptions.NoMaskData = options.isSet("-nm");

		if(options.isSet("--includes")) {
			if(convOptions.Format == ConversionOptions::PURE_C) {
				options.get("--includes")->getStrings(convOptions.AdditionalIncludes);
			} else {
				cout << "Warning: Additional includes for output format different than C files. Ignored." << endl;
			}
		}

		convOptions.InterlaceMasks = convOptions.Palette.TransparentIndex >= 0 && options.isSet("-im");
	}
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
	unsigned char andMaskLut[0x100], orMaskLut[0x100], flippedAndMaskLut[0x100], flippedOrMaskLut[0x100];
	unsigned char col1, col2, col3, col4, col5, col6, col7, col8;
	unsigned char maskCol1, maskCol2, maskCol3, maskCol4, maskCol5, maskCol6, maskCol7, maskCol8;
	unsigned char colorIndex;

	switch(convOptions.Mode) {
		case 0: 
		    for (col1 = 0; col1 < 0x10; ++col1) {
			    for (col2 = 0; col2 < 0x10; ++col2) {
		        	vector<unsigned char> colors = {col1, col2};
		            colorIndex = ColorCombinator::CombineColData(colors);
		            maskCol1 = (col1 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
		            maskCol2 = (col2 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
		        	vector<unsigned char> maskColors = {maskCol1, maskCol2};
		            andMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
		            flippedAndMaskLut[colorIndex] = ColorCombinator::FlipByteNum(convOptions.Mode, andMaskLut[colorIndex]);

		            maskCol1 = (col1 == convOptions.Palette.TransparentIndex) ? 0 : col1;
		            maskCol2 = (col2 == convOptions.Palette.TransparentIndex) ? 0 : col2;
		            maskColors = {maskCol1, maskCol2};
		            orMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
		            flippedOrMaskLut[colorIndex] = ColorCombinator::FlipByteNum(convOptions.Mode, orMaskLut[colorIndex]);
		        }
		    }
			break;			
		case 1:
			for (col1 = 0; col1 < 0x4; ++col1) {
			    for (col2 = 0; col2 < 0x4; ++col2) {
			    	for (col3 = 0; col3 < 0x4; ++col3) {
						for (col4 = 0; col4 < 0x4; ++col4) {
				        	vector<unsigned char> colors = {col1, col2, col3, col4};
				            colorIndex = ColorCombinator::CombineColData(colors);
				            maskCol1 = (col1 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
				            maskCol2 = (col2 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
				            maskCol3 = (col3 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
				            maskCol4 = (col4 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
				        	vector<unsigned char> maskColors = {maskCol1, maskCol2, maskCol3, maskCol4};
				            andMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
				            flippedAndMaskLut[colorIndex] = ColorCombinator::FlipByteNum(convOptions.Mode, andMaskLut[colorIndex]);

				            maskCol1 = (col1 == convOptions.Palette.TransparentIndex) ? 0 : col1;
				            maskCol2 = (col2 == convOptions.Palette.TransparentIndex) ? 0 : col2;
				            maskCol3 = (col3 == convOptions.Palette.TransparentIndex) ? 0 : col3;
				            maskCol4 = (col4 == convOptions.Palette.TransparentIndex) ? 0 : col4;
				            maskColors = {maskCol1, maskCol2, maskCol3, maskCol4};
				            orMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
				            flippedOrMaskLut[colorIndex] = ColorCombinator::FlipByteNum(convOptions.Mode, orMaskLut[colorIndex]);
				        }
				    }
				}
			}

		    break;
		case 2:
			for (col1 = 0; col1 < 0x2; ++col1) {
			    for (col2 = 0; col2 < 0x2; ++col2) {
			    	for (col3 = 0; col3 < 0x2; ++col3) {
						for (col4 = 0; col4 < 0x2; ++col4) {
							for (col5 = 0; col5 < 0x2; ++col5) {
								for (col6 = 0; col6 < 0x2; ++col6) {
									for (col7 = 0; col7 < 0x2; ++col7) {
										for (col8 = 0; col8 < 0x2; ++col8) {
								        	vector<unsigned char> colors = {col1, col2, col3, col4, col5, col6, col7, col8};
								            colorIndex = ColorCombinator::CombineColData(colors);
								            maskCol1 = (col1 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol2 = (col2 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol3 = (col3 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol4 = (col4 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol5 = (col5 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol6 = (col6 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol7 = (col7 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								            maskCol8 = (col8 == convOptions.Palette.TransparentIndex) ? ColorCombinator::MaxValueByMode(convOptions.Mode) : 0;
								        	vector<unsigned char> maskColors = {maskCol1, maskCol2, maskCol3, maskCol4, maskCol5, maskCol6, maskCol7, maskCol8};
								            andMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
								            flippedAndMaskLut[colorIndex] = ColorCombinator::FlipByteNum(convOptions.Mode, andMaskLut[colorIndex]);

								            maskCol1 = (col1 == convOptions.Palette.TransparentIndex) ? 0 : col1;
								            maskCol2 = (col2 == convOptions.Palette.TransparentIndex) ? 0 : col2;
								            maskCol3 = (col3 == convOptions.Palette.TransparentIndex) ? 0 : col3;
								            maskCol4 = (col4 == convOptions.Palette.TransparentIndex) ? 0 : col4;
								            maskCol5 = (col5 == convOptions.Palette.TransparentIndex) ? 0 : col5;
								            maskCol6 = (col6 == convOptions.Palette.TransparentIndex) ? 0 : col6;
								            maskCol7 = (col7 == convOptions.Palette.TransparentIndex) ? 0 : col7;
								            maskCol8 = (col8 == convOptions.Palette.TransparentIndex) ? 0 : col8;
								            maskColors = {maskCol1, maskCol2, maskCol3, maskCol4, maskCol5, maskCol6, maskCol7, maskCol8};
								            orMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
								            flippedOrMaskLut[colorIndex] = ColorCombinator::FlipByteNum(convOptions.Mode, orMaskLut[colorIndex]);
								        }
								    }
								}
							}
				        }
				    }
				}
			}

			break;
	}

    OutputGenerator generator;
	switch (convOptions.Format) {
		case ConversionOptions::ASSEMBLER:
			generator.DumpTableASM(andMaskLut, 0x100, "andMasks");
			generator.DumpTableASM(orMaskLut, 0x100, "orMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableASM(flippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableASM(flippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
		case ConversionOptions::ASSEMBLER_ASXXXX:
			generator.DumpTableASXXXX(andMaskLut, 0x100, "andMasks");
			generator.DumpTableASXXXX(orMaskLut, 0x100, "orMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableASXXXX(flippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableASXXXX(flippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
		case ConversionOptions::BINARY:
			generator.DumpTableBIN(andMaskLut, 0x100, "andMasks");
			generator.DumpTableBIN(orMaskLut, 0x100, "orMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableBIN(flippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableBIN(flippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
		case ConversionOptions::PURE_C:
			generator.DumpTableC(andMaskLut, 0x100, "andMasks");
			generator.DumpTableC(andMaskLut, 0x100, "andMasks");
		    if(convOptions.CreateFlipLut) {
				generator.DumpTableC(flippedAndMaskLut, 0x100, "flippedAndMasks");
				generator.DumpTableC(flippedOrMaskLut, 0x100, "flippedOrMasks");
		    }
			break;
	}
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

	ConversionOptions convOptions;
	int optionsResult = extractConversionOptions(options, convOptions);
	if (optionsResult) {
		cout << "Couldn't parse conversion options. Use img2cpc --help for more information." << endl;
		return optionsResult;
	}
		//convOptions.Dump();

	vector<string *> &lastArgs = options.lastArgs;
	vector<Tile> tiles;

	for (long int i = 0, li = lastArgs.size(); i < li; ++i) {
		string filename = *lastArgs[i];
		int result = processImage(filename, tiles, convOptions, options);
		if (result) {
			cout << "Error processing image: " << filename << endl;
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
	
	return 0;
}