#ifndef _CONVERSION_OPTIONS_H_
#define _CONVERSION_OPTIONS_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <json/json.h>
#include "ColorCombinator.hpp"

using namespace std;

#define PALETTE_JSON_KEY				"palette"
#define TILEWIDTH_JSON_KEY				"tileWidth"
#define TILEHEIGHT_JSON_KEY				"tileHeight"
#define BASENAME_JSON_KEY				"baseName"
#define ABSOLUTE_BASENAME_JSON_KEY		"absoluteBaseName"
#define FORMAT_JSON_KEY 				"format"
#define MODE_JSON_KEY 					"mode"
#define OUTPUT_FILE_NAME_JSON_KEY 		"outputFileName"
#define SCANLINE_ORDER_JSON_KEY 		"scanlineOrder"
#define ZIGZAG_JSON_KEY 				"zigZag"
#define HALFFLIP_JSON_KEY 				"halfFlip"
#define OUTPUT_SIZE_JSON_KEY 			"outputSize"
#define INTERLACE_MASKS_JSON_KEY		"interlaceMasks"
#define CREATE_TILESET_JSON_KEY			"createTileset"
#define CREATE_FLIP_LUT_JSON_KEY		"createFlipLut"
#define NO_MASK_DATA_JSON_KEY			"noMaskData"
#define ONE_FILE_PER_SOURCE_JSON_KEY	"oneFilePerSource"
#define RLE_JSON_KEY					"rle"
#define IS_SCR_JSON_KEY					"isScr"

class ConversionOptions {
public:
	ConversionOptions() : TileWidth(-1),
		TileHeight(-1),
		Format(ASSEMBLER),
		Mode(0),
		ScanlineOrder{ 0,1,2,3,4,5,6,7 },
		ZigZag(false),
		CreateFlipLut(false),
		IsScr(false) { };

	enum OutputFormat { ASSEMBLER = 0, ASSEMBLER_ASXXXX = 1, BINARY = 2, PURE_C = 3 };
	enum OutputPalette { NONE, FIRMWARE, HARDWARE };
	enum ByteOrder { ROW, COLUMN };

	inline static const char* ToString(OutputFormat f) {
		switch (f) {
		case ASSEMBLER: return "asm";
		case ASSEMBLER_ASXXXX: return "s";
		case BINARY: return "bin";
		case PURE_C: return "c";
		default: return "unknown";
		}
	}

	inline static const char* ToString(OutputPalette p) {
		switch (p) {
		case NONE: return "none";
		case FIRMWARE: return "firmware";
		case HARDWARE: return "hardware";
		default: return "unknown";
		}
	}

	TPalette Palette;

	int TileWidth;
	int TileHeight;
	string BaseName;
	bool AbsoluteBaseName;

	OutputFormat Format;
	int Mode;
	string OutputFileName;
	vector<int> ScanlineOrder;
	bool ZigZag;
	bool HalfFlip;
	bool InterlaceMasks;
	bool OutputSize;
	OutputPalette PaletteFormat;

	bool CreateTileset;
	bool CreateFlipLut;
	bool NoMaskData;
	bool OneFilePerSourceFile;
	bool RLE;
	bool IsScr;

	ByteOrder PixelOrder;

	vector<string> AdditionalIncludes;

	unsigned char AndMaskLut[0x100];
	unsigned char OrMaskLut[0x100];
	unsigned char FlippedAndMaskLut[0x100];
	unsigned char FlippedOrMaskLut[0x100];

	ByteOrder ParseByteOrder(const string &byteOrderString) {
		string orderLower(byteOrderString);
		transform(orderLower.begin(), orderLower.end(), orderLower.begin(), ::tolower);
		if (orderLower == "row") this->PixelOrder = ROW;
		else if(orderLower == "col") this->PixelOrder = COLUMN;
		return this->PixelOrder;
	}

	OutputFormat ParseFormat(const string &formatString) {
		string fmtLower(formatString);
		transform(fmtLower.begin(), fmtLower.end(), fmtLower.begin(), ::tolower);

		//cout << fmtLower << endl;

		if (fmtLower == "asm") this->Format = ASSEMBLER;
		else if(fmtLower == "s") this->Format = ASSEMBLER_ASXXXX;
		else if (fmtLower == "bin") this->Format = BINARY;
		else if (fmtLower == "c") this->Format = PURE_C;

		return this->Format;
	};

	Json::Value ToJson() {
		Json::Value root;
		root[PALETTE_JSON_KEY] = this->Palette.ToJSON();
		root[TILEWIDTH_JSON_KEY] = this->TileWidth;
		root[TILEHEIGHT_JSON_KEY] = this->TileHeight;
		root[BASENAME_JSON_KEY] = this->BaseName;
		root[ABSOLUTE_BASENAME_JSON_KEY] = this->AbsoluteBaseName;
		root[FORMAT_JSON_KEY] = ToString(this->Format);
		root[MODE_JSON_KEY] = this->Mode;
		root[OUTPUT_FILE_NAME_JSON_KEY] = this->OutputFileName;
		root[SCANLINE_ORDER_JSON_KEY] = Json::Value();
		for (int s : this->ScanlineOrder) {
			root[SCANLINE_ORDER_JSON_KEY].append(s);
		}
		root[ZIGZAG_JSON_KEY] = this->ZigZag;
		root[HALFFLIP_JSON_KEY] = this->HalfFlip;
		root[OUTPUT_SIZE_JSON_KEY] = this->OutputSize;
		root[INTERLACE_MASKS_JSON_KEY] = this->InterlaceMasks;
		root[CREATE_TILESET_JSON_KEY] = this->CreateTileset;
		root[CREATE_FLIP_LUT_JSON_KEY] = this->CreateFlipLut;
		root[NO_MASK_DATA_JSON_KEY] = this->NoMaskData;
		root[ONE_FILE_PER_SOURCE_JSON_KEY] = this->OneFilePerSourceFile;
		root[RLE_JSON_KEY] = this->RLE;
		root[IS_SCR_JSON_KEY] = this->IsScr;
		return root;
	};

	void FromJson(Json::Value root) {
		this->Palette.FromJSON(root[PALETTE_JSON_KEY]);
		this->TileWidth = root[TILEWIDTH_JSON_KEY].asInt();
		this->TileHeight = root[TILEHEIGHT_JSON_KEY].asInt();
		this->BaseName = root[BASENAME_JSON_KEY].asString();
		this->AbsoluteBaseName = root[ABSOLUTE_BASENAME_JSON_KEY].asBool();
		this->Format = ParseFormat(root[FORMAT_JSON_KEY].asString());
		this->Mode = root[MODE_JSON_KEY].asInt();
		this->OutputFileName = root[OUTPUT_FILE_NAME_JSON_KEY].asString();
		this->ScanlineOrder.clear();
		Json::Value scanlines = root[SCANLINE_ORDER_JSON_KEY];
		for (int i = 0, li = scanlines.size(); i<li; ++i) {
			this->ScanlineOrder.push_back(scanlines[i].asInt());
		}
		this->ZigZag = root[ZIGZAG_JSON_KEY].asBool();
		this->HalfFlip = root[HALFFLIP_JSON_KEY].asBool();
		this->OutputSize = root[OUTPUT_SIZE_JSON_KEY].asBool();
		this->InterlaceMasks = root[INTERLACE_MASKS_JSON_KEY].asBool();
		this->CreateTileset = root[CREATE_TILESET_JSON_KEY].asBool();
		this->CreateFlipLut = root[CREATE_FLIP_LUT_JSON_KEY].asBool();
		this->NoMaskData = root[NO_MASK_DATA_JSON_KEY].asBool();
		this->OneFilePerSourceFile = root[ONE_FILE_PER_SOURCE_JSON_KEY].asBool();
		this->RLE = root[RLE_JSON_KEY].asBool();
		this->IsScr = root[IS_SCR_JSON_KEY].asBool();
		this->InitLutTables();
	};

	void Dump() {
		cout << this->ToJson() << endl;
	}

	void InitLutTables() {
		unsigned char colorIndex;
		unsigned char pixelsPerByte = 2 * (int) pow(2, this->Mode);
		unsigned char maxIndex = (int) pow(2, (8 / pixelsPerByte));

		vector<unsigned char> colors;
		vector<unsigned char> maskColors;
		for(int colByte = 0; colByte < 256; ++colByte) {
			unsigned char byte = colByte;
			unsigned char mask = maxIndex - 1;
			colors.clear();
			for(int i=0; i<pixelsPerByte; ++i) {
				colors.push_back(byte & mask);
				byte = byte >> (8 / pixelsPerByte);
			}
	        colorIndex = ColorCombinator::CombineColData(colors);
	        maskColors.clear();
			for(int i=0; i<pixelsPerByte; ++i) {
				maskColors.push_back(colors[i] == this->Palette.TransparentIndex ? ColorCombinator::MaxValueByMode(this->Mode) : 0);
			}
			this->AndMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
	        this->FlippedAndMaskLut[colorIndex] = ColorCombinator::FlipByteNum(this->Mode, this->AndMaskLut[colorIndex]);
	        maskColors.clear();
			for(int i=0; i<pixelsPerByte; ++i) {
				maskColors.push_back(colors[i] == this->Palette.TransparentIndex ? 0 : colors[i]);
			}
	        this->OrMaskLut[colorIndex] = ColorCombinator::CombineColData(maskColors);
	        this->FlippedOrMaskLut[colorIndex] = ColorCombinator::FlipByteNum(this->Mode, this->OrMaskLut[colorIndex]);
		}
	}
};

#endif