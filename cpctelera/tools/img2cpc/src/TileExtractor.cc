#include "TileExtractor.hpp"

vector<Tile*> TileExtractor::GetTiles(const string &fileName) {
	string currentFileName = FileUtils::Sanitize(FileUtils::RemoveExtension(fileName));
	string baseName = FileUtils::Sanitize(this->Options.BaseName);

	vector<Tile*> result;

	FIBITMAP *dib = FileUtils::LoadImage(fileName);
	cout << "Processing image '" << fileName << "'... ";
	if (dib != NULL) {
		unsigned int imageWidth = FreeImage_GetWidth(dib);
		unsigned int imageHeight = FreeImage_GetHeight(dib);

		unsigned int tileWidth = this->Options.TileWidth > 0 ? this->Options.TileWidth : imageWidth;
		unsigned int tileHeight = this->Options.TileHeight > 0 ? this->Options.TileHeight : imageHeight;

		unsigned int numTilesY = (imageHeight / tileHeight) + ((imageHeight % tileHeight) > 0 ? 1 : 0);
		unsigned int numTilesX = (imageWidth / tileWidth) + ((imageWidth % tileWidth) > 0 ? 1 : 0);
		unsigned int totalTiles = numTilesY * numTilesX;

		unsigned int numDigits = (int) log10 ((double) totalTiles) + 1;

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
				Tile* theTile = this->getTile(tileBitmap);
				theTile->SourceFileName = fileName;
				FreeImage_Unload(tileBitmap);

				theTile->TileHeight = this->TileHeight;
				theTile->TileWidth = this->TileWidth;
				theTile->Palette = this->Options.Palette;
				theTile->TileWidthInBytes = theTile->TileWidth / this->ModeIncrement;

				stringstream nameStream;
				if(!baseName.empty()) {
					nameStream << baseName;;
					if(!this->Options.AbsoluteBaseName) {
						nameStream  << "_" << currentFileName;					
					}
				} else {
					nameStream << currentFileName;
				}
				if (realTileHeight != imageHeight || realTileWidth != imageWidth) {
					nameStream << "_" << setw(numDigits) << setfill('0') << tileIdx;
				}
				theTile->Name = nameStream.str();
				
				++tileIdx;
				result.push_back(theTile);
			}
		}
		FreeImage_Unload(dib);
		cout << "Done!" << endl;
	}
	else {
		cout << "Error loading image!" << endl;
	}
	return result;
}

Tile* TileExtractor::getTile(FIBITMAP* bmp) {
	Tile* result = new Tile();
	if(this->Options.RLE) {
		this->fillTileRLE(result, bmp);
	} else if(this->Options.IsScr) {
		this->fillTileSCR(result, bmp);
	} else {
		if(this->Options.PixelOrder == ConversionOptions::ROW) {
			this->fillTileByRows(result, bmp);
		}
		if(this->Options.PixelOrder == ConversionOptions::COLUMN) {
			this->fillTileByCols(result, bmp);
		}		
	}
	return result;
}

void TileExtractor::fillTileSCR(Tile* tile, FIBITMAP* bmp) {
	unsigned int scanlineCount = this->Options.ScanlineOrder.size();
	unsigned int charCount = this->TileHeight / scanlineCount;
	if((this->TileHeight % scanlineCount) != 0) {
		charCount += 1;
	}

	for (unsigned int y = 0; y < scanlineCount; ++y) {
		unsigned int remainingBytes = 0x800;
		unsigned int rowIndex = this->Options.ScanlineOrder[y];

		// Current line to process is rowIndex.
		for(unsigned int charIdx = 0; charIdx < charCount; ++charIdx) {
			if(rowIndex < this->TileHeight) {
				for (unsigned int x = 0; x < this->TileWidth; x += this->ModeIncrement) {
					getByteAt(tile, bmp, x, rowIndex, false, false);
					--remainingBytes;
				}
				rowIndex += scanlineCount;
			}
		}
		while(remainingBytes > 0) {
			tile->Data.push_back(0x00);
			for(unsigned int i=0; i<this->ModeIncrement; ++i) {
				tile->SourceValues.push_back((unsigned char)0x00);
				tile->MaskValues.push_back((unsigned char)0x00);
			}
			--remainingBytes;
		}
	}
}

#define START_OF_LINE 512
#define OPAQUE_PIXELS 0x00
#define END_OF_LINE 0xFF
#define END_OF_SPRITE 0xFE
#define TRANSPARENT_PIXELS_MASK 0xFF

void TileExtractor::fillTileRLE(Tile* tile, FIBITMAP* bmp) {
	int scanlineCount = this->Options.ScanlineOrder.size();
	int scanlineIdx = 0;
	int counterIdx = -1;
	int maskMode = START_OF_LINE;

//	cout << hex << setw(2) << setfill('0') << endl;
	for (unsigned int y = 0; y < this->TileHeight; ++y) {
		int charIdx = y / scanlineCount;
		bool oddY = ((y & 1) == 1);
		bool flip = this->Options.ZigZag && oddY;
		bool halfFlip = this->Options.HalfFlip && oddY;

		unsigned int rowIndex = 0;
		do {
			rowIndex = (charIdx * scanlineCount) + this->Options.ScanlineOrder[scanlineIdx];
			scanlineIdx = (scanlineIdx + 1) % scanlineCount;
		} while (rowIndex >= this->TileHeight);
		maskMode = START_OF_LINE;

		// Current line to process is rowIndex.
		for (unsigned int x = 0; x < this->TileWidth; x += this->ModeIncrement) {

			ColorAndMaskValues byteValues = getColorAndMask(bmp, x, rowIndex, flip, halfFlip);
//			cout << "byteValues(" << x <<", " << rowIndex << "):\tM=" << (int)byteValues.MaskByte << ",\tC=" << (int)byteValues.ColorByte << endl << flush;

			for (unsigned char palIdx : byteValues.ColorValues) {
				tile->SourceValues.push_back(palIdx);
			}
			for (unsigned char maskIdx : byteValues.TransparentValues) {
				tile->MaskValues.push_back(maskIdx);
			}

			if(byteValues.MaskByte == TRANSPARENT_PIXELS_MASK) {
				// Transparent.
				if(maskMode != TRANSPARENT_PIXELS_MASK) {
					maskMode = TRANSPARENT_PIXELS_MASK;
					counterIdx = tile->Data.size();
					// Number of transparent bytes
					tile->Data.push_back(0x0);
				}
				tile->Data[counterIdx]++;
			} else {
				if(maskMode == START_OF_LINE) {
					// 0 transparent bytes.
					tile->Data.push_back(0);
				}
				if(maskMode != OPAQUE_PIXELS) {
					maskMode = OPAQUE_PIXELS;
					counterIdx = tile->Data.size();
					// number of opaque bytes.
					tile->Data.push_back(0);
				}
				tile->Data.push_back(byteValues.ColorByte);
				tile->Data[counterIdx]++;
			}
			/*
			cout << "tile->Data = {";
			for(unsigned char c : tile->Data) {
				cout << (int)c << " ";
			}
			cout << "}" << endl;
*/
		}
		// End of line. Check if transparent mode, or opaque.
		unsigned char eolValue = END_OF_LINE;
		if(y == (unsigned int)(this->TileHeight - 1)) {
			eolValue = END_OF_SPRITE;
		}
		if(maskMode == OPAQUE_PIXELS) {
			tile->Data.push_back(eolValue);
		} else {
			tile->Data[counterIdx] = eolValue;
		}
	}
}

void TileExtractor::fillTileByRows(Tile* tile, FIBITMAP* bmp) {
	int scanlineCount = this->Options.ScanlineOrder.size();
	int scanlineIdx = 0;
	for (unsigned int y = 0; y < this->TileHeight; ++y) {
		int charIdx = y / scanlineCount;
		bool oddY = ((y & 1) == 1);
		bool flip = this->Options.ZigZag && oddY;
		bool halfFlip = this->Options.HalfFlip && oddY;

		unsigned int rowIndex = 0;
		do {
			rowIndex = (charIdx * scanlineCount) + this->Options.ScanlineOrder[scanlineIdx];
			scanlineIdx = (scanlineIdx + 1) % scanlineCount;
		} while (rowIndex >= this->TileHeight);

		// Current line to process is rowIndex.
		for (unsigned int x = 0; x < this->TileWidth; x += this->ModeIncrement) {
			getByteAt(tile, bmp, x, rowIndex, flip, halfFlip);
		}
	}
}

void TileExtractor::fillTileByCols(Tile* tile, FIBITMAP* bmp) {
	int scanlineCount = this->Options.ScanlineOrder.size();
	int scanlineIdx = 0;
	// Current line to process is rowIndex.
	for (unsigned int x = 0; x < this->TileWidth; x += this->ModeIncrement) {
		for (unsigned int y = 0; y < this->TileHeight; ++y) {
			int charIdx = y / scanlineCount;
			bool oddY = ((y & 1) == 1);
			bool flip = this->Options.ZigZag && oddY;
			bool halfFlip = this->Options.HalfFlip && oddY;

			unsigned int rowIndex = 0;
			do {
				rowIndex = (charIdx * scanlineCount) + this->Options.ScanlineOrder[scanlineIdx];
				scanlineIdx = (scanlineIdx + 1) % scanlineCount;
			} while (rowIndex >= this->TileHeight);

			getByteAt(tile, bmp, x, rowIndex, flip, halfFlip);
		}
	}
}

void TileExtractor::getByteAt(Tile* tile, FIBITMAP* bmp, int col, int row, bool flip, bool halfFlip) {
	ColorAndMaskValues byteValues = getColorAndMask(bmp, col, row, flip, halfFlip);

	tile->Data.push_back(byteValues.ColorByte);
	tile->MaskData.push_back(byteValues.MaskByte);
	for (unsigned char palIdx : byteValues.ColorValues) {
		tile->SourceValues.push_back(palIdx);
	}
	for (unsigned char maskIdx : byteValues.TransparentValues) {
		tile->MaskValues.push_back(maskIdx);
	}
}

ColorAndMaskValues TileExtractor::getColorAndMask(FIBITMAP* bmp, int x, int y, bool flip, bool halfFlip) {
	ColorAndMaskValues result;

	result.ColorValues = extractPixels(bmp, x, y, flip, halfFlip);
	result.TransparentValues = extractTransPixels(result.ColorValues);
	result.ColorByte = ColorCombinator::CombineColData(result.ColorValues);
	result.MaskByte = ColorCombinator::CombineColData(result.TransparentValues);
	return result;
}

vector<unsigned char> TileExtractor::extractPixels(FIBITMAP* bmp, int x, int y, bool flip, bool halfFlip) {
	vector<unsigned char> result;
	for (unsigned int offset = 0; offset < this->ModeIncrement; ++offset) {
		if ((x + offset) < this->TileWidth)
		{
			int sourceX = 0;
			if(flip) {
				if(halfFlip) {
					sourceX = this->TileWidth - x - offset - 1;
				} else {
					sourceX = this->TileWidth - x - this->ModeIncrement + offset;					
				}
			} else {
				if(halfFlip) {
					sourceX = x + this->ModeIncrement - offset - 1;
				} else {
					sourceX = x + offset;					
				}
			}
			RGBQUAD rgb;
			FreeImage_GetPixelColor(bmp, sourceX, this->TileHeight - y - 1, &rgb);
			Color current = Color(rgb.rgbReserved, rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
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