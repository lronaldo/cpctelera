#include "TileExtractor.hpp"

vector<Tile> TileExtractor::GetTiles(const string &fileName) {
	string currentFileName = FileUtils::Sanitize(FileUtils::RemoveExtension(fileName));
	string baseName = FileUtils::Sanitize(this->Options.BaseName);

	vector<Tile> result;

	FIBITMAP *dib = FileUtils::LoadImage(fileName);
	if (dib != NULL) {
		unsigned int imageWidth = FreeImage_GetWidth(dib);
		unsigned int imageHeight = FreeImage_GetHeight(dib);

		unsigned int tileWidth = this->Options.TileWidth > 0 ? this->Options.TileWidth : imageWidth;
		unsigned int tileHeight = this->Options.TileHeight > 0 ? this->Options.TileHeight : imageHeight;

		this->ModeIncrement = this->getModeIncrement();

		int tileIdx = 0;

		for (unsigned int imgPosY = 0; imgPosY<imageHeight; imgPosY += tileHeight) {
			unsigned int remainingHeight = imageHeight - imgPosY;
			unsigned int realTileHeight = remainingHeight < tileHeight ? remainingHeight : tileHeight;

			this->TileHeight = realTileHeight;

			for (unsigned int imgPosX = 0; imgPosX<imageWidth; imgPosX += tileWidth) {
				unsigned int remainingWidth = imageWidth - imgPosX;
				unsigned int realTileWidth = remainingWidth < tileWidth ? remainingWidth : tileWidth;
				this->TileWidth = realTileWidth;

				FIBITMAP *tileBitmap = FreeImage_Copy(dib, imgPosX, imgPosY, imgPosX + realTileWidth, imgPosY + realTileHeight);
				Tile theTile = this->getTile(tileBitmap);
				theTile.SourceFileName = fileName;
				FreeImage_Unload(tileBitmap);

				theTile.TileHeight = this->TileHeight;
				theTile.TileWidth = this->TileWidth;
				theTile.Palette = this->Options.Palette;
				theTile.TileWidthInBytes = theTile.TileWidth / this->ModeIncrement;

				stringstream nameStream;
				if(!baseName.empty()) {
					nameStream << baseName << "_";
				}
				nameStream << currentFileName;
				if (realTileHeight != imageHeight || realTileWidth != imageWidth) {
					nameStream << "_" << tileIdx;
				}
				theTile.Name = nameStream.str();
				
				++tileIdx;
				result.push_back(theTile);
			}
		}
		FreeImage_Unload(dib);
	}
	else {
		cout << "Error loading image: " << fileName << endl;
	}
	return result;
}

Tile TileExtractor::getTile(FIBITMAP* bmp) {
	Tile result;
	int scanlineCount = this->Options.ScanlineOrder.size();
	int scanlineIdx = 0;
	for (unsigned int y = 0; y < this->TileHeight; ++y) {
		int charIdx = y / scanlineCount;
		bool flip = this->Options.ZigZag && ((y & 1) == 1);

		unsigned int rowIndex = 0;
		do {
			rowIndex = (charIdx * scanlineCount) + this->Options.ScanlineOrder[scanlineIdx];
			scanlineIdx = (scanlineIdx + 1) % scanlineCount;
		} while (rowIndex >= this->TileHeight);

		// Current line to process is rowIndex.
		for (unsigned int x = 0; x < this->TileWidth; x += this->ModeIncrement) {
			ColorAndMaskValues byteValues = getColorAndMask(bmp, x, rowIndex, flip);

			result.Data.push_back(byteValues.ColorByte);
			result.MaskData.push_back(byteValues.MaskByte);
			for (unsigned char palIdx : byteValues.ColorValues) {
				result.SourceValues.push_back(palIdx);
			}
			for (unsigned char maskIdx : byteValues.TransparentValues) {
				result.MaskValues.push_back(maskIdx);
			}
		}
	}
	return result;
}

ColorAndMaskValues TileExtractor::getColorAndMask(FIBITMAP* bmp, int x, int y, bool flip) {
	ColorAndMaskValues result;

	result.ColorValues = extractPixels(bmp, x, y, flip);
	result.TransparentValues = extractTransPixels(result.ColorValues);
	result.ColorByte = ColorCombinator::CombineColData(result.ColorValues);
	result.MaskByte = ColorCombinator::CombineColData(result.TransparentValues);
	return result;
}

vector<unsigned char> TileExtractor::extractPixels(FIBITMAP* bmp, int x, int y, bool flip) {
	vector<unsigned char> result;
	for (unsigned int offset = 0; offset < this->ModeIncrement; ++offset) {
		if ((x + offset) < this->TileWidth)
		{
			int sourceX = (flip) ? this->TileWidth - x - this->ModeIncrement + offset : x + offset;
			RGBQUAD rgb;
			FreeImage_GetPixelColor(bmp, sourceX, this->TileHeight - y - 1, &rgb);
			Color current = Color(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
			unsigned char color = (unsigned char) this->Options.Palette.getNearestIndex(current);
			result.push_back(color);
		}
		else {
			result.push_back((unsigned char)this->Options.Palette.TransparentIndex);
		}
	}
	return result;
}

vector<unsigned char> TileExtractor::extractTransPixels(vector<unsigned char> colors) {
	vector<unsigned char> result;
	for (unsigned char colorValue : colors) {
		if (colorValue == this->Options.Palette.TransparentIndex) {
			result.push_back(ColorCombinator::MaxValueByMode(this->Options.Mode));
		}
		else {
			result.push_back(0);
		}
	}
	return result;
}

unsigned int TileExtractor::getModeIncrement() {
	switch (this->Options.Mode) {
	case 0:
		return 2;
	case 1:
		return 4;
	case 2:
		return 8;
	default:
		throw "Mode not valid.";
	}
}