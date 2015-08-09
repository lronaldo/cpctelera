#ifndef _CONVERSION_OPTIONS_H_
#define _CONVERSION_OPTIONS_H_

#include <iostream>
#include <algorithm>
#include <vector>
#include <json/json.h>

using namespace std;

class ConversionOptions {
public:
	ConversionOptions() : TileWidth(-1), 
						  TileHeight(-1),  
						  Format(ASSEMBLER),
						  Mode(0),
						  ScanlineOrder {0,1,2,3,4,5,6,7},
						  ZigZag(false) { };

	enum OutputFormat { ASSEMBLER, BINARY, PURE_C };

	inline const char* ToString(OutputFormat f) {
		switch(f) {
			case ASSEMBLER: return "asm";
			case BINARY: return "bin";
			case PURE_C: return "c";
			default: return "unknown";
		}
	}

	TPalette Palette;

	int TileWidth;
	int TileHeight;
	string BaseName;

	OutputFormat Format;
	int Mode;
	string OutputFileName;
	vector<int> ScanlineOrder;
	bool ZigZag;
	bool InterlaceMasks;

	OutputFormat ParseFormat(const string &formatString) {
		string fmtLower(formatString);
		transform(fmtLower.begin(), fmtLower.end(), fmtLower.begin(), ::tolower);
		
		//cout << fmtLower << endl;

		if(fmtLower == "asm") this->Format = ASSEMBLER;
		else if(fmtLower == "bin") this->Format = BINARY;
		else if(fmtLower == "c") this->Format = PURE_C;

		return this->Format;
	};

	Json::Value ToJson() {
		Json::Value root;
		root["palette"] = this->Palette.ToJSON();
		root["tileWidth"] = this->TileWidth;
		root["tileHeight"] = this->TileHeight;
		root["baseName"] = this->BaseName;
		root["format"] = ToString(this->Format);
		root["mode"] = this->Mode;
		root["outputFileName"] = this->OutputFileName;
		root["scanlineOrder"] = Json::Value();
		for(int s : this->ScanlineOrder) {
			root["scanlineOrder"].append(s);
		}
		root["zigZag"] = this->ZigZag;
		return root;
	};

	void FromJson(Json::Value root) {
		this->Palette.FromJSON(root["palette"]);
		this->TileWidth = root["tileWidth"].asInt();
		this->TileHeight = root["tileHeight"].asInt();
		this->BaseName = root["baseName"].asString();
		this->Format = ParseFormat(root["format"].asString());
		this->Mode = root["mode"].asInt();
		this->OutputFileName = root["outputFileName"].asString();
		this->ScanlineOrder.clear();
		Json::Value scanlines = root["scanlineOrder"];
		for ( int i=0, li=scanlines.size(); i<li; ++i) {
		   this->ScanlineOrder.push_back(scanlines[i].asInt());
		}
		this->ZigZag = root["zigzag"].asBool();
	};

	void Dump() {
		cout << this->ToJson() << endl;
	}
};

#endif