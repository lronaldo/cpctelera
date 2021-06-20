#include "Palette.hpp"
#include <limits>
#include <algorithm>

TPalette::TPalette() {
	UpdateMaxEntries(0, false);
}

void TPalette::UpdateMaxEntries(int mode, bool usesMasks) {
	switch(mode) {
		case 0:
			_maxEntries = 16;
			break;
		case 1:
			_maxEntries = 4;
			break;
		case 2:
			_maxEntries = 2;
			break;
	}
	if(usesMasks) { _maxEntries++; };
}

int TPalette::getNearestIndex(Color &color) {
	int paletteIdx = -1;
	double currentDistance = numeric_limits<double>::max();
	for (int i = 0, li = this->Current.size(); i<li; ++i) {
		double dist = color.Distance(this->Current[i]);
		if (dist < currentDistance) {
			currentDistance = dist;
			paletteIdx = i;
		}
	}
	return paletteIdx;
};

vector<int> TPalette::GetPaletteAsFW() {
	vector<int> result;
	for (Color color : this->Current) {
		double currentDistance = numeric_limits<double>::max();
		int fwColor = -1;
		for (int fwIdx = 0, lFwIdx = TPalette::Firmware.size(); fwIdx<lFwIdx; ++fwIdx) {
			double dist = color.Distance(TPalette::Firmware[fwIdx]);
			if (dist < currentDistance) {
				currentDistance = dist;
				fwColor = fwIdx;
			}
		}
		result.push_back(fwColor);
	}
	return result;
};

vector<int> TPalette::GetPaletteAsHW() {
	vector<int> result;
	for (Color color : this->Current) {
		double currentDistance = numeric_limits<double>::max();
		int hwColor = -1;
		map<int, Color&>::const_iterator it;
		for (it = TPalette::Hardware.begin(); it != TPalette::Hardware.end(); ++it) {
			double dist = color.Distance(it->second);
			if (dist < currentDistance) {
				currentDistance = dist;
				hwColor = it->first;
			}
		}
		result.push_back(hwColor);
	}
	return result;
};

int TPalette::ParseFW(vector<string> &fwIndices) {
	int fwIndicesSize = fwIndices.size();
	if (fwIndicesSize < 1 || fwIndicesSize > _maxEntries) {
		cout << "Error." << endl
			<< "There must be at least one palette value and " << _maxEntries << " palette values maximum." << endl;
		return -2;
	}
	int fwPalSize = TPalette::Firmware.size();
	for (string fwIdx : fwIndices) {
		long int realIdx = strtol(fwIdx.c_str(), nullptr, 0);
		if (realIdx >= 0 && realIdx < fwPalSize) {
			this->Current.push_back(TPalette::Firmware[realIdx]);
		}
		else {
			cout << "Error." << endl
				<< "Palette index " << fwIdx << " is out of bounds [0," << fwPalSize - 1 << "]" << endl;
			return -2;
		}
	}
	return 0;
};

int TPalette::ParseHW(vector<string> &hwIndices) {
	int hwIndicesSize = hwIndices.size();
	if (hwIndicesSize < 1 || hwIndicesSize > _maxEntries) {
		cout << "Error." << endl
			<< "There must be at least one palette value and " << _maxEntries << " palette values maximum." << endl;
		return -2;
	}
	map<int, Color&> &hwPalette = TPalette::Hardware;
	for (string hwIdx : hwIndices) {

		long int realIdx = strtol(hwIdx.c_str(), nullptr, 0);

		if (hwPalette.count(realIdx) != 0) {
			this->Current.push_back(hwPalette.at(realIdx));
		}
		else {
			cout << "Error." << endl
				<< "Hardware color value " << hwIdx << " is not valid." << endl;
			return -2;
		}
	}
	return 0;
};

int TPalette::ParseRGB(vector<string> &rgbValues) {
	//cout << "RGB!!";

	int rgbValuesSize = rgbValues.size();
	if (rgbValuesSize < 1 || rgbValuesSize > _maxEntries) {
		cout << "Error." << endl
			<< "There must be at least one palette value and " << _maxEntries << " palette values maximum." << endl;
		return -2;
	}
	for (string val : rgbValues) {
		long int intValue = strtol(val.c_str(), nullptr, 0);
		Color currentColor((unsigned char)((intValue & 0xFF0000) >> 16), (unsigned char)((intValue & 0xFF00) >> 8), (unsigned char)(intValue & 0xFF));
		this->Current.push_back(currentColor);
	}
	return 0;
};

int TPalette::ParseFile(string &fileName) {
	ifstream file(fileName, ifstream::binary);
	Json::Value root;
	file >> root;

	return this->FromJSON(root);
};

int TPalette::FromJSON(Json::Value &root) {
	string type = root.get("type", "rgb").asString();
	transform(type.begin(), type.end(), type.begin(), ::tolower);

	vector<string> valuesVector;
	const Json::Value values = root["values"];
	if (values != Json::Value::null) {
		for (int i = 0, li = values.size(); i < li; ++i) {
			valuesVector.push_back(values[i].asString());
		}

		if (type == "firmware") {
			this->ParseFW(valuesVector);
		}
		else if (type == "hardware") {
			this->ParseHW(valuesVector);
		}
		else if (type == "rgb") {
			this->ParseRGB(valuesVector);
		}
		else {
			cout << "Error." << endl
				<< "Palette type '" << root["type"] << "' not supported." << endl
				<< "Supported types are 'firmware','hardware' and 'rgb'." << endl;
			return -1;
		}
	}
	else {
		cout << "Error." << endl
			<< "No values specified for palette." << endl;
		return -1;
	}

	Json::Value transValue = root["transparent"];
	if (transValue != Json::Value::null) {
		int maxValue = this->Current.size();
		int value = transValue.asInt();
		this->TransparentIndex = value;
		if (this->TransparentIndex < 0 || this->TransparentIndex >= maxValue) {
			cout << "Error." << endl << "Transparent color must be between 0 and " << maxValue - 1 << "." << endl;
			return -1;
		}
	}

	return 0;
};

Json::Value TPalette::ToJSON() {
	Json::Value result;

	result["type"] = "rgb";
	Json::Value values;
	for (Color c : this->Current) {
		values.append(c.toInt());
	}
	result["values"] = values;
	result["transparent"] = this->TransparentIndex;

	return result;
};

void TPalette::Dump() {
	cout << "[" << endl;
	for (Color c : this->Current) {
		cout << "\t";
		c.Dump();
		cout << endl;
	}
	cout << "]" << endl;

	if (this->TransparentIndex >= 0) {
		cout << "Transparent color is: ";
		this->Current[this->TransparentIndex].Dump();
		cout << endl;
	}
};

vector<Color> TPalette::Firmware = {
	Color(0, 0, 0),
	Color(0, 0, 128),
	Color(0, 0, 255),
	Color(128, 0, 0),
	Color(128, 0, 128),
	Color(128, 0, 255),
	Color(255, 0, 0),
	Color(255, 0, 128),
	Color(255, 0, 255),
	Color(0, 128, 0),
	Color(0, 128, 128),
	Color(0, 128, 255),
	Color(128, 128, 0),
	Color(128, 128, 128),
	Color(128, 128, 255),
	Color(255, 128, 0),
	Color(255, 128, 128),
	Color(255, 128, 255),
	Color(0, 255, 0),
	Color(0, 255, 128),
	Color(0, 255, 255),
	Color(128, 255, 0),
	Color(128, 255, 128),
	Color(128, 255, 255),
	Color(255, 255, 0),
	Color(255, 255, 128),
	Color(255, 255, 255)
};

map <int, Color&> TPalette::Hardware = {
	{ 0x54, Firmware[0] },
	{ 0x44, Firmware[1] },
	{ 0x50, Firmware[1] },
	{ 0x55, Firmware[2] },
	{ 0x5C, Firmware[3] },
	{ 0x58, Firmware[4] },
	{ 0x5D, Firmware[5] },
	{ 0x4C, Firmware[6] },
	{ 0x45, Firmware[7] },
	{ 0x48, Firmware[7] },
	{ 0x4D, Firmware[8] },
	{ 0x56, Firmware[9] },
	{ 0x46, Firmware[10] },
	{ 0x57, Firmware[11] },
	{ 0x5E, Firmware[12] },
	{ 0x40, Firmware[13] },
	{ 0x41, Firmware[13] },
	{ 0x5F, Firmware[14] },
	{ 0x4E, Firmware[15] },
	{ 0x47, Firmware[16] },
	{ 0x4F, Firmware[17] },
	{ 0x52, Firmware[18] },
	{ 0x42, Firmware[19] },
	{ 0x51, Firmware[19] },
	{ 0x53, Firmware[20] },
	{ 0x5A, Firmware[21] },
	{ 0x59, Firmware[22] },
	{ 0x5B, Firmware[23] },
	{ 0x4A, Firmware[24] },
	{ 0x43, Firmware[25] },
	{ 0x49, Firmware[25] },
	{ 0x4B, Firmware[26] }
};