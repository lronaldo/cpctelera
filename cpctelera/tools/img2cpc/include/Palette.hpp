#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <json/json.h>
#include "Color.hpp"

using namespace std;

class TPalette {
public:
	TPalette();
	vector<Color> Current;
	int TransparentIndex = -1;

	int getNearestIndex(Color &color);

	int ParseFW(vector<string> &fwIndices);
	int ParseHW(vector<string> &hwIndices);
	int ParseRGB(vector<string> &rgbValues);

	int ParseFile(string &fileName);
	int FromJSON(Json::Value &root);
	Json::Value ToJSON();

	void Dump();

	static vector<Color> Firmware;
	static map <int, Color&> Hardware;

	vector<int> GetPaletteAsFW();
	vector<int> GetPaletteAsHW();

	void UpdateMaxEntries(int mode, bool usesMasks);

private:
	int _maxEntries;
};

#endif