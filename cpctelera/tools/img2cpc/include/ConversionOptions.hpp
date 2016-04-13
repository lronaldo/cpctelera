#ifndef _CONVERSION_OPTIONS_H_
#define _CONVERSION_OPTIONS_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <json/json.h>

using namespace std;

#define PALETTE_JSON_KEY			"palette"
#define TILEWIDTH_JSON_KEY			"tileWidth"
#define TILEHEIGHT_JSON_KEY			"tileHeight"
#define BASENAME_JSON_KEY			"baseName"
#define ABSOLUTE_BASENAME_JSON_KEY	"absoluteBaseName"
#define FORMAT_JSON_KEY 			"format"
#define MODE_JSON_KEY 				"mode"
#define OUTPUT_FILE_NAME_JSON_KEY 	"outputFileName"
#define SCANLINE_ORDER_JSON_KEY 	"scanlineOrder"
#define ZIGZAG_JSON_KEY 			"zigZag"
#define OUTPUT_SIZE_JSON_KEY 		"outputSize"

class ConversionOptions {
public:
	ConversionOptions() : TileWidth(-1),
		TileHeight(-1),
		Format(ASSEMBLER),
		Mode(0),
		ScanlineOrder{ 0,1,2,3,4,5,6,7 },
<<<<<<< HEAD
		ZigZag(false) { };
=======
		ZigZag(false),
		CreateFlipLut(false) { };
>>>>>>> img2cpc_c

	enum OutputFormat { ASSEMBLER, ASSEMBLER_ASXXXX, BINARY, PURE_C };
	enum OutputPalette { NONE, FIRMWARE, HARDWARE };

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
	bool InterlaceMasks;
	bool OutputSize;
	OutputPalette PaletteFormat;

	bool CreateTileset;
<<<<<<< HEAD
	bool OneFilePerSourceFile;

=======
	bool CreateFlipLut;
	bool NoMaskData;
	bool OneFilePerSourceFile;

	vector<string> AdditionalIncludes;

>>>>>>> img2cpc_c
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
<<<<<<< HEAD
		root["palette"] = this->Palette.ToJSON();
		root["tileWidth"] = this->TileWidth;
		root["tileHeight"] = this->TileHeight;
		root["baseName"] = this->BaseName;
		root["format"] = ToString(this->Format);
		root["mode"] = this->Mode;
		root["outputFileName"] = this->OutputFileName;
		root["scanlineOrder"] = Json::Value();
		for (int s : this->ScanlineOrder) {
			root["scanlineOrder"].append(s);
=======
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
>>>>>>> img2cpc_c
		}
		root[ZIGZAG_JSON_KEY] = this->ZigZag;
		root[OUTPUT_SIZE_JSON_KEY] = this->OutputSize;
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
<<<<<<< HEAD
		Json::Value scanlines = root["scanlineOrder"];
=======
		Json::Value scanlines = root[SCANLINE_ORDER_JSON_KEY];
>>>>>>> img2cpc_c
		for (int i = 0, li = scanlines.size(); i<li; ++i) {
			this->ScanlineOrder.push_back(scanlines[i].asInt());
		}
		this->ZigZag = root[ZIGZAG_JSON_KEY].asBool();
		this->OutputSize = root[OUTPUT_SIZE_JSON_KEY].asBool();
	};

	void Dump() {
		cout << this->ToJson() << endl;
	}
};

#endif