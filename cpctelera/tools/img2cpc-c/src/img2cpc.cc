
#include "img2cpc.hpp"

int initializeParser(ezOptionParser &parser) {
	parser.overview = "img2cpc - (c) 2007-2015 Retroworks.";
	parser.syntax = "img2cpc [OPTIONS] fileNames";
	parser.example = "img2cpc -w 8 -h 8 --outputFormat asm -bn tile -m 0 tiles.png\n";
	parser.footer = "If you liked this program, drop an email at: augusto.ruiz@gmail.com\n";

	parser.add("", 0, 1, 0, "Tile width. If not set, the image width is used.", "-w", "--tileWidth");
	parser.add("", 0, 1, 0, "Tile height. If not set, the image height is used.", "-h", "--tileHeight");
	parser.add("", 0, 1, 0, "Tile base name. If not set, the filename will be used.", "-bn", "--baseName");

	parser.add("asm", 0, 1, 0, "Output format. If not set, data will be generated in assembly format.", "-of", "--outputFormat");
	parser.add("", 0, 0, 0, "Create a flipped values look-up table for the current palette and mode.", "-f");

	parser.add("0", 0, 1, 0, "Specifies the CPC Mode to generate data for. Valid values are 0 (default), 1 or 2.", "-m", "--mode");
	parser.add("", 0, 0, 0, "Generate tile map", "-map", "--tilemap");
	parser.add("gfx", 0, 1, 0, "Output file name. Default value is gfx. img2cpc will append the proper extension based on format.", "-o", "--outputFileName");
	parser.add("", 0, 1, ',', "Scanline order. Default value is 01234567", "-s", "--scanlineOrder");
	parser.add("", 0, 0, 0, "Zigzag. Generate data in zigzag order.", "-z", "--zigzag");

  parser.add("", 0, 1, ',', "Palette specified in firmware values.", "-fwp", "--firmwarePalette");
  parser.add("", 0, 1, ',', "Palette specified in RGB values. RGB values must be specified as 0xRRGGBB, where RR, GG and BB are hexadecimal, or as an int value.\nExamples: 0x1122FF, 255", "-rgbp", "--rgbPalette");
  parser.add("", 0, 1, ',', "Palette specified in hardware values.", "-hwp", "--hardwarePalette");
  parser.add("", 0, 1, 0, "Specifies input palette file.", "-p", "--palette");
  parser.add("", 0, 1, 0, "Specifies transparent color (as index in palette).", "-t", "--transparentColor");
  parser.add("", 0, 0, 0, "Interlaced masks. Mask values will be interlaced with pixel values.", "-im", "--interlacedMasks");

  parser.add("", 0, 0, 0, "Output palette (hardware values)", "-ophw");
  parser.add("", 0, 0, 0, "Output palette (firmware values)", "-opfw");

  parser.add("", 0, 0, 0, "Generates PNG images to check tile output.", "-g", "--generatePNG");

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

int processImage(const string& filename, ConversionOptions &convOptions, ezOptionParser &options) {
  TileExtractor extractor;
  extractor.Options = convOptions;
  vector<Tile> tiles = extractor.GetTiles(filename);
  for(Tile t:tiles) {
    //t.Dump();
    if(options.isSet("-g")) {
      t.GenImage();
      if(options.isSet("-t")) {
        t.GenMaskImage();
      }
    }
  }

  OutputGenerator generator;
  switch(convOptions.Format) {
    case ConversionOptions::ASSEMBLER: 
      generator.GenerateASM(tiles, convOptions);
      break;
    case ConversionOptions::BINARY: 
      generator.GenerateBIN(tiles, convOptions);
      break;
    case ConversionOptions::PURE_C: 
      generator.GenerateC(tiles, convOptions);
      generator.GenerateH(tiles, convOptions);
      break;
  }

  return 0;
}

int extractPalette(ezOptionParser &options, TPalette &palette) {
  int result = 0;
  if(!options.isSet("-fwp") && !options.isSet("-rgbp") && !options.isSet("-hwp") && !options.isSet("-p")) {
    return -1;
  }
  if(options.isSet("-fwp")) {
    // Palette specified in firmware indexes.
    vector<string> fwIndices;
    options.get("-fwp")->getStrings(fwIndices);
    result = palette.ParseFW(fwIndices);
  }
  else if(options.isSet("-rgbp")) {
    // Palette specified as rgb values.
    vector<string> rgbValues;
    options.get("-rgbp")->getStrings(rgbValues);
    result = palette.ParseRGB(rgbValues);
  } else if(options.isSet("-hwp")) {
    // Palette specified as hardware values.
    vector<string> hwIndices;
    options.get("-hwp")->getStrings(hwIndices);
    result = palette.ParseHW(hwIndices);
  } else if(options.isSet("-p")) {
    string paletteFile;
    options.get("-p")->getString(paletteFile);
    result = palette.ParseFile(paletteFile);
  }

  if(options.isSet("-t")) {
    int maxValue = palette.Current.size();
    int value;
    options.get("-t")->getInt(value);
    palette.TransparentIndex = value;
    if(palette.TransparentIndex < 0 || palette.TransparentIndex >= maxValue) {
      cout << "Transparent color must be between 0 and " << maxValue - 1 << "." << endl;
      result = -1;
    }
  }
  return result;
}

int extractConversionOptions(ezOptionParser &options, ConversionOptions &convOptions) {
  int result = 0;
  result = extractPalette(options, convOptions.Palette);
  if(!result) {
      convOptions.PaletteFormat = ConversionOptions::NONE;
      if(options.isSet("-ophw")) {
        convOptions.PaletteFormat = ConversionOptions::HARDWARE;
      } else if(options.isSet("-opfw")) {
        convOptions.PaletteFormat = ConversionOptions::FIRMWARE;
      }

    //convOptions.Palette.Dump();
    if(options.isSet("-w")) {
      options.get("-w")->getInt(convOptions.TileWidth); 
    }
    if(options.isSet("-h")) {
      options.get("-h")->getInt(convOptions.TileHeight);
    }
    if(options.isSet("-bn")) {
      options.get("-bn")->getString(convOptions.BaseName);
    }

    string outputFmt;
    options.get("-of")->getString(outputFmt);
    //cout << outputFmt << endl;
    convOptions.ParseFormat(outputFmt);

    options.get("-m")->getInt(convOptions.Mode);
    if(convOptions.Mode < 0 || convOptions.Mode > 2) {
      result = -1;
      cout << "Error." << endl
           << "CPC Mode must be 0, 1 or 2." << endl;
    }

    options.get("-o")->getString(convOptions.OutputFileName);

    if(options.isSet("-s")) {
      convOptions.ScanlineOrder.clear();
      options.get("-s")->getInts(convOptions.ScanlineOrder);
      cout << endl;
    }

    if(options.isSet("-z")) {
      convOptions.ZigZag = true;
    }

    convOptions.InterlaceMasks = convOptions.Palette.TransparentIndex >= 0 && options.isSet("-im");
  }
  return result;
}

int main(int argc, const char** argv)
{
  if(initializeImageLoader()) {
  	return -1;
  }
  ezOptionParser options;
  if(initializeParser(options)) {
  	return -1;
  }

  options.parse(argc, argv);

  if(options.isSet("--help")) {
    showUsage(options);
  	return 0;
  }

  ConversionOptions convOptions;
  int optionsResult = extractConversionOptions(options, convOptions);
  if(optionsResult) {
    cout << "Couldn't parse conversion options. Use img2cpc --help for more information." << endl;
    return optionsResult;
  }
  //convOptions.Dump();

  vector<string *> &lastArgs = options.lastArgs;
  for(long int i=0, li=lastArgs.size(); i < li; ++i) {
    string filename = *lastArgs[i];
    int result = processImage(filename, convOptions, options);
    if(result) {
    	cout << "Error processing image: " << filename << endl;
    	return result;
    }
  }

  return 0;
}